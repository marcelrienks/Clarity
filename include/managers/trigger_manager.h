#pragma once

#include "utilities/types.h"
#include "hardware/gpio_pins.h"
#include "interfaces/i_trigger.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <esp32-hal-log.h>
#include <map>
#include <memory>
#include <vector>
#include <string>

/**
 * @class TriggerManager
 * @brief Simplified direct GPIO polling trigger manager with priority-based alert evaluation
 * 
 * @architecture:
 * - Core 0: Direct GPIO polling and UI management
 * 
 * @key_simplifications:
 * 1. Direct GPIO polling - no interrupts or queues
 * 2. Pin change detection via state comparison
 * 3. Priority evaluation from lowest to highest (highest priority action wins)
 * 4. No cross-core communication needed
 */
class TriggerManager
{
public:
    TriggerManager(const TriggerManager &) = delete;
    TriggerManager &operator=(const TriggerManager &) = delete;
    static TriggerManager &GetInstance();

    // Core Functionality
    void init();
    void RegisterAllTriggers();
    void RegisterTrigger(std::unique_ptr<AlertTrigger> trigger);
    void ProcessTriggerEvents();
    void ExecuteTriggerAction(AlertTrigger* trigger, TriggerExecutionState state);
    void InitializeTriggersFromGpio();
    
    // Get startup panel override (null if no override needed)
    const char* GetStartupPanelOverride() const { return startupPanelOverride_; }


private:
    TriggerManager() = default;
    ~TriggerManager() = default;

    void setup_gpio_pins();
    void CheckGpioChanges();
    void CheckTriggerChange(const char* triggerId, bool currentPinState);
    void InitializeTrigger(const char* triggerId, bool currentPinState);
    AlertTrigger* FindTriggerById(const char* triggerId);
    void UpdateActiveTriggersList(AlertTrigger* trigger, TriggerExecutionState newState);

    // Trigger registry (Core 0 exclusive ownership - no mutex needed)
    std::vector<std::unique_ptr<AlertTrigger>> triggers_;
    
    // Single persistent list of currently active triggers (sorted by priority)
    std::vector<std::pair<TriggerPriority, AlertTrigger*>> activeTriggers_;
    
    // Startup panel override (set by InitializeTriggersFromGpio if active triggers require specific panel)
    const char* startupPanelOverride_ = nullptr;
    
};