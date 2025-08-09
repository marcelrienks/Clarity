#include "managers/input_manager.h"
#include "utilities/types.h"
#include <Arduino.h>

#ifdef CLARITY_DEBUG
#include "esp32-hal-log.h"
#define LOG_TAG "InputManager"
#else
#define log_d(...)
#define log_w(...)
#define log_e(...)
#endif

InputManager::InputManager(std::shared_ptr<InputButtonSensor> buttonSensor, IPanelService* panelService)
    : buttonSensor_(buttonSensor)
    , panelService_(panelService)
    , currentService_(nullptr)
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
    
    if (!panelService_) {
        log_e("Panel service is null (should be injected via constructor)");
        return;
    }
    
    // Initialize the button sensor
    buttonSensor_->Init();
    
    // Initialize state
    lastButtonState_ = IsButtonPressed();
    buttonState_ = ButtonState::IDLE;
    
    log_i("Initial button state: %s", lastButtonState_ ? "PRESSED" : "RELEASED");
    
    // Register input actions
    RegisterInputActions();
    
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
            // Check for long press threshold
            if (currentTime - pressStartTime_ >= LONG_PRESS_THRESHOLD_MS) {
                log_i("Long press detected");
                
                // Check for panel switch action first
                auto it = longPressActions_.find(currentPanelName_);
                if (it != longPressActions_.end() && it->second.enabled && it->second.targetPanel) {
                    log_i("Long press triggers panel switch to: %s", it->second.targetPanel);
                    if (panelService_) {
                        panelService_->CreateAndLoadPanel(it->second.targetPanel);
                    }
                } else if (currentService_) {
                    // No panel switch action, process immediately
                    currentService_->OnLongPress();
                }
                
                buttonState_ = ButtonState::LONG_PRESS_SENT;
            }
            
            // Check for timeout
            CheckPressTimeout();
            break;

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
        
        if (pressDuration >= SHORT_PRESS_MIN_MS && pressDuration < LONG_PRESS_THRESHOLD_MS) {
            log_i("Short press detected");
            
            // Check for panel switch action first
            auto it = shortPressActions_.find(currentPanelName_);
            if (it != shortPressActions_.end() && it->second.enabled && it->second.targetPanel) {
                log_i("Short press triggers panel switch to: %s", it->second.targetPanel);
                if (panelService_) {
                    panelService_->CreateAndLoadPanel(it->second.targetPanel);
                }
            } else if (currentService_) {
                // No panel switch action, process immediately
                currentService_->OnShortPress();
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

void InputManager::RegisterInputActions()
{
    log_d("Registering input actions for panels");
    
    // Configure input actions based on input.md specification
    
    // OilPanel: Short=none, Long=config
    shortPressActions_[PanelNames::OIL] = {nullptr, false};
    longPressActions_[PanelNames::OIL] = {PanelNames::CONFIG, true};
    
    // SplashPanel: Short=skip animation, Long=config
    shortPressActions_[PanelNames::SPLASH] = {nullptr, false};  // Handled internally by panel
    longPressActions_[PanelNames::SPLASH] = {PanelNames::CONFIG, true};
    
    // ErrorPanel: Short=cycle errors, Long=clear all
    shortPressActions_[PanelNames::ERROR] = {nullptr, false};  // Handled internally by panel
    longPressActions_[PanelNames::ERROR] = {nullptr, false};   // Handled internally by panel
    
    // ConfigPanel: Short=next option, Long=select
    shortPressActions_[PanelNames::CONFIG] = {nullptr, false}; // Handled internally by panel
    longPressActions_[PanelNames::CONFIG] = {nullptr, false};  // Handled internally by panel
}

