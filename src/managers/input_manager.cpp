#include "managers/input_manager.h"
#include <Arduino.h>

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
                if (currentService_) {
                    log_i("Long press detected");
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

void InputManager::SetInputService(IInputService* service)
{
    currentService_ = service;
    log_d("Input service set: %p", service);
}

void InputManager::ClearInputService()
{
    currentService_ = nullptr;
    log_d("Input service cleared");
}

void InputManager::HandleButtonPress()
{
    unsigned long currentTime = GetCurrentTime();
    
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
            if (currentService_) {
                log_i("Short press detected");
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