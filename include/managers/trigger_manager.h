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
 * @brief Simplified dual-core trigger manager with priority-based alert evaluation
 * 
 * @architecture:
 * - Core 1: Monitors GPIO pins, updates trigger active/inactive status
 * - Core 0: Evaluates triggers by priority, executes actions/restore functions
 * 
 * @key_simplifications:
 * 1. No complex state tracking - GPIO pin state directly controls trigger active status
 * 2. Priority evaluation from lowest to highest (highest priority action wins)
 * 3. Generic action/restore pattern via function objects
 * 4. Single mutex for trigger access, eliminating complex state management
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
    std::map<std::string, TriggerExecutionState> ProcessPendingTriggerEvents();
    ExecutionPlan PlanExecutionFromStates(const std::map<std::string, TriggerExecutionState>& consolidatedStates);
    void InitializeTriggersFromGpio();
    
    // Get startup panel override (null if no override needed)
    const char* GetStartupPanelOverride() const { return startupPanelOverride_; }

    // GPIO State Change Handlers (called from Core 1 task)
    void HandleGpioStateChange(const char* triggerId, bool pinState);

    // Core 1 Task Methods
    static void TriggerMonitoringTask(void* pvParameters);
    
    // CRITICAL: Get task handle for temporary suspension during LVGL operations
    TaskHandle_t GetTaskHandle() const { return triggerTaskHandle_; }
    
    // Static interrupt handlers
    static void IRAM_ATTR keyPresentIsrHandler(void* arg);
    static void IRAM_ATTR keyNotPresentIsrHandler(void* arg);
    static void IRAM_ATTR lockStateIsrHandler(void* arg);
    static void IRAM_ATTR themeSwitchIsrHandler(void* arg);

private:
    TriggerManager() = default;
    ~TriggerManager() = default;

    void setup_gpio_interrupts();
    AlertTrigger* FindTriggerById(const char* triggerId);
    void ExecuteHighestPriorityAction();
    void UpdateActiveTriggersList(AlertTrigger* trigger, TriggerExecutionState newState);

    // Trigger registry (Core 0 exclusive ownership - no mutex needed)
    std::vector<std::unique_ptr<AlertTrigger>> triggers_;
    
    // Single persistent list of currently active triggers (sorted by priority)
    std::vector<std::pair<TriggerPriority, AlertTrigger*>> activeTriggers_;
    
    // Startup panel override (set by InitializeTriggersFromGpio if active triggers require specific panel)
    const char* startupPanelOverride_ = nullptr;
    
    // ISR communication
    QueueHandle_t isrEventQueue = nullptr;
    TaskHandle_t triggerTaskHandle_ = nullptr;
};

// GPIO Interrupt Service Routine declarations
extern "C" {
    void IRAM_ATTR gpio_key_present_isr(void* arg);
    void IRAM_ATTR gpio_key_not_present_isr(void* arg);
    void IRAM_ATTR gpio_lock_state_isr(void* arg);
    void IRAM_ATTR gpio_theme_switch_isr(void* arg);
}