#include "triggers/lock_trigger.h"

// Constructors and Destructors

/// @brief Constructor with optional restoration mode
/// @param enable_restoration Whether to restore previous panel when lock becomes disengaged
LockTrigger::LockTrigger(bool enable_restoration)
    : _lock_sensor(std::make_shared<LockSensor>()), 
      _enable_restoration(enable_restoration)
{
}

// Core Functionality Methods

/// @brief Initialize the trigger and GPIO pins
void LockTrigger::init()
{
    log_d("...");
    
    _lock_sensor->init();
    
    // Get initial lock state for logging
    bool initial_state = std::get<bool>(_lock_sensor->get_reading());
    log_d("Lock trigger initialized with state: %s", initial_state ? "engaged" : "disengaged");
}

/// @brief Evaluate the trigger condition based on lock sensor reading
/// @return true if lock pin is HIGH (engaged), false if LOW (disengaged)
bool LockTrigger::evaluate()
{
    log_d("...");
    
    // Get current lock state from GPIO pin
    bool current_state = std::get<bool>(_lock_sensor->get_reading());
    
    // Simple logic: trigger when lock pin is HIGH (engaged)
    bool should_trigger = current_state;
    
    log_d("Lock state: %s, should_trigger: %d", 
          current_state ? "engaged" : "disengaged", 
          should_trigger);
    
    return should_trigger;
}

/// @brief Get the target panel name to switch to when triggered
/// @return Panel name for lock status display
const char* LockTrigger::get_target_panel() const
{
    return PanelNames::Lock;
}

/// @brief Get the trigger identifier for registration/management
/// @return Unique trigger identifier string
const char* LockTrigger::get_id() const
{
    return TRIGGER_ID;
}

/// @brief Whether to restore the previous panel when lock becomes disengaged
/// @return true if previous panel should be restored, false to stay on lock panel
bool LockTrigger::should_restore() const
{
    return _enable_restoration;
}