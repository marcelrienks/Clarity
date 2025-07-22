#pragma once

#include "utilities/trigger_messages.h"
#include "utilities/types.h"
#include "hardware/gpio_pins.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <esp32-hal-log.h>
#include <map>
#include <string>

/**
 * @class TriggerManager
 * @brief Core 1 stateful trigger manager with application state awareness
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
    void get_queue_handles(QueueHandle_t* high_queue, QueueHandle_t* medium_queue, QueueHandle_t* low_queue);

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

    // Message Queue Management
    void post_message(const char* action, const char* target, const char* trigger_id, TriggerPriority priority);
    void remove_message_from_queue(const char* trigger_id);
    void update_message_in_queue(const char* trigger_id, const char* action, const char* target);
    QueueHandle_t get_target_queue(TriggerPriority priority);

    // GPIO Interrupt Setup
    void setup_gpio_interrupts();
    

    // Instance Data Members
    QueueHandle_t _high_priority_queue = nullptr;
    QueueHandle_t _medium_priority_queue = nullptr;
    QueueHandle_t _low_priority_queue = nullptr;
    QueueHandle_t _isr_event_queue = nullptr;       ///< Queue for ISR events to Core 1 task

    // Shared application state (protected by mutexes)
    SemaphoreHandle_t _state_mutex = nullptr;
    const char* _current_panel = PanelNames::Oil;
    const char* _current_theme = Themes::Day;

    // Pending message tracking
    std::map<std::string, bool> _pending_messages;

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