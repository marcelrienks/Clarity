#pragma once

#include <esp32-hal-log.h>
#include <utilities/types.h>
#include <memory>
#include <functional>

/**
 * @interface ITrigger
 * @brief Simplified trigger interface with action/restore pattern
 * 
 * @details Triggers represent alert conditions with active/inactive states.
 * Each trigger has an action (executed when active) and restore (executed when inactive).
 * State changes in GPIO pins directly control trigger active/inactive status.
 * 
 * @key_concepts:
 * - Active/Inactive: All triggers have binary state based on GPIO pin state
 * - Action: Function executed when trigger becomes active
 * - Restore: Function executed when trigger becomes inactive  
 * - Priority: Triggers are evaluated lowest to highest priority (highest wins)
 * 
 * @simplified_flow:
 * 1. GPIO pin state change -> trigger active/inactive
 * 2. Action executed on active, restore executed on inactive
 * 3. Multiple active triggers: highest priority action wins
 */
class ITrigger
{
public:
    virtual ~ITrigger() = default;
    
    // Core Methods
    virtual void init() = 0;
    virtual const char* GetId() const = 0;
    virtual TriggerPriority GetPriority() const = 0;
    virtual TriggerExecutionState GetState() const = 0;
    
    // Action/Restore Pattern - returns requests instead of executing
    virtual TriggerActionRequest GetActionRequest() = 0;
    virtual TriggerActionRequest GetRestoreRequest() = 0;
    
    // State Management
    virtual void SetState(TriggerExecutionState state) = 0;
};

// Base class for alert triggers with request-based actions
class AlertTrigger : public ITrigger
{
protected:
    TriggerExecutionState state_ = TriggerExecutionState::INIT;
    TriggerPriority priority_;
    const char* id_;

public:
    AlertTrigger(const char* id, TriggerPriority priority)
        : id_(id), priority_(priority) {}

    const char* GetId() const override { return id_; }
    TriggerPriority GetPriority() const override { return priority_; }
    TriggerExecutionState GetState() const override { return state_; }
    
    void SetState(TriggerExecutionState state) override { 
        state_ = state;
        const char* stateStr = (state == TriggerExecutionState::INIT) ? "INIT" : 
                              (state == TriggerExecutionState::ACTIVE) ? "ACTIVE" : "INACTIVE";
        log_d("Trigger %s set to %s", id_, stateStr);
    }
};