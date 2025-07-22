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
struct TriggerState
{
    std::string action;           ///< Action to perform
    std::string target;           ///< Target of the action
    TriggerPriority priority;     ///< Priority level
    uint64_t timestamp;           ///< When trigger was activated
    bool active;                  ///< Whether trigger is currently active
    
    TriggerState() = default;
    TriggerState(const char* act, const char* tgt, TriggerPriority prio, uint64_t ts)
        : action(act), target(tgt), priority(prio), timestamp(ts), active(true) {}
};

/**
 * @class TriggerManager
 * @brief Core 1 stateful trigger manager with shared state-based triggers
 */
class TriggerManager
{
public:
    const char * TriggerMonitorTask = "TriggerMonitorTask";

    // Constructors and Destructors
    TriggerManager(const TriggerManager &) = delete;
    TriggerManager &operator=(const TriggerManager &) = delete;

    // Static Methods
    static TriggerManager &get_instance();

    // Core Functionality Methods
    void init();
    void handle_key_present_interrupt(bool key_present);
    void handle_key_not_present_interrupt(bool key_not_present);
    void handle_lock_state_interrupt(bool lock_engaged);
    void handle_theme_switch_interrupt(bool night_mode);
    void update_application_state(const char* panel_name, const char* theme_name);
    TriggerState* get_highest_priority_trigger();
    void clear_trigger_state_public(const char* trigger_id) { clear_trigger_state(trigger_id); }

    // Core 1 Task Methods
    static void trigger_monitoring_task(void* pvParameters);
    
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
    void set_trigger_state(const char* trigger_id, const char* action, const char* target, TriggerPriority priority);
    void clear_trigger_state(const char* trigger_id);
    void update_trigger_state(const char* trigger_id, const char* action, const char* target);
    
    // Helper methods for simplified logic
    void handle_panel_state_change(bool state, const char* panel_name, const char* trigger_id, TriggerPriority priority);
    bool should_update_highest_priority(const TriggerState& trigger, TriggerState* current_highest, TriggerPriority current_priority, uint64_t current_timestamp);

public:

    // GPIO Interrupt Setup
    void setup_gpio_interrupts();
    

    // Instance Data Members
    QueueHandle_t _isr_event_queue = nullptr;       ///< Queue for ISR events to Core 1 task

    // Shared application state (protected by mutexes)
    SemaphoreHandle_t _state_mutex = nullptr;
    const char* _current_panel = PanelNames::Oil;
    const char* _current_theme = Themes::Day;

    // Shared trigger state (protected by trigger_mutex)
    std::map<std::string, TriggerState> _active_triggers;
    SemaphoreHandle_t _trigger_mutex = nullptr;

    // Hardware state tracking
    bool _key_present_state = false;
    bool _key_not_present_state = false;
    bool _lock_engaged_state = false;
    bool _night_mode_state = false;

    // Core 1 task handle
    TaskHandle_t _trigger_task_handle = nullptr;
};

// GPIO Interrupt Service Routine declarations
extern "C" {
    void IRAM_ATTR gpio_key_present_isr(void* arg);
    void IRAM_ATTR gpio_key_not_present_isr(void* arg);
    void IRAM_ATTR gpio_lock_state_isr(void* arg);
    void IRAM_ATTR gpio_theme_switch_isr(void* arg);
}