#pragma once

#include "utilities/types.h"
#include "hardware/gpio_pins.h"
#include "managers/panel_manager.h"
#include "managers/style_manager.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <esp32-hal-log.h>
#include <map>
#include <string>
#include <esp32-hal-gpio.h>
#include <utilities/types.h>

/**
 * @class TriggerManager
 * @brief Core 1 stateful trigger manager with shared state-based triggers
 */
class TriggerManager
{
public:
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
    std::pair<const char*, TriggerState*> GetHighestPriorityTrigger();

    // Core 1 Task Methods
    static void TriggerMonitoringTask(void* pvParameters);
    
    // Static interrupt handlers (public for extern C access)
    static void IRAM_ATTR keyPresentIsrHandler(void* arg);
    static void IRAM_ATTR keyNotPresentIsrHandler(void* arg);
    static void IRAM_ATTR lockStateIsrHandler(void* arg);
    static void IRAM_ATTR themeSwitchIsrHandler(void* arg);

private:
    // Constructors and Destructors
    TriggerManager() = default;
    ~TriggerManager() = default;

    // Shared State Management
    void SetTriggerState(const char* triggerId, const char* action, const char* target, TriggerPriority priority);
    
    // Helper methods for simplified logic
    void HandlePanelStateChange(bool state, const char* panelName, const char* triggerId, TriggerPriority priority);
    bool ShouldUpdateHighestPriority(const TriggerState& trigger, TriggerState* currentHighest, TriggerPriority currentPriority, uint64_t currentTimestamp);

public:
    void setup_gpio_interrupts();
    void ClearTriggerState(const char* triggerId);
    

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