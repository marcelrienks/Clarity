#include "sensors/key_sensor.h"
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
    KeyState state = readKeyState();
    return static_cast<int32_t>(state);
}

/// @brief Get current key state directly (for panels)
/// @return Current KeyState based on GPIO readings
KeyState KeySensor::getKeyState()
{
    return readKeyState();
}

/// @brief Read GPIO pins and determine key state
/// @return KeyState based on GPIO pin readings
KeyState KeySensor::readKeyState()
{
    bool pin25High = gpioProvider_->digitalRead(gpio_pins::KEY_PRESENT);
    bool pin26High = gpioProvider_->digitalRead(gpio_pins::KEY_NOT_PRESENT);
    
    if (pin25High && pin26High) {
        return KeyState::Inactive; // Both pins HIGH - invalid state
    }
    
    if (pin25High) {
        return KeyState::Present;
    }
    
    if (pin26High) {
        return KeyState::NotPresent;
    }
    
    return KeyState::Inactive; // Both pins LOW
}