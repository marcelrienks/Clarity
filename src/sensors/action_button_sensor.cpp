#include "sensors/action_button_sensor.h"
#include "managers/interrupt_manager.h"
#include <Arduino.h>

#include "esp32-hal-log.h"

// Constructors and Destructors

ActionButtonSensor::ActionButtonSensor(IGpioProvider *gpioProvider) : gpioProvider_(gpioProvider)
{
    log_v("ActionButtonSensor() constructor called");
}

// ISensor Interface Implementation

/// @brief Initialize the action button sensor on GPIO 32
/// @details Configures GPIO with pull-down resistor for button input
void ActionButtonSensor::Init()
{
    log_v("Init() called");
    // Configure GPIO 32 as input with pull-down resistor
    // This ensures LOW state when button is not pressed (Normally Open button to 3.3V)
    gpioProvider_->PinMode(gpio_pins::INPUT_BUTTON, INPUT_PULLDOWN);

    log_d("ActionButtonSensor initialized on GPIO %d with pull-down", gpio_pins::INPUT_BUTTON);

    // Log initial state after proper configuration
    bool initialState = gpioProvider_->DigitalRead(gpio_pins::INPUT_BUTTON);
    log_i("GPIO %d initial state after configuration: %s", gpio_pins::INPUT_BUTTON,
          initialState ? "HIGH (pressed)" : "LOW (released)");
    
    log_i("ActionButtonSensor initialization completed on GPIO %d", gpio_pins::INPUT_BUTTON);
}

/// @brief Get the current button state as a sensor reading
/// @return Reading containing 1 if pressed, 0 if released
Reading ActionButtonSensor::GetReading()
{
    log_v("GetReading() called");
    // Return button state as boolean (0 = not pressed, 1 = pressed)
    bool pressed = IsButtonPressed();
    return static_cast<int32_t>(pressed ? 1 : 0);
}

/// @brief Check if the action button is currently pressed
/// @return true if button is pressed (GPIO HIGH), false otherwise
bool ActionButtonSensor::IsButtonPressed()
{
    log_v("IsButtonPressed() called");
    // GPIO 32 reads HIGH when button is pressed (connected to 3.3V)
    return gpioProvider_->DigitalRead(gpio_pins::INPUT_BUTTON);
}

bool ActionButtonSensor::readButtonState()
{
    log_v("readButtonState() called");
    return gpioProvider_->DigitalRead(gpio_pins::INPUT_BUTTON);
}

bool ActionButtonSensor::HasStateChanged()
{
    log_v("HasStateChanged() called");
    
    bool currentState = readButtonState();
    bool changed = DetectChange(currentState, previousButtonState_);
    
    if (changed)
    {
        log_d("Button state changed from %s to %s", 
              previousButtonState_ ? "PRESSED" : "RELEASED",
              currentState ? "PRESSED" : "RELEASED");
    }
    
    return changed;
}

void ActionButtonSensor::OnInterruptTriggered()
{
    // This method is no longer used - button timing is handled by InterruptManager
    log_d("OnInterruptTriggered() deprecated - button handling moved to InterruptManager 8-step flow");
}