#include "managers/action_manager.h"
#include "managers/error_manager.h"
#include "utilities/types.h"
#include <Arduino.h>
#include <cstring>

#ifdef CLARITY_DEBUG
#include "esp32-hal-log.h"
#define LOG_TAG "ActionManager"
#else
#define log_d(...)
#define log_w(...)
#define log_e(...)
#endif

ActionManager::ActionManager(std::shared_ptr<ActionButtonSensor> buttonSensor)
    : buttonSensor_(buttonSensor)
    , currentService_(nullptr)
    , buttonState_(ButtonState::IDLE)
    , pressStartTime_(0)
    , debounceStartTime_(0)
    , lastButtonState_(false)
    , initialized_(false)
    , pendingAction_(nullptr)
    , pendingActionTimestamp_(0)
{
}

void ActionManager::Init()
{
    if (initialized_) {
        log_w("ActionManager already initialized");
        return;
    }

    if (!buttonSensor_) {
        log_e("Button sensor is null");
        ErrorManager::Instance().ReportCriticalError("ActionManager", 
            "Cannot initialize - button sensor is null");
        return;
    }
    
    
    // Initialize the button sensor
    buttonSensor_->Init();
    
    // Initialize state
    lastButtonState_ = IsButtonPressed();
    buttonState_ = ButtonState::IDLE;
    
    log_i("Initial button state: %s", lastButtonState_ ? "PRESSED" : "RELEASED");
    
    
    initialized_ = true;
    log_i("ActionManager initialized");
}

void ActionManager::ProcessInputEvents()
{
    if (!initialized_) {
        return;
    }

    bool currentButtonState = IsButtonPressed();
    
    // Log only actual button state changes
    static bool lastLoggedState = false;
    static bool firstCall = true;
    if (firstCall || currentButtonState != lastLoggedState) {
        log_i("Button state changed: %s", currentButtonState ? "PRESSED" : "RELEASED");
        lastLoggedState = currentButtonState;
        firstCall = false;
    }
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
                    log_d("Button press confirmed after debounce at %lu ms", currentTime);
                } else {
                    buttonState_ = ButtonState::IDLE;
                }
            }
            break;

        case ButtonState::PRESSED:
        {
            // Check for long press threshold (2-5 seconds)
            unsigned long pressDuration = currentTime - pressStartTime_;
            if (pressDuration >= LONG_PRESS_THRESHOLD_MS && pressDuration <= LONG_PRESS_MAX_MS && buttonState_ != ButtonState::LONG_PRESS_SENT) {
                log_i("Long press detected after %lu ms (started at %lu ms)", pressDuration, pressStartTime_);
                
                if (currentService_) {
                    // Get action from current panel
                    Action action = currentService_->GetLongPressAction();
                    if (action.IsValid()) {
                        // Check if panel can process action immediately
                        if (currentService_->CanProcessInput()) {
                            log_d("Executing long press action immediately");
                            action.execute();
                        } else {
                            // Store the action for later processing (overwrites any pending action)
                            pendingAction_ = std::move(action);
                            pendingActionTimestamp_ = currentTime;
                            log_d("Long press action queued for later");
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

void ActionManager::RegisterPanel(IActionService* service, const char* panelName)
{
    currentService_ = service;
    currentPanelName_ = panelName ? panelName : "";
    log_d("Panel registered for actions: %s %p", panelName, service);
}

void ActionManager::ClearPanel()
{
    currentService_ = nullptr;
    log_d("Panel registration cleared");
}


void ActionManager::HandleButtonPress()
{
    unsigned long currentTime = GetCurrentTime();
    
    log_i("ActionManager: Button press detected at %lu ms", currentTime);
    
    buttonState_ = ButtonState::DEBOUNCE;
    debounceStartTime_ = currentTime;
}

void ActionManager::HandleButtonRelease()
{
    unsigned long currentTime = GetCurrentTime();
    
    log_d("Button release detected at %lu ms", currentTime);
    
    if (buttonState_ == ButtonState::PRESSED) {
        // Calculate press duration
        unsigned long pressDuration = currentTime - pressStartTime_;
        log_d("Button release duration: %lu ms (from %lu to %lu)", pressDuration, pressStartTime_, currentTime);
        
        if (pressDuration > LONG_PRESS_MAX_MS) {
            // Press was too long (>5 seconds), ignore it
            log_w("Button press too long (%lu ms), ignoring", pressDuration);
        }
        else if (pressDuration >= SHORT_PRESS_MIN_MS && pressDuration < LONG_PRESS_THRESHOLD_MS) {
            log_i("Short press detected");
            
            if (currentService_) {
                // Get action from current panel
                Action action = currentService_->GetShortPressAction();
                if (action.IsValid()) {
                    // Check if panel can process action immediately
                    if (currentService_->CanProcessInput()) {
                        log_d("Executing short press action immediately");
                        action.execute();
                    } else {
                        // Store the action for later processing
                        pendingAction_ = std::move(action);
                        pendingActionTimestamp_ = currentTime;
                        log_d("Short press action queued for later");
                    }
                }
            }
        }
        else if (pressDuration >= LONG_PRESS_THRESHOLD_MS && pressDuration <= LONG_PRESS_MAX_MS) {
            // This is a long press that completed without being sent during press hold
            log_i("Long press detected on release");
            
            if (currentService_) {
                Action action = currentService_->GetLongPressAction();
                if (action.IsValid()) {
                    if (currentService_->CanProcessInput()) {
                        log_d("Executing long press action on release");
                        action.execute();
                    } else {
                        pendingAction_ = std::move(action);
                        pendingActionTimestamp_ = currentTime;
                        log_d("Long press action queued for later");
                    }
                }
            }
        }
    }
    
    buttonState_ = ButtonState::IDLE;
}

void ActionManager::CheckPressTimeout()
{
    unsigned long currentTime = GetCurrentTime();
    
    if (currentTime - pressStartTime_ >= MAX_PRESS_TIME_MS) {
        log_w("Button press timeout reached, resetting to idle");
        buttonState_ = ButtonState::IDLE;
    }
}

bool ActionManager::IsButtonPressed() const
{
    if (!buttonSensor_ || !initialized_) {
        return false;
    }
    
    // Use the button sensor to check if button is pressed
    bool pressed = buttonSensor_->IsButtonPressed();
    
    // Enhanced logging to debug button press detection
    static bool lastLoggedState = false;
    static bool firstCall = true;
    static unsigned long logCount = 0;
    
    logCount++;
    
    // Log every 500 calls or on state changes (more frequent logging)
    if (firstCall || pressed != lastLoggedState || (logCount % 500 == 0)) {
        log_i("ActionManager: Button check %lu - GPIO 32 state: %s", logCount, pressed ? "HIGH (PRESSED)" : "LOW (released)");
        lastLoggedState = pressed;
        firstCall = false;
    }
    
    return pressed;
}

unsigned long ActionManager::GetCurrentTime() const
{
    // Use millis() equivalent - this will need to be mocked for testing
    return millis();
}

// IInterruptService Interface Implementation

void ActionManager::CheckInterrupts()
{
    if (!initialized_) {
        return;
    }
    
    static unsigned long checkCount = 0;
    checkCount++;
    
    // Log every 500 calls to verify this method is being called
    if (checkCount % 500 == 0) {
        log_i("ActionManager: CheckInterrupts called %lu times", checkCount);
    }
    
    // Process button input events
    ProcessInputEvents();
    
    // Process any pending actions
    ProcessPendingActions();
}

bool ActionManager::HasPendingInterrupts() const
{
    if (!initialized_) {
        return false;
    }
    
    // Always return true to ensure continuous button polling
    // This is necessary to detect button state changes
    return true;
}

// Action Processing Methods

void ActionManager::ProcessPendingActions()
{
    if (!pendingAction_.IsValid()) {
        return;
    }
    
    unsigned long currentTime = GetCurrentTime();
    
    // Check if pending action has expired
    if (currentTime - pendingActionTimestamp_ > INPUT_TIMEOUT_MS) {
        log_d("Pending action expired, discarding");
        pendingAction_ = Action(nullptr);
        pendingActionTimestamp_ = 0;
        return;
    }
    
    // Check if current service can now process the action
    if (currentService_ && currentService_->CanProcessInput()) {
        log_d("Executing queued action");
        
        pendingAction_.execute();
        
        // Clear the pending action after processing
        pendingAction_ = Action(nullptr);
        pendingActionTimestamp_ = 0;
    }
}


