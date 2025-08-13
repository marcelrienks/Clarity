#include "sensors/action_button_sensor.h"
#include <Arduino.h>

#ifdef CLARITY_DEBUG
    #include "esp32-hal-log.h"
    #define LOG_TAG "ActionButtonSensor"
#else
    #define log_d(...)
#endif

// Constructors and Destructors

ActionButtonSensor::ActionButtonSensor(IGpioProvider *gpioProvider) : gpioProvider_(gpioProvider)
{
}

// ISensor Interface Implementation

void ActionButtonSensor::Init()
{
    // Configure GPIO 32 as input with pull-down resistor
    // This ensures LOW state when button is not pressed (Normally Open button to 3.3V)
    gpioProvider_->PinMode(gpio_pins::INPUT_BUTTON, INPUT_PULLDOWN);

    log_d("ActionButtonSensor initialized on GPIO %d with pull-down", gpio_pins::INPUT_BUTTON);

    // Log initial state after proper configuration
    bool initialState = gpioProvider_->DigitalRead(gpio_pins::INPUT_BUTTON);
    log_i("GPIO %d initial state after configuration: %s", gpio_pins::INPUT_BUTTON,
          initialState ? "HIGH (pressed)" : "LOW (released)");
}

Reading ActionButtonSensor::GetReading()
{
    // Return button state as boolean (0 = not pressed, 1 = pressed)
    bool pressed = IsButtonPressed();
    return static_cast<int32_t>(pressed ? 1 : 0);
}

bool ActionButtonSensor::IsButtonPressed()
{
    // GPIO 32 reads HIGH when button is pressed (connected to 3.3V)
    return gpioProvider_->DigitalRead(gpio_pins::INPUT_BUTTON);
}