#include "sensors/key_sensor.h"

// Constructors and Destructors

/// @brief Constructor for KeySensor
KeySensor::KeySensor()
{
}

// Core Functionality Methods

/// @brief Initialize the key sensor hardware
void KeySensor::init()
{
    // Configure both GPIO pins for digital input (safe to call multiple times)
    log_d("Initializing key sensor on GPIO %d (key present) and GPIO %d (key not present)",
          GpioPins::KEY_PRESENT, GpioPins::KEY_NOT_PRESENT);

    pinMode(GpioPins::KEY_PRESENT, INPUT_PULLDOWN);
    pinMode(GpioPins::KEY_NOT_PRESENT, INPUT_PULLDOWN);
}

/// @brief Get the current key reading
/// @return KeyState indicating present, not present, or inactive
Reading KeySensor::get_reading()
{
    KeyState state;
    if (digitalRead(GpioPins::KEY_PRESENT))
    {
        state = KeyState::Present;
        log_d("Key state: Present (pin %d HIGH)", GpioPins::KEY_PRESENT);
    }
    else if (digitalRead(GpioPins::KEY_NOT_PRESENT))
    {
        state = KeyState::NotPresent;
        log_d("Key state: NotPresent (pin %d HIGH)", GpioPins::KEY_NOT_PRESENT);
    }
    else
    {
        state = KeyState::Inactive;
        log_d("Key state: Inactive (both pins LOW)");
    }

    return static_cast<int32_t>(state);
}