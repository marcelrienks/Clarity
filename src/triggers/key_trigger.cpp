#include "triggers/key_trigger.h"
#include "utilities/types.h"
#include "hardware/gpio_pins.h"
#include <esp32-hal-log.h>
#include <Arduino.h>

// Constructors and Destructors

/// @brief Constructor with optional restoration mode
/// @param enable_restoration Whether to restore previous panel when key becomes inactive
KeyTrigger::KeyTrigger(bool enable_restoration)
    : _enable_restoration(enable_restoration),
      _last_key_state(KeyState::Inactive)
{
}

// Core Functionality Methods

/// @brief Read key state directly from GPIO pins
/// @return Current key state
KeyState KeyTrigger::read_key_state()
{
    bool pin25_high = digitalRead(GpioPins::KEY_PRESENT);
    bool pin26_high = digitalRead(GpioPins::KEY_NOT_PRESENT);

    if (pin25_high && pin26_high)
    {
        // Both pins HIGH - invalid state, treat as inactive
        log_d("Key state: Invalid (both pins HIGH), treating as Inactive");
        return KeyState::Inactive;
    }
    else if (pin25_high)
    {
        log_d("Key state: Present (pin %d HIGH)", GpioPins::KEY_PRESENT);
        return KeyState::Present;
    }
    else if (pin26_high)
    {
        log_d("Key state: NotPresent (pin %d HIGH)", GpioPins::KEY_NOT_PRESENT);
        return KeyState::NotPresent;
    }
    else
    {
        log_d("Key state: Inactive (both pins LOW)");
        return KeyState::Inactive;
    }
}

/// @brief Evaluate the trigger condition based on key sensor reading
/// @return true if key state has changed and trigger should activate
bool KeyTrigger::evaluate()
{
    log_d("...");

    // Get current key state directly from GPIO
    KeyState current_key_state = read_key_state();

    // Simple logic: trigger if either pin 25 OR pin 26 is HIGH (but not both)
    bool should_trigger = false;

    if (current_key_state == KeyState::Present || current_key_state == KeyState::NotPresent)
    {
        // Pin 25 HIGH (Present) OR Pin 26 HIGH (NotPresent) - always trigger
        should_trigger = true;
        log_d("Key state: %s, triggering",
              current_key_state == KeyState::Present ? "Present" : "NotPresent");
    }
    else
    {
        // Both pins LOW (Inactive) - no trigger
        should_trigger = false;
        log_d("Key state: Inactive, no trigger");
    }

    // Update last state
    _last_key_state = current_key_state;

    log_d("current_key_state: %d, should_trigger: %d", static_cast<int>(current_key_state), should_trigger);

    return should_trigger;
}

/// @brief Get the trigger identifier
/// @return Unique trigger identifier string
const char *KeyTrigger::get_id() const
{
    return TRIGGER_ID;
}

/// @brief Get the target panel name to switch to when triggered
/// @return Panel name based on current key state
const char *KeyTrigger::get_target_panel() const
{
    // Always return KeyPanel for active states, restoration handled by InterruptManager
    return PanelNames::Key;
}

/// @brief Initialize the trigger and GPIO pins
void KeyTrigger::init()
{
    log_d("...");

    // Initialize GPIO pins for key detection with pull-down resistors
    log_d("Initializing key trigger on GPIO %d (key present) and GPIO %d (key not present)",
          GpioPins::KEY_PRESENT, GpioPins::KEY_NOT_PRESENT);
    
    pinMode(GpioPins::KEY_PRESENT, INPUT_PULLDOWN);
    pinMode(GpioPins::KEY_NOT_PRESENT, INPUT_PULLDOWN);

    // Initialize last state to Inactive to ensure trigger fires on first active state
    _last_key_state = KeyState::Inactive;
    KeyState current_state = read_key_state();
    log_d("Initial key state: %d (forcing last state to Inactive to enable triggering)", static_cast<int>(current_state));
}

/// @brief Whether to restore the previous panel when key becomes inactive
/// @return true if previous panel should be restored when key becomes inactive
bool KeyTrigger::should_restore() const
{
    return _enable_restoration;
}

/// @brief Get current key state (public interface for components)
/// @return Current key state as Reading variant
Reading KeyTrigger::get_reading()
{
    KeyState state = read_key_state();
    return static_cast<int32_t>(state);
}