#include "triggers/key_trigger.h"
#include "utilities/types.h"
#include <esp32-hal-log.h>
#include <Arduino.h>

// Constructors and Destructors

/// @brief Constructor with optional restoration mode
/// @param enable_restoration Whether to restore previous panel when key becomes inactive
KeyTrigger::KeyTrigger(bool enable_restoration)
    : _enable_restoration(enable_restoration),
      _key_sensor(std::make_shared<KeySensor>()),
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
    auto reading = _key_sensor->get_reading();
    KeyState current_key_state = static_cast<KeyState>(std::get<int32_t>(reading));

    // Trigger if state has changed from inactive to active (present or not present)
    bool should_trigger = false;

    if (_last_key_state == KeyState::Inactive &&
        (current_key_state == KeyState::Present || current_key_state == KeyState::NotPresent))
    {
        // Key became active (either present or not present)
        should_trigger = true;
        log_d("Key state changed from Inactive to %s, triggering",
              current_key_state == KeyState::Present ? "Present" : "NotPresent");
    }
    else if ((_last_key_state == KeyState::Present || _last_key_state == KeyState::NotPresent) &&
             current_key_state == KeyState::Inactive && _enable_restoration)
    {
        // Key became inactive and restoration is enabled
        should_trigger = true;
        log_d("Key state changed to Inactive, triggering restoration");
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

/// @brief Initialize the trigger and key sensor
void KeyTrigger::init()
{
    log_d("...");

    // Initialize the key sensor (handles GPIO configuration)
    log_d("Initializing unified key trigger with key sensor");
    _key_sensor->init();

    // Initialize last state
    auto reading = _key_sensor->get_reading();
    _last_key_state = static_cast<KeyState>(std::get<int32_t>(reading));
    log_d("Initial key state: %d", static_cast<int>(_last_key_state));
}

/// @brief Whether to restore the previous panel when key becomes inactive
/// @return true if previous panel should be restored when key becomes inactive
bool KeyTrigger::should_restore() const
{
    return _enable_restoration;
}