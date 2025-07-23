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
          gpio_pins::KEY_PRESENT, gpio_pins::KEY_NOT_PRESENT);

    pinMode(gpio_pins::KEY_PRESENT, INPUT_PULLDOWN);
    pinMode(gpio_pins::KEY_NOT_PRESENT, INPUT_PULLDOWN);
}

/// @brief Get the current key reading
/// @return KeyState indicating present, not present, or inactive
Reading KeySensor::get_reading()
{
    bool pin25High = digitalRead(gpio_pins::KEY_PRESENT);
    bool pin26High = digitalRead(gpio_pins::KEY_NOT_PRESENT);
    
    KeyState state = determine_key_state(pin25High, pin26High);
    log_key_state(state, pin25High, pin26High);
    
    return static_cast<int32_t>(state);
}

KeyState KeySensor::determine_key_state(bool pin25High, bool pin26High)
{
    if (pin25High && pin26High)
    {
        return KeyState::Inactive;  // Invalid state - both pins HIGH
    }
    
    if (pin25High)
    {
        return KeyState::Present;
    }
    
    if (pin26High)
    {
        return KeyState::NotPresent;
    }
    
    return KeyState::Inactive;  // Both pins LOW
}

void KeySensor::log_key_state(KeyState state, bool pin25High, bool pin26High)
{
    switch (state)
    {
        case KeyState::Inactive:
            if (pin25High && pin26High)
            {
                log_w("Key state: Invalid (both pins HIGH), treating as Inactive");
            }
            else
            {
                log_d("Key state: Inactive (both pins LOW)");
            }
            break;
            
        case KeyState::Present:
            log_d("Key state: Present (pin %d HIGH)", gpio_pins::KEY_PRESENT);
            break;
            
        case KeyState::NotPresent:
            log_d("Key state: NotPresent (pin %d HIGH)", gpio_pins::KEY_NOT_PRESENT);
            break;
    }
}