#include "sensors/key_sensor.h"
#include "utilities/reading_helper.h"
#include <Arduino.h>
#include <esp32-hal-log.h>

// Constructors and Destructors

/// @brief Constructor for KeySensor
KeySensor::KeySensor(IGpioProvider* gpioProvider) : gpioProvider_(gpioProvider)
{
}

// Core Functionality Methods

/// @brief Initialize the key sensor hardware
void KeySensor::init()
{
    // Configure both GPIO pins for digital input (safe to call multiple times)
    log_d("Initializing key sensor on GPIO %d (key present) and GPIO %d (key not present)",
          gpio_pins::KEY_PRESENT, gpio_pins::KEY_NOT_PRESENT);

    gpioProvider_->pinMode(gpio_pins::KEY_PRESENT, INPUT_PULLDOWN);
    gpioProvider_->pinMode(gpio_pins::KEY_NOT_PRESENT, INPUT_PULLDOWN);
}

/// @brief Get the current key reading
/// @return KeyState indicating present, not present, or inactive
Reading KeySensor::getReading()
{
    // Use injected GPIO provider for consistent hardware abstraction
    KeyState state = ReadingHelper::readKeyState(gpioProvider_);
    return static_cast<int32_t>(state);
}