#pragma once

#include <esp32-hal-log.h>
#include <utilities/types.h>
#include <memory>

/**
 * @interface ITrigger
 * @brief Interface for interrupt trigger conditions that can cause panel switches
 * 
 * @details Triggers monitor sensor readings and evaluate conditions to determine
 * when a panel switch should occur. Similar to ISensor pattern, triggers provide
 * a standard interface for condition evaluation with lifecycle management.
 * 
 * @lifecycle:
 * 1. init(): Initialize trigger resources and sensor connections
 * 2. evaluate(): Check current conditions and return trigger decision
 * 3. get_target_panel(): Return panel name to switch to when triggered
 * 4. should_restore(): Whether to restore previous panel when condition clears
 * 
 * @usage_examples:
 * - KeyTrigger: Monitor key sensor, trigger when key detected
 * - TemperatureTrigger: Monitor temperature, trigger on over-temperature
 * - PressureTrigger: Monitor pressure, trigger on low pressure warning
 * 
 * @integration: Triggers are registered with InterruptManager and evaluated
 * during the main loop before normal panel updates.
 */
class ITrigger
{
public:
    virtual ~ITrigger() = default;
    
    /// @brief Initialize the trigger and any required resources
    virtual void init() = 0;
    
    /// @brief Evaluate the trigger condition based on current sensor readings
    /// @return true if trigger condition is met and panel switch should occur
    virtual bool evaluate() = 0;
    
    /// @brief Get the target panel name to switch to when triggered
    /// @return Panel name string (e.g., PanelNames::Key)
    virtual const char* get_target_panel() const = 0;
    
    /// @brief Get the trigger identifier for registration/management
    /// @return Unique trigger identifier string
    virtual const char* get_id() const = 0;
    
    /// @brief Whether to restore the previous panel when condition clears
    /// @return true if previous panel should be restored, false to stay on target panel
    virtual bool should_restore() const { return false; }
};