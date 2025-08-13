#include "sensors/key_sensor.h"
#include <Arduino.h>
#include <esp32-hal-log.h>

// Constructors and Destructors

/// @brief Constructor for KeySensor
KeySensor::KeySensor(IGpioProvider *gpioProvider) : gpioProvider_(gpioProvider)
{
}

// Core Functionality Methods

/// @brief Initialize the key sensor hardware
void KeySensor::Init()
{
    // Configure both GPIO pins for digital input (safe to call multiple times)
    static bool initialized = false;
    if (!initialized)
    {
        log_d("Initializing key sensor on GPIO %d (key present) and GPIO %d (key not present)", gpio_pins::KEY_PRESENT,
              gpio_pins::KEY_NOT_PRESENT);
        initialized = true;
    }

    gpioProvider_->PinMode(gpio_pins::KEY_PRESENT, INPUT_PULLDOWN);
    gpioProvider_->PinMode(gpio_pins::KEY_NOT_PRESENT, INPUT_PULLDOWN);

    // Attach interrupts for immediate state change detection
    gpioProvider_->AttachInterrupt(gpio_pins::KEY_PRESENT, nullptr, CHANGE);
    gpioProvider_->AttachInterrupt(gpio_pins::KEY_NOT_PRESENT, nullptr, CHANGE);
}

/// @brief Get the current key reading
/// @return KeyState indicating present, not present, or inactive
Reading KeySensor::GetReading()
{
    KeyState state = readKeyState();
    return static_cast<int32_t>(state);
}

/// @brief Get current key state directly (for panels)
/// @return Current KeyState based on GPIO readings
KeyState KeySensor::GetKeyState()
{
    return readKeyState();
}

/// @brief Read GPIO pins and determine key state
/// @return KeyState based on GPIO pin readings
KeyState KeySensor::readKeyState()
{
    bool pin25High = gpioProvider_->DigitalRead(gpio_pins::KEY_PRESENT);
    bool pin26High = gpioProvider_->DigitalRead(gpio_pins::KEY_NOT_PRESENT);

    if (pin25High && pin26High)
    {
        return KeyState::Inactive; // Both pins HIGH - invalid state
    }

    if (pin25High)
    {
        return KeyState::Present;
    }

    if (pin26High)
    {
        return KeyState::NotPresent;
    }

    return KeyState::Inactive; // Both pins LOW
}