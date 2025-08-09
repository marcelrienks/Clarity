#include "managers/input_manager.h"
#include "actions/input_actions.h"
#include "utilities/types.h"
#include <Arduino.h>
#include <cstring>

#ifdef CLARITY_DEBUG
#include "esp32-hal-log.h"
#define LOG_TAG "InputManager"
#else
#define log_d(...)
#define log_w(...)
#define log_e(...)
#endif

InputManager::InputManager(std::shared_ptr<InputButtonSensor> buttonSensor)
    : buttonSensor_(buttonSensor)
    , currentService_(nullptr)
    , panelSwitchCallback_(nullptr)
    , buttonState_(ButtonState::IDLE)
    , pressStartTime_(0)
    , debounceStartTime_(0)
    , lastButtonState_(false)
    , initialized_(false)
{
}

void InputManager::Init()
{
    if (initialized_) {
        log_w("InputManager already initialized");
        return;
    }

    if (!buttonSensor_) {
        log_e("Button sensor is null");
        return;
    }
    
    
    // Initialize the button sensor
    buttonSensor_->Init();
    
    // Initialize state
    lastButtonState_ = IsButtonPressed();
    buttonState_ = ButtonState::IDLE;
    
    log_i("Initial button state: %s", lastButtonState_ ? "PRESSED" : "RELEASED");
    
    
    initialized_ = true;
    log_i("InputManager initialized");
}

void InputManager::ProcessInputEvents()
{
    if (!initialized_) {
        return;
    }

    bool currentButtonState = IsButtonPressed();
    unsigned long currentTime = GetCurrentTime();

    // Detect button state changes
    if (currentButtonState != lastButtonState_) {
        if (currentButtonState) {
            // Button pressed
            HandleButtonPress();
        } else {
            // Button released
            HandleButtonRelease();
        }
        lastButtonState_ = currentButtonState;
    }

    // Handle state-specific logic
    switch (buttonState_) {
        case ButtonState::DEBOUNCE:
            if (currentTime - debounceStartTime_ >= DEBOUNCE_TIME_MS) {
                if (IsButtonPressed()) {
                    buttonState_ = ButtonState::PRESSED;
                    pressStartTime_ = currentTime;
                } else {
                    buttonState_ = ButtonState::IDLE;
                }
            }
            break;

        case ButtonState::PRESSED:
        {
            // Check for long press threshold (1-3 seconds)
            unsigned long pressDuration = currentTime - pressStartTime_;
            if (pressDuration >= LONG_PRESS_THRESHOLD_MS && pressDuration <= LONG_PRESS_MAX_MS && buttonState_ != ButtonState::LONG_PRESS_SENT) {
                log_i("Long press detected");
                
                if (currentService_) {
                    // Get action from current panel
                    auto action = currentService_->GetLongPressAction();
                    if (action) {
                        // Check if panel can process action immediately
                        if (currentService_->CanProcessInput() && action->CanExecute()) {
                            log_d("Executing long press action immediately: %s", action->GetDescription());
                            action->Execute();
                            
                            // Check if this is a SimplePanelSwitchAction that needs callback handling
                            if (strcmp(action->GetActionType(), "SimplePanelSwitchAction") == 0 && panelSwitchCallback_) {
                                auto simplePanelSwitchAction = static_cast<SimplePanelSwitchAction*>(action.get());
                                log_d("Handling SimplePanelSwitchAction via callback (immediate long press)");
                                panelSwitchCallback_(simplePanelSwitchAction->GetTargetPanel());
                            }
                        } else {
                            // Store the action for later processing (overwrites any pending short press)
                            pendingAction_.action = std::move(action);
                            pendingAction_.timestamp = currentTime;
                            log_d("Long press action queued for later: %s", pendingAction_.action->GetDescription());
                        }
                    }
                }
                
                buttonState_ = ButtonState::LONG_PRESS_SENT;
            }
            
            // Check for timeout
            CheckPressTimeout();
            break;
        }

        case ButtonState::LONG_PRESS_SENT:
            CheckPressTimeout();
            break;

        case ButtonState::IDLE:
        default:
            // No action needed
            break;
    }
}

void InputManager::SetInputService(IInputService* service, const char* panelName)
{
    currentService_ = service;
    currentPanelName_ = panelName ? panelName : "";
    log_d("Input service set for panel %s: %p", panelName, service);
}

void InputManager::ClearInputService()
{
    currentService_ = nullptr;
    log_d("Input service cleared");
}

void InputManager::SetPanelSwitchCallback(std::function<void(const char*)> callback)
{
    panelSwitchCallback_ = callback;
    log_d("Panel switch callback set: %p", &callback);
}

void InputManager::HandleButtonPress()
{
    unsigned long currentTime = GetCurrentTime();
    
    log_d("Button press detected at %lu ms", currentTime);
    
    buttonState_ = ButtonState::DEBOUNCE;
    debounceStartTime_ = currentTime;
}

void InputManager::HandleButtonRelease()
{
    unsigned long currentTime = GetCurrentTime();
    
    if (buttonState_ == ButtonState::PRESSED) {
        // Calculate press duration
        unsigned long pressDuration = currentTime - pressStartTime_;
        
        if (pressDuration > LONG_PRESS_MAX_MS) {
            // Press was too long (>3 seconds), ignore it
            log_w("Button press too long (%lu ms), ignoring", pressDuration);
        }
        else if (pressDuration >= SHORT_PRESS_MIN_MS && pressDuration < LONG_PRESS_THRESHOLD_MS) {
            log_i("Short press detected");
            
            if (currentService_) {
                // Get action from current panel
                auto action = currentService_->GetShortPressAction();
                if (action) {
                    // Check if panel can process action immediately
                    if (currentService_->CanProcessInput() && action->CanExecute()) {
                        log_d("Executing short press action immediately: %s", action->GetDescription());
                        action->Execute();
                        
                        // Check if this is a SimplePanelSwitchAction that needs callback handling
                        if (strcmp(action->GetActionType(), "SimplePanelSwitchAction") == 0 && panelSwitchCallback_) {
                            auto simplePanelSwitchAction = static_cast<SimplePanelSwitchAction*>(action.get());
                            log_d("Handling SimplePanelSwitchAction via callback (immediate short press)");
                            panelSwitchCallback_(simplePanelSwitchAction->GetTargetPanel());
                        }
                    } else {
                        // Store the action for later processing
                        pendingAction_.action = std::move(action);
                        pendingAction_.timestamp = currentTime;
                        log_d("Short press action queued for later: %s", pendingAction_.action->GetDescription());
                    }
                }
            }
        }
        else if (pressDuration >= LONG_PRESS_THRESHOLD_MS && pressDuration <= LONG_PRESS_MAX_MS) {
            // This is a long press that completed without being sent during press hold
            log_i("Long press detected on release");
            
            if (currentService_) {
                auto action = currentService_->GetLongPressAction();
                if (action) {
                    if (currentService_->CanProcessInput() && action->CanExecute()) {
                        log_d("Executing long press action on release: %s", action->GetDescription());
                        action->Execute();
                        
                        // Check if this is a SimplePanelSwitchAction that needs callback handling
                        if (strcmp(action->GetActionType(), "SimplePanelSwitchAction") == 0 && panelSwitchCallback_) {
                            auto simplePanelSwitchAction = static_cast<SimplePanelSwitchAction*>(action.get());
                            log_d("Handling SimplePanelSwitchAction via callback (long press on release)");
                            panelSwitchCallback_(simplePanelSwitchAction->GetTargetPanel());
                        }
                    } else {
                        pendingAction_.action = std::move(action);
                        pendingAction_.timestamp = currentTime;
                        log_d("Long press action queued for later: %s", pendingAction_.action->GetDescription());
                    }
                }
            }
        }
    }
    
    buttonState_ = ButtonState::IDLE;
}

void InputManager::CheckPressTimeout()
{
    unsigned long currentTime = GetCurrentTime();
    
    if (currentTime - pressStartTime_ >= MAX_PRESS_TIME_MS) {
        log_w("Button press timeout reached, resetting to idle");
        buttonState_ = ButtonState::IDLE;
    }
}

bool InputManager::IsButtonPressed() const
{
    if (!buttonSensor_ || !initialized_) {
        return false;
    }
    
    // Use the button sensor to check if button is pressed
    return buttonSensor_->IsButtonPressed();
}

unsigned long InputManager::GetCurrentTime() const
{
    // Use millis() equivalent - this will need to be mocked for testing
    return millis();
}

// IInterrupt Interface Implementation

void InputManager::CheckInterrupts()
{
    if (!initialized_) {
        return;
    }
    
    // Process button input events
    ProcessInputEvents();
    
    // Process any pending actions
    ProcessPendingActions();
}

bool InputManager::HasPendingInterrupts() const
{
    if (!initialized_) {
        return false;
    }
    
    // Check if button state has changed or if we have pending actions
    bool currentButtonState = IsButtonPressed();
    bool buttonStateChanged = (currentButtonState != lastButtonState_);
    bool hasPendingAction = pendingAction_.HasAction();
    bool inActiveState = (buttonState_ != ButtonState::IDLE);
    
    return buttonStateChanged || hasPendingAction || inActiveState;
}

// Action Processing Methods

void InputManager::ProcessPendingActions()
{
    if (!pendingAction_.HasAction()) {
        return;
    }
    
    unsigned long currentTime = GetCurrentTime();
    
    // Check if pending action has expired
    if (currentTime - pendingAction_.timestamp > INPUT_TIMEOUT_MS) {
        log_d("Pending action expired, discarding: %s", pendingAction_.action->GetDescription());
        pendingAction_.Clear();
        return;
    }
    
    // Check if current service can now process the action
    if (currentService_ && currentService_->CanProcessInput()) {
        log_d("Executing queued action: %s", pendingAction_.action->GetDescription());
        
        if (pendingAction_.action->CanExecute()) {
            pendingAction_.action->Execute();
            
            // Check if this is a SimplePanelSwitchAction that needs callback handling
            if (strcmp(pendingAction_.action->GetActionType(), "SimplePanelSwitchAction") == 0 && panelSwitchCallback_) {
                auto simplePanelSwitchAction = static_cast<SimplePanelSwitchAction*>(pendingAction_.action.get());
                log_d("Handling SimplePanelSwitchAction via callback");
                panelSwitchCallback_(simplePanelSwitchAction->GetTargetPanel());
            }
        }
        
        // Clear the pending action after processing
        pendingAction_.Clear();
    }
}



