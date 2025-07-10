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
    
    // For now, return true to always trigger (simplified implementation)
    // In a real implementation, this would check for lock state changes
    return true;
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
    
    // Get initial lock state
    _previous_state = std::get<bool>(_lock_sensor->get_reading());
    log_v("Lock trigger initialized with state: %s", _previous_state ? "engaged" : "disengaged");
}

bool LockTrigger::should_restore() const
{
    return _enable_restoration;
}