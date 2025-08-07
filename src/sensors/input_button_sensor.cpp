#include "sensors/input_button_sensor.h"
#include <Arduino.h>

#ifdef CLARITY_DEBUG
#include "esp32-hal-log.h"
#define LOG_TAG "InputButtonSensor"
#else
#define log_d(...)
#endif

// Constructors and Destructors

InputButtonSensor::InputButtonSensor(IGpioProvider* gpioProvider) 
    : gpioProvider_(gpioProvider)
{
}

// ISensor Interface Implementation

void InputButtonSensor::Init()
{
    // Initialize input button sensor on GPIO 34

    // Configure GPIO 34 as input (no pull-up/down needed - button provides 3.3V)
    gpioProvider_->PinMode(gpio_pins::INPUT_BUTTON, INPUT);
}

Reading InputButtonSensor::GetReading()
{
    // Return button state as boolean (0 = not pressed, 1 = pressed)
    bool pressed = IsButtonPressed();
    return static_cast<int32_t>(pressed ? 1 : 0);
}

bool InputButtonSensor::IsButtonPressed()
{
    // GPIO 34 reads HIGH when button is pressed (connected to 3.3V)
    return gpioProvider_->DigitalRead(gpio_pins::INPUT_BUTTON);
}