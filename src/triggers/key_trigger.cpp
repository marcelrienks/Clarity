#include "triggers/key_trigger.h"
#include "utilities/types.h"
#include <esp32-hal-log.h>

// Constructors and Destructors

/// @brief Constructor with optional restoration mode
/// @param enable_restoration Whether to restore previous panel when key becomes inactive
KeyTrigger::KeyTrigger(bool enable_restoration)
    : _enable_restoration(enable_restoration),
      _last_key_state(KeyState::Inactive)
{
}

// Core Functionality Methods


/// @brief Evaluate the trigger condition based on key sensor reading
/// @return true if key state has changed and trigger should activate
bool KeyTrigger::evaluate()
{
    log_d("...");

    // Get current key state from sensor
    Reading sensor_reading = _key_sensor.get_reading();
    KeyState current_key_state = static_cast<KeyState>(std::get<int32_t>(sensor_reading));

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
const char* KeyTrigger::get_target_panel() const
{
    // Always return KeyPanel for active states, restoration handled by InterruptManager
    return PanelNames::Key;
}

/// @brief Initialize the trigger and key sensor
void KeyTrigger::init()
{
    log_d("...");

    // Initialize the key sensor
    _key_sensor.init();

    // Initialize last state to Inactive to ensure trigger fires on first active state
    _last_key_state = KeyState::Inactive;
    Reading current_reading = _key_sensor.get_reading();
    KeyState current_state = static_cast<KeyState>(std::get<int32_t>(current_reading));
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
    return _key_sensor.get_reading();
}