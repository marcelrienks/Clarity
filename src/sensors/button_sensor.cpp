#include "sensors/button_sensor.h"
#include <Arduino.h>

#include "esp32-hal-log.h"

// Constructors and Destructors

ButtonSensor::ButtonSensor(IGpioProvider *gpioProvider) : gpioProvider_(gpioProvider)
{
    log_v("ButtonSensor() constructor called");
}

// ISensor Interface Implementation

void ButtonSensor::Init()
{
    log_v("Init() called");
    gpioProvider_->PinMode(gpio_pins::INPUT_BUTTON, INPUT_PULLDOWN);
    log_d("ButtonSensor initialized on GPIO %d with pull-down", gpio_pins::INPUT_BUTTON);
    
    bool initialState = gpioProvider_->DigitalRead(gpio_pins::INPUT_BUTTON);
    log_i("GPIO %d initial state: %s", gpio_pins::INPUT_BUTTON,
          initialState ? "HIGH (pressed)" : "LOW (released)");
    
    log_i("ButtonSensor initialization completed on GPIO %d", gpio_pins::INPUT_BUTTON);
}

/// @brief Get the current button state as a sensor reading
/// @return Reading containing the current button action
Reading ButtonSensor::GetReading()
{
    log_v("GetReading() called");
    // Process button state to detect actions
    ProcessButtonState();
    
    // Return the detected action as a reading
    return static_cast<int32_t>(detectedAction_);
}

/// @brief Get the current button action
/// @return ButtonAction indicating the type of press detected
ButtonAction ButtonSensor::GetButtonAction()
{
    log_v("GetButtonAction() called");
    ProcessButtonState();
    return detectedAction_;
}

/// @brief Check if a button action is ready
/// @return true if a valid button action has been detected
bool ButtonSensor::HasButtonAction() const
{
    return actionReady_;
}

/// @brief Clear the current button action (after processing)
void ButtonSensor::ClearButtonAction()
{
    log_v("ClearButtonAction() called");
    detectedAction_ = ButtonAction::NONE;
    actionReady_ = false;
}

/// @brief Check if the action button is currently pressed
/// @return true if button is pressed (GPIO HIGH), false otherwise
bool ButtonSensor::IsButtonPressed()
{
    log_v("IsButtonPressed() called");
    // GPIO 32 reads HIGH when button is pressed (connected to 3.3V)
    return gpioProvider_->DigitalRead(gpio_pins::INPUT_BUTTON);
}

bool ButtonSensor::ReadButtonState()
{
    log_v("ReadButtonState() called");
    return gpioProvider_->DigitalRead(gpio_pins::INPUT_BUTTON);
}

bool ButtonSensor::HasStateChanged()
{
    log_v("HasStateChanged() called");
    
    ProcessButtonState();
    return actionReady_;
}

void ButtonSensor::ProcessButtonState()
{
    bool currentState = ReadButtonState();
    unsigned long currentTime = millis();
    
    if (currentState && !currentButtonState_)
    {
        buttonPressStartTime_ = currentTime;
        currentButtonState_ = true;
        log_d("Button press started at %lu ms", buttonPressStartTime_);
    }
    else if (!currentState && currentButtonState_)
    {
        buttonPressDuration_ = currentTime - buttonPressStartTime_;
        currentButtonState_ = false;
        
        log_d("Button released after %lu ms", buttonPressDuration_);
        
        ButtonAction action = DetermineAction(buttonPressDuration_);
        if (action != ButtonAction::NONE)
        {
            detectedAction_ = action;
            actionReady_ = true;
            log_i("Button action detected: %s", 
                  action == ButtonAction::SHORT_PRESS ? "SHORT_PRESS" : "LONG_PRESS");
        }
    }
    else if (currentState && currentButtonState_)
    {
        unsigned long heldDuration = currentTime - buttonPressStartTime_;
        if (heldDuration > LONG_PRESS_MAX_MS)
        {
            log_w("Button press timeout after %lu ms", heldDuration);
            currentButtonState_ = false;
            buttonPressStartTime_ = 0;
        }
    }
}

/// @brief Determine button action based on press duration
/// @param duration Press duration in milliseconds
/// @return ButtonAction type based on duration
ButtonAction ButtonSensor::DetermineAction(unsigned long duration)
{
    if (duration < DEBOUNCE_MS)
    {
        log_v("Press duration %lu ms - ignored (debounce)", duration);
        return ButtonAction::NONE;
    }
    else if (duration <= SHORT_PRESS_MAX_MS)
    {
        log_d("Press duration %lu ms - SHORT_PRESS", duration);
        return ButtonAction::SHORT_PRESS;
    }
    else if (duration <= LONG_PRESS_MAX_MS)
    {
        log_d("Press duration %lu ms - LONG_PRESS", duration);
        return ButtonAction::LONG_PRESS;
    }
    else
    {
        log_v("Press duration %lu ms - ignored (timeout)", duration);
        return ButtonAction::NONE;
    }
}

void ButtonSensor::OnInterruptTriggered()
{
    ProcessButtonState();
}