#pragma once

// Project Includes
#include "utilities/types.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_sensor.h"

#include <functional>

/**
 * @interface ITriggerService
 * @brief Interface for GPIO trigger management and event processing
 * 
 * @details This interface abstracts trigger management functionality,
 * providing access to GPIO monitoring, trigger state management, and
 * trigger event processing. Implementations should handle initialization,
 * GPIO polling, trigger detection, and action execution.
 * 
 * @design_pattern Interface Segregation - Focused on trigger operations only
 * @testability Enables mocking for unit tests with simulated GPIO states
 * @dependency_injection Replaces direct TriggerManager singleton access
 * @gpio_dependency Requires IGpioProvider for hardware abstraction
 */
class ITriggerService
{
public:
    virtual ~ITriggerService() = default;

    // Core Functionality Methods
    
    /**
     * @brief Initialize the trigger service and setup GPIO pins
     */
    virtual void Init() = 0;

    /**
     * @brief Process trigger events by polling GPIO states and detecting changes
     * @details This method should be called regularly (typically in main loop)
     * to detect GPIO state changes and execute appropriate trigger actions
     */
    virtual void ProcessTriggerEvents() = 0;

    /**
     * @brief Add a new trigger with associated sensor and callback
     * @param triggerName Unique name for the trigger
     * @param sensor Sensor to monitor for trigger conditions
     * @param callback Function to call when trigger fires
     */
    virtual void AddTrigger(const std::string& triggerName, ISensor* sensor, std::function<void()> callback) = 0;

    /**
     * @brief Check if a trigger exists with the given name
     * @param triggerName Name of the trigger to check
     * @return true if trigger exists, false otherwise
     */
    virtual bool HasTrigger(const std::string& triggerName) const = 0;

    /**
     * @brief Execute a specific trigger action
     * @param mapping Pointer to trigger mapping configuration
     * @param state Execution state for the trigger
     */
    virtual void ExecuteTriggerAction(Trigger* mapping, TriggerExecutionState state) = 0;

    // Startup Configuration Methods
    
    /**
     * @brief Get startup panel override if active triggers require specific panel
     * @return Panel name string if override needed, nullptr otherwise
     * @details Called during application startup to determine if active triggers
     * require loading a specific panel instead of the default configuration panel
     */
    virtual const char* GetStartupPanelOverride() const = 0;
};