#include "triggers/key_trigger.h"
#include "utilities/types.h"
#include <esp32-hal-log.h>
#include <variant>

// Constructors and Destructors

/// @brief Constructor with optional restoration mode
/// @param enable_restoration Whether to restore previous panel when key is removed
KeyTrigger::KeyTrigger(bool enable_restoration)
    : _enable_restoration(enable_restoration)
{
    _key_sensor = std::make_shared<KeySensor>();
}

// Core Functionality Methods

/// @brief Evaluate the trigger condition based on key sensor reading
/// @return true if key is present and trigger should activate
bool KeyTrigger::evaluate()
{
    return true;
}

/// @brief Get the trigger identifier
/// @return Unique trigger identifier string
const char *KeyTrigger::get_id() const
{
    return TRIGGER_ID;
}

/// @brief Get the target panel name to switch to when triggered
/// @return Panel name for key monitoring (KeyPanel)
const char *KeyTrigger::get_target_panel() const
{
    return PanelNames::Key;
}

/// @brief Initialize the trigger and key sensor
void KeyTrigger::init()
{
    log_d("...");

    _key_sensor->init();

    // Get initial state to avoid false triggers on startup
    Reading initial_reading = _key_sensor->get_reading();
    if (std::holds_alternative<bool>(initial_reading))
    {
        _previous_state = std::get<bool>(initial_reading);
    }
}

/// @brief Whether to restore the previous panel when key is removed
/// @return true if previous panel should be restored when condition clears
bool KeyTrigger::should_restore() const
{
    return _enable_restoration;
}