#include "sensors/key_sensor.h"
#include <Arduino.h>
#include <esp32-hal-log.h>

// Constructors and Destructors

/// @brief Constructor for KeySensor
KeySensor::KeySensor(IGpioProvider *gpioProvider) : gpioProvider_(gpioProvider)
{
    log_v("KeySensor() constructor called");
}

// Core Functionality Methods

/// @brief Initialize the key sensor hardware
void KeySensor::Init()
{
    log_v("Init() called");
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

    // Register coordinated interrupts instead of direct GPIO interrupts
    bool polledRegistered = RegisterPolledInterrupt(
        "key_state_monitor",          // Unique ID
        Priority::IMPORTANT,          // Important priority for key changes
        InterruptEffect::LOAD_PANEL,  // Could trigger panel changes
        150                          // 150ms polling for responsive key detection
    );
    
    if (polledRegistered)
    {
        log_d("Registered polled interrupt for key state monitoring");
    }
    else
    {
        log_w("Failed to register polled interrupt for key sensor");
    }
    
    log_i("KeySensor initialization completed on GPIO %d and %d with coordinated interrupts", 
          gpio_pins::KEY_PRESENT, gpio_pins::KEY_NOT_PRESENT);
}

/// @brief Get the current key reading
/// @return KeyState indicating present, not present, or inactive
Reading KeySensor::GetReading()
{
    log_v("GetReading() called");
    KeyState state = readKeyState();
    return static_cast<int32_t>(state);
}

/// @brief Get current key state directly (for panels)
/// @return Current KeyState based on GPIO readings
KeyState KeySensor::GetKeyState()
{
    log_v("GetKeyState() called");
    return readKeyState();
}

/// @brief Read GPIO pins and determine key state
/// @return KeyState based on GPIO pin readings
KeyState KeySensor::readKeyState()
{
    log_v("readKeyState() called");
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

bool KeySensor::HasStateChanged()
{
    log_v("HasStateChanged() called");
    
    KeyState currentState = readKeyState();
    bool changed = DetectChange(currentState, previousKeyState_);
    
    if (changed)
    {
        log_d("Key state changed from %d to %d", static_cast<int>(previousKeyState_), static_cast<int>(currentState));
    }
    
    return changed;
}

void KeySensor::OnInterruptTriggered()
{
    log_v("OnInterruptTriggered() called");
    
    KeyState currentState = readKeyState();
    log_i("Key sensor interrupt triggered - current state: %s", 
          currentState == KeyState::Present ? "Present" : 
          currentState == KeyState::NotPresent ? "NotPresent" : "Inactive");
    
    // Example custom behavior based on key state
    switch (currentState)
    {
        case KeyState::Present:
            log_i("Key inserted - system could activate panels");
            // Could trigger specific panel loading, theme changes, etc.
            break;
            
        case KeyState::NotPresent:
            log_i("Key removed - system could enter standby mode");
            // Could trigger security panels, screen locking, etc.
            break;
            
        case KeyState::Inactive:
            log_d("Key state inactive - no specific action");
            break;
    }
}