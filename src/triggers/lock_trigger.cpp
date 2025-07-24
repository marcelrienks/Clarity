#include "triggers/lock_trigger.h"

// Constructors and Destructors

/// @brief Constructor with optional restoration mode
/// @param enable_restoration Whether to restore previous panel when lock becomes disengaged
LockTrigger::LockTrigger(bool enableRestoration)
    : lockSensor_(std::make_shared<LockSensor>()), 
      enableRestoration_(enableRestoration)
{
}

// Core Functionality Methods

/// @brief Initialize the trigger and GPIO pins
void LockTrigger::init()
{
    log_d("...");
    
    lockSensor_->init();
    
    // Get initial lock state for logging
    bool initialState = std::get<bool>(lockSensor_->GetReading());
    log_d("Lock trigger initialized with state: %s", initialState ? "engaged" : "disengaged");
}

/// @brief Evaluate the trigger condition based on lock sensor reading
/// @return true if lock pin is HIGH (engaged), false if LOW (disengaged)
bool LockTrigger::evaluate()
{
    log_d("...");
    
    // Get current lock state from GPIO pin
    bool currentState = std::get<bool>(lockSensor_->GetReading());
    
    // Simple logic: trigger when lock pin is HIGH (engaged)
    bool shouldTrigger = currentState;
    
    log_d("Lock state: %s, shouldTrigger: %d", 
          currentState ? "engaged" : "disengaged", 
          shouldTrigger);
    
    return shouldTrigger;
}

/// @brief Get the target panel name to switch to when triggered
/// @return Panel name for lock status display
const char* LockTrigger::GetTargetPanel() const
{
    return PanelNames::LOCK;
}

/// @brief Get the trigger identifier for registration/management
/// @return Unique trigger identifier string
const char* LockTrigger::GetId() const
{
    return TRIGGER_ID;
}

/// @brief Whether to restore the previous panel when lock becomes disengaged
/// @return true if previous panel should be restored, false to stay on lock panel
bool LockTrigger::ShouldRestore() const
{
    return enableRestoration_;
}