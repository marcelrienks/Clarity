#include "sensors/button_sensor.h"
#include "utilities/logging.h"
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
    
    bool initialState = gpioProvider_->DigitalRead(gpio_pins::INPUT_BUTTON);
    log_i("GPIO %d initial state: %s", gpio_pins::INPUT_BUTTON,
          initialState ? "HIGH (pressed)" : "LOW (released)");
    
    log_i("ButtonSensor initialization completed on GPIO %d", gpio_pins::INPUT_BUTTON);
}

/// @brief Get the current button state as a sensor reading
/// @return Reading containing the current button state (bool)
Reading ButtonSensor::GetReading()
{
    log_v("GetReading() called");
    // Process button state to detect actions
    ProcessButtonState();
    
    // Return the current button state as boolean for ActionHandler compatibility
    bool state = ReadButtonState();
    log_v("ButtonSensor::GetReading() returning %s", state ? "true" : "false");
    return state;
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
    if (!gpioProvider_) {
        log_e("ButtonSensor::ReadButtonState() - gpioProvider_ is null!");
        return false;
    }
    
    bool state = gpioProvider_->DigitalRead(gpio_pins::INPUT_BUTTON);
    
    static unsigned long lastDebugTime = 0;
    unsigned long currentTime = millis();
    
    // Log every 2 seconds to verify GPIO is being read
    if (currentTime - lastDebugTime > 2000) {
        log_i("ButtonSensor GPIO %d continuous read = %s", gpio_pins::INPUT_BUTTON, state ? "HIGH" : "LOW");
        lastDebugTime = currentTime;
    }
    
    log_v("ButtonSensor::ReadButtonState() GPIO %d = %s", gpio_pins::INPUT_BUTTON, state ? "HIGH" : "LOW");
    return state;
}

bool ButtonSensor::HasStateChanged()
{
    log_v("HasStateChanged() called");
    ProcessButtonState();
    
    // If no action is ready, clear the interrupt ID
    if (!actionReady_)
    {
        triggerInterruptId_ = nullptr;
    }
    
    return actionReady_;
}

const char* ButtonSensor::GetTriggerInterruptId() const
{
    return triggerInterruptId_;
}

void ButtonSensor::ProcessButtonState()
{
    // Get the logical button state (ReadButtonState handles the GPIO inversion)
    bool currentState = ReadButtonState();
    unsigned long currentTime = millis();
    
    static unsigned long lastDebugTime = 0;
    static bool lastLoggedState = false;
    
    // Debug every 3 seconds or on state changes
    if ((currentTime - lastDebugTime > 3000) || (currentState != lastLoggedState)) {
        // Button state processing
        lastDebugTime = currentTime;
        lastLoggedState = currentState;
    }
    
    if (currentState && !currentButtonState_)
    {
        buttonPressStartTime_ = currentTime;
        currentButtonState_ = true;
        log_t("ButtonSensor: PRESS STARTED at %lu ms (GPIO HIGH detected)", buttonPressStartTime_);
    }
    else if (!currentState && currentButtonState_)
    {
        buttonPressDuration_ = currentTime - buttonPressStartTime_;
        currentButtonState_ = false;
        
        log_t("ButtonSensor: PRESS ENDED after %lu ms (GPIO LOW detected)", buttonPressDuration_);
        
        ButtonAction action = DetermineAction(buttonPressDuration_);
        if (action != ButtonAction::NONE)
        {
            detectedAction_ = action;
            actionReady_ = true;
            log_t("ButtonSensor: ACTION DETECTED: %s (actionReady=true)", 
                  action == ButtonAction::SHORT_PRESS ? "SHORT_PRESS" : "LONG_PRESS");
                  
            // In simplified system, determine which interrupt should be triggered
            if (action == ButtonAction::SHORT_PRESS)
            {
                log_t("ButtonSensor: Should trigger 'short_press' action");
                triggerInterruptId_ = "short_press";
            }
            else if (action == ButtonAction::LONG_PRESS)
            {
                log_t("ButtonSensor: Should trigger 'long_press' action");
                triggerInterruptId_ = "long_press";
            }
        }
        else
        {
            log_w("ButtonSensor: Press duration %lu ms resulted in NO ACTION", buttonPressDuration_);
        }
    }
    else if (currentState && currentButtonState_)
    {
        unsigned long heldDuration = currentTime - buttonPressStartTime_;
        if (heldDuration > LONG_PRESS_MAX_MS)
        {
            log_w("ButtonSensor: PRESS TIMEOUT after %lu ms - resetting state", heldDuration);
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
        return ButtonAction::SHORT_PRESS;
    }
    else if (duration <= LONG_PRESS_MAX_MS)
    {
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