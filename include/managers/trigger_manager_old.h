#pragma once

#include "utilities/trigger_messages.h"
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
 *
 * @details This manager runs on Core 1 and handles:
 * - Hardware interrupt detection and GPIO state monitoring
 * - Application state tracking (current panel and theme)
 * - Intelligent decision making for message posting/removal
 * - Multiple priority queue management
 * - State synchronization with Core 0
 *
 * @architectural_principles:
 * - Core 1 makes all decisions based on hardware state and application state
 * - Only posts messages when actual state change is needed
 * - Eliminates redundant processing by checking current state
 * - Manages multiple priority queues for different trigger types
 *
 * @state_awareness:
 * - Tracks current panel and theme from Core 0 notifications
 * - Maintains pending message state per trigger
 * - Compares hardware state with application state for decisions
 * - Thread-safe state access with mutex protection
 */
class TriggerManager
{
public:
    // Constructors and Destructors
    TriggerManager(const TriggerManager &) = delete;
    TriggerManager &operator=(const TriggerManager &) = delete;

    // Static Methods
    static TriggerManager &get_instance();

    // Core Functionality Methods
    /// @brief Initialize the dual-core trigger system
    void init_dual_core_system();

    /// @brief Handle key present hardware interrupt (Core 1)
    /// @param key_present Current hardware state of key presence
    void handle_key_present_interrupt(bool key_present);

    /// @brief Handle lock state hardware interrupt (Core 1)
    /// @param lock_engaged Current hardware state of lock engagement
    void handle_lock_state_interrupt(bool lock_engaged);

    /// @brief Handle theme switch hardware interrupt (Core 1)
    /// @param night_mode Current hardware state of theme switch
    void handle_theme_switch_interrupt(bool night_mode);

    /// @brief Update application state from Core 0 (thread-safe)
    /// @param panel_name Current panel name
    /// @param theme_name Current theme name
    void update_application_state(const char* panel_name, const char* theme_name);

    /// @brief Get application state for Core 0 synchronization (thread-safe)
    /// @param panel_name Output parameter for current panel
    /// @param theme_name Output parameter for current theme
    void get_application_state(char* panel_name, char* theme_name);\n    \n    /// @brief Get queue handles for Core 0 processing\n    /// @param high_queue Output parameter for high priority queue\n    /// @param medium_queue Output parameter for medium priority queue  \n    /// @param low_queue Output parameter for low priority queue\n    void get_queue_handles(QueueHandle_t* high_queue, QueueHandle_t* medium_queue, QueueHandle_t* low_queue);

    // Core 1 Task Methods
    /// @brief Core 1 main monitoring task
    /// @param pvParameters Task parameters (unused)
    static void trigger_monitoring_task(void* pvParameters);

private:
    // Constructors and Destructors
    TriggerManager() = default;
    ~TriggerManager() = default;

    // Message Queue Management
    /// @brief Post message to appropriate priority queue
    /// @param action Action to perform
    /// @param target Target panel/theme name
    /// @param trigger_id Trigger identifier
    /// @param priority Message priority level
    void post_message(const char* action, const char* target, const char* trigger_id, TriggerPriority priority);

    /// @brief Remove message from queue by trigger ID
    /// @param trigger_id Trigger identifier to remove
    void remove_message_from_queue(const char* trigger_id);

    /// @brief Update existing message in queue
    /// @param trigger_id Trigger identifier to update
    /// @param action New action
    /// @param target New target
    void update_message_in_queue(const char* trigger_id, const char* action, const char* target);

    /// @brief Get target queue for priority level
    /// @param priority Priority level
    /// @return Queue handle for the priority level
    QueueHandle_t get_target_queue(TriggerPriority priority);

    // GPIO Interrupt Setup
    /// @brief Setup GPIO interrupts for all triggers
    void setup_gpio_interrupts();

    // Static interrupt handlers (must be static for ESP32)
    static void IRAM_ATTR key_present_isr_handler(void* arg);
    static void IRAM_ATTR key_not_present_isr_handler(void* arg);
    static void IRAM_ATTR lock_state_isr_handler(void* arg);
    static void IRAM_ATTR theme_switch_isr_handler(void* arg);

    // Instance Data Members
    QueueHandle_t _high_priority_queue = nullptr;      ///< High priority message queue
    QueueHandle_t _medium_priority_queue = nullptr;    ///< Medium priority message queue
    QueueHandle_t _low_priority_queue = nullptr;       ///< Low priority message queue

    // Shared application state (protected by mutexes)
    SemaphoreHandle_t _state_mutex = nullptr;          ///< Mutex for application state
    char _current_panel[32] = {0};                     ///< Current panel name
    char _current_theme[32] = {0};                     ///< Current theme name

    // Pending message tracking
    std::map<std::string, bool> _pending_messages;     ///< Track queued messages per trigger

    // Hardware state tracking
    bool _key_present_state = false;                   ///< Current key present hardware state
    bool _lock_engaged_state = false;                  ///< Current lock engaged hardware state
    bool _night_mode_state = false;                    ///< Current theme switch hardware state

    // Core 1 task handle
    TaskHandle_t _trigger_task_handle = nullptr;       ///< Core 1 monitoring task handle
};

// GPIO Interrupt Service Routine declarations
extern "C" {
    void IRAM_ATTR gpio_key_present_isr(void* arg);
    void IRAM_ATTR gpio_key_not_present_isr(void* arg);
    void IRAM_ATTR gpio_lock_state_isr(void* arg);
    void IRAM_ATTR gpio_theme_switch_isr(void* arg);
}