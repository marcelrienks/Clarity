#include "triggers/lock_trigger.h"

// Constructors and Destructors
LockTrigger::LockTrigger(bool enable_restoration)
    : _lock_sensor(std::make_shared<LockSensor>()), 
      _enable_restoration(enable_restoration)
{
}

// Core Functionality Methods
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

const char* LockTrigger::get_id() const
{
    return TRIGGER_ID;
}

const char* LockTrigger::get_target_panel() const
{
    return PanelNames::Lock;
}

void LockTrigger::init()
{
    log_d("...");
    
    _lock_sensor->init();
    
    // Get initial lock state for logging
    bool initial_state = std::get<bool>(_lock_sensor->get_reading());
    log_d("Lock trigger initialized with state: %s", initial_state ? "engaged" : "disengaged");
}

bool LockTrigger::should_restore() const
{
    return _enable_restoration;
}