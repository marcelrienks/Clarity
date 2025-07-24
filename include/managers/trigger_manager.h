#pragma once

#include "utilities/trigger_messages.h"
#include "utilities/types.h"
#include "hardware/gpio_pins.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <esp32-hal-log.h>
#include <map>
#include <string>

/**
 * @brief Structure for shared trigger state
 * 
 * @details This structure represents the state of an active trigger,
 * including its action, target, priority, and timing information.
 */
struct TriggerState//TODO: inconsistency between string and const char* usage
{
    std::string action;           ///< Action to perform
    std::string target;           ///< Target of the action
    TriggerPriority priority;     ///< Priority level
    uint64_t timestamp;           ///< When trigger was activated
    bool active;                  ///< Whether trigger is currently active
    
    TriggerState() = default;
    TriggerState(const char* act, const char* tgt, TriggerPriority prio, uint64_t ts)//TODO: remove abbreviations
        : action(act), target(tgt), priority(prio), timestamp(ts), active(true) {}
};

/**
 * @class TriggerManager
 * @brief Core 1 stateful trigger manager with shared state-based triggers
 */
class TriggerManager
{
public:
//TODO: remove all leading/trailing underscores from variable names across the project
//TODO: all constant variables should be uppercase
    const char * TRIGGER_MONITOR_TASK = "TriggerMonitorTask";

    // Constructors and Destructors
    TriggerManager(const TriggerManager &) = delete;
    TriggerManager &operator=(const TriggerManager &) = delete;

    // Static Methods
    static TriggerManager &GetInstance();

    // Core Functionality Methods
    void init();
    void HandleKeyPresentInterrupt(bool keyPresent);
    void HandleKeyNotPresentInterrupt(bool keyNotPresent);
    void HandleLockStateInterrupt(bool lockEngaged);
    void HandleThemeSwitchInterrupt(bool nightMode);
    void NotifyApplicationStateUpdated();
    TriggerState* GetHighestPriorityTrigger();
    void ClearTriggerStatePublic(const char* triggerId) { ClearTriggerState(triggerId); }

    // Core 1 Task Methods
    static void TriggerMonitoringTask(void* pvParameters);
    
    // Static interrupt handlers (public for extern C access)
    static void IRAM_ATTR key_present_isr_handler(void* arg);
    static void IRAM_ATTR key_not_present_isr_handler(void* arg);
    static void IRAM_ATTR lock_state_isr_handler(void* arg);
    static void IRAM_ATTR theme_switch_isr_handler(void* arg);

private:
    // Constructors and Destructors
    TriggerManager() = default;
    ~TriggerManager() = default;

    // Shared State Management
    void SetTriggerState(const char* triggerId, const char* action, const char* target, TriggerPriority priority);
    void ClearTriggerState(const char* triggerId);
    void UpdateTriggerState(const char* triggerId, const char* action, const char* target);
    
    // Helper methods for simplified logic
    void HandlePanelStateChange(bool state, const char* panelName, const char* triggerId, TriggerPriority priority);
    bool ShouldUpdateHighestPriority(const TriggerState& trigger, TriggerState* currentHighest, TriggerPriority currentPriority, uint64_t currentTimestamp);

public:

    // GPIO Interrupt Setup
    void setup_gpio_interrupts();
    

    // Instance Data Members
    QueueHandle_t isrEventQueue = nullptr;       ///< Queue for ISR events to Core 1 task

    // Shared application state (protected by mutexes)
    SemaphoreHandle_t stateMutex = nullptr;

    // Shared trigger state (protected by trigger_mutex)
    std::map<const char*, TriggerState> activeTriggers_;
    SemaphoreHandle_t triggerMutex_ = nullptr;

    // Hardware state tracking
    bool keyPresentState_ = false;
    bool keyNotPresentState_ = false;
    bool lockEngagedState_ = false;
    bool nightModeState_ = false;

    // Core 1 task handle
    TaskHandle_t triggerTaskHandle_ = nullptr;
};

// GPIO Interrupt Service Routine declarations
extern "C" {
    void IRAM_ATTR gpio_key_present_isr(void* arg);
    void IRAM_ATTR gpio_key_not_present_isr(void* arg);
    void IRAM_ATTR gpio_lock_state_isr(void* arg);
    void IRAM_ATTR gpio_theme_switch_isr(void* arg);
}