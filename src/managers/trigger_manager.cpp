#include "managers/trigger_manager.h"
#include "utilities/trigger_messages.h"
#include <esp32-hal-gpio.h>
#include <esp32-hal-log.h>
#include <string.h>
#include <utilities/types.h>

// Static Methods

TriggerManager &TriggerManager::get_instance()
{
    static TriggerManager instance;
    return instance;
}

// Core Functionality Methods

void TriggerManager::init()
{
    log_d("...");

    // Create priority queues
    _high_priority_queue = xQueueCreate(HIGH_PRIORITY_QUEUE_SIZE, sizeof(TriggerMessage));
    _medium_priority_queue = xQueueCreate(MEDIUM_PRIORITY_QUEUE_SIZE, sizeof(TriggerMessage));
    _low_priority_queue = xQueueCreate(LOW_PRIORITY_QUEUE_SIZE, sizeof(TriggerMessage));

    _isr_event_queue = xQueueCreate(10, sizeof(ISREvent));
    _state_mutex = xSemaphoreCreateMutex();

    setup_gpio_interrupts();

    xTaskCreatePinnedToCore(
        TriggerManager::trigger_monitoring_task, // Task function
        TriggerManager::TriggerMonitorTask,      // Task name
        4096,                                    // Stack size
        nullptr,                                 // Parameters
        configMAX_PRIORITIES - 1,                // Priority
        &_trigger_task_handle,                   // Task handle
        1                                        // Core 1 (PRO_CPU)
    );
}

void TriggerManager::handle_key_present_interrupt(bool key_present_state)
{
    log_d("...");

    // Thread-safe state access
    if (xSemaphoreTake(_state_mutex, pdMS_TO_TICKS(PANEL_STATE_MUTEX_TIMEOUT)) == pdTRUE)
    {
        if (key_present_state == true)
        {
            // Key present bulb is on - show KeyPanel with Present state
            if (_current_panel != PanelNames::Key)
            {
                post_message(ACTION_LOAD_PANEL, PanelNames::Key, TRIGGER_KEY_PRESENT, TriggerPriority::CRITICAL);
                _pending_messages[TRIGGER_KEY_PRESENT] = true;
            }
        }
        else
        {
            // Key not present bulb turned off - return to previous state
            if (_current_panel != PanelNames::Key)
            {
                // KeyPanel is showing - post restore message
                post_message(ACTION_RESTORE_PREVIOUS_PANEL, "", TRIGGER_KEY_PRESENT, TriggerPriority::NORMAL);
                _pending_messages[TRIGGER_KEY_PRESENT] = false;
            }
            else if (_pending_messages[TRIGGER_KEY_PRESENT])
            {
                // KeyPanel not showing but message is pending - remove message
                remove_message_from_queue(TRIGGER_KEY_PRESENT);
                _pending_messages[TRIGGER_KEY_PRESENT] = false;
            }
        }

        // Update hardware state
        _key_present_state = key_present_state;
        xSemaphoreGive(_state_mutex);
    }
}

void TriggerManager::handle_key_not_present_interrupt(bool key_not_present_state)
{
    log_d("...");

    // Thread-safe state access
    if (xSemaphoreTake(_state_mutex, pdMS_TO_TICKS(PANEL_STATE_MUTEX_TIMEOUT)) == pdTRUE)
    {
        if (key_not_present_state == true)
        {
            // Key not present bulb is on - show KeyPanel with NotPresent state
            if (_current_panel != PanelNames::Key)
            {
                post_message(ACTION_LOAD_PANEL, PanelNames::Key, TRIGGER_KEY_NOT_PRESENT, TriggerPriority::CRITICAL);
                _pending_messages[TRIGGER_KEY_NOT_PRESENT] = true;
            }
        }
        else
        {
            // Key not present bulb turned off - return to previous state
            if (_current_panel != PanelNames::Key)
            {
                // KeyPanel is showing - post restore message
                post_message(ACTION_RESTORE_PREVIOUS_PANEL, "", TRIGGER_KEY_NOT_PRESENT, TriggerPriority::NORMAL);
                _pending_messages[TRIGGER_KEY_NOT_PRESENT] = false;
            }
            else if (_pending_messages[TRIGGER_KEY_NOT_PRESENT])
            {
                // KeyPanel not showing but message is pending - remove message
                remove_message_from_queue(TRIGGER_KEY_NOT_PRESENT);
                _pending_messages[TRIGGER_KEY_NOT_PRESENT] = false;
            }
        }

        // Update hardware state
        _key_not_present_state = key_not_present_state;
        xSemaphoreGive(_state_mutex);
    }
}

void TriggerManager::handle_lock_state_interrupt(bool lock_engaged)
{
    log_d("...");

    // Thread-safe state access
    if (xSemaphoreTake(_state_mutex, pdMS_TO_TICKS(PANEL_STATE_MUTEX_TIMEOUT)) == pdTRUE)
    {
        if (lock_engaged == true)
        {
            // Lock engaged - check if LockPanel is already showing
            if (_current_panel != PanelNames::Lock)
            {
                post_message(ACTION_LOAD_PANEL, PanelNames::Lock, TRIGGER_LOCK_STATE, TriggerPriority::IMPORTANT);
                _pending_messages[TRIGGER_LOCK_STATE] = true;
            }
        }
        else
        {
            // Lock disengaged - check current state
            if (_current_panel == PanelNames::Lock)
            {
                // LockPanel is showing - post restore message
                post_message(ACTION_RESTORE_PREVIOUS_PANEL, "", TRIGGER_LOCK_STATE, TriggerPriority::NORMAL);
                _pending_messages[TRIGGER_LOCK_STATE] = false;
            }
            else if (_pending_messages[TRIGGER_LOCK_STATE])
            {
                // LockPanel not showing but message is pending - remove message
                remove_message_from_queue(TRIGGER_LOCK_STATE);
                _pending_messages[TRIGGER_LOCK_STATE] = false;
            }
        }

        // Update hardware state
        _lock_engaged_state = lock_engaged;
        xSemaphoreGive(_state_mutex);
    }
}

void TriggerManager::handle_theme_switch_interrupt(bool night_mode)
{
    log_d("...");

    // Thread-safe state access
    if (xSemaphoreTake(_state_mutex, pdMS_TO_TICKS(THEME_STATE_MUTEX_TIMEOUT)) == pdTRUE)
    {
        const char *target_theme = night_mode ? THEME_NIGHT : THEME_DAY;

        if (strcmp(_current_theme, target_theme) != 0)
        {
            if (_pending_messages[TRIGGER_THEME_SWITCH])
            {
                // Update existing message
                update_message_in_queue(TRIGGER_THEME_SWITCH, ACTION_CHANGE_THEME, target_theme);
            }
            else
            {
                // Post new message
                post_message(ACTION_CHANGE_THEME, target_theme, TRIGGER_THEME_SWITCH, TriggerPriority::NORMAL);
                _pending_messages[TRIGGER_THEME_SWITCH] = true;
            }
        }
        else
        {
            // Target theme already active
            if (_pending_messages[TRIGGER_THEME_SWITCH])
            {
                // Remove pending message - no change needed
                remove_message_from_queue(TRIGGER_THEME_SWITCH);
                _pending_messages[TRIGGER_THEME_SWITCH] = false;
            }
        }

        // Update hardware state
        _night_mode_state = night_mode;
        xSemaphoreGive(_state_mutex);
    }
}

void TriggerManager::update_application_state(const char* panel_name, const char* theme_name)
{
    log_d("...");

    if (xSemaphoreTake(_state_mutex, pdMS_TO_TICKS(PANEL_STATE_MUTEX_TIMEOUT)) == pdTRUE)
    {
        _current_panel = panel_name;
        _current_theme = theme_name;
        
        xSemaphoreGive(_state_mutex);
    }
}

// Task Methods

void TriggerManager::trigger_monitoring_task(void *pvParameters)
{
    log_d("...");

    TriggerManager &manager = TriggerManager::get_instance();
    ISREvent event;

    // Task loop - process ISR events safely
    while (1)
    {
        // Wait for ISR events from interrupt handlers
        if (xQueueReceive(manager._isr_event_queue, &event, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            // Process the event in task context (safe for logging, mutex, etc.)
            switch (event.event_type)
            {
            case ISREventType::KEY_PRESENT:
                manager.handle_key_present_interrupt(event.pin_state);
                break;

            case ISREventType::KEY_NOT_PRESENT:
                manager.handle_key_not_present_interrupt(event.pin_state);
                break;

            case ISREventType::LOCK_STATE_CHANGE:
                manager.handle_lock_state_interrupt(event.pin_state);
                break;

            case ISREventType::THEME_SWITCH:
                manager.handle_theme_switch_interrupt(event.pin_state);
                break;
            }
        }

        // Optional: Periodic state validation or cleanup
        // This could include checking for stale messages or state synchronization
    }
}

// Private Methods

void TriggerManager::post_message(const char *action, const char *target, const char *trigger_id, TriggerPriority priority)
{
    log_d("...");

    TriggerMessage msg;
    strcpy(msg.trigger_id, trigger_id);
    strcpy(msg.action, action);
    strcpy(msg.target, target);
    msg.priority = priority;
    msg.timestamp = esp_timer_get_time();

    QueueHandle_t target_queue = get_target_queue(priority);
    xQueueSend(target_queue, &msg, 0);
}

void TriggerManager::remove_message_from_queue(const char *trigger_id)
{
    // Note: FreeRTOS doesn't have built-in message removal by ID
    // This is a simplified implementation - a more sophisticated approach
    // would involve draining the queue, filtering, and re-posting
    log_d("...");

    // TODO: find a solution to this

    // For now, we rely on the pending_messages tracking to prevent
    // processing of messages that should be removed
    // A full implementation would drain and filter the queues
}

void TriggerManager::update_message_in_queue(const char *trigger_id, const char *action, const char *target)
{
    // Note: Similar to remove_message_from_queue, this is simplified
    // A full implementation would locate and update the specific message
    log_d("Marked trigger %s for message update - Action: %s, Target: %s", trigger_id, action, target);

    // TODO: find a solution to this
    //  For now, post a new message (which will be the latest)
    //  The pending_messages tracking helps prevent duplicate processing
    if (strcmp(action, ACTION_CHANGE_THEME) == 0)
    {
        post_message(action, target, trigger_id, TriggerPriority::NORMAL);
    }
}

// TODO: I feel like this is overly complicated
QueueHandle_t TriggerManager::get_target_queue(TriggerPriority priority)
{
    switch (priority)
    {
    case TriggerPriority::CRITICAL:
        return _high_priority_queue;
    case TriggerPriority::IMPORTANT:
        return _medium_priority_queue;
    case TriggerPriority::NORMAL:
        return _low_priority_queue;
    default:
        return _low_priority_queue; // Default fallback
    }
}

void TriggerManager::setup_gpio_interrupts()
{
    log_d("...");

    // Configure GPIO pins
    pinMode(GpioPins::KEY_PRESENT, INPUT_PULLDOWN);
    pinMode(GpioPins::KEY_NOT_PRESENT, INPUT_PULLDOWN);
    pinMode(GpioPins::LOCK, INPUT_PULLDOWN);

    // Setup interrupts for key present/not present (both edges)
    attachInterruptArg(GpioPins::KEY_PRESENT, key_present_isr_handler, (void *)true, CHANGE);
    attachInterruptArg(GpioPins::KEY_NOT_PRESENT, key_not_present_isr_handler, (void *)false, CHANGE);
    attachInterruptArg(GpioPins::LOCK, lock_state_isr_handler, nullptr, CHANGE);
}

// Static interrupt handlers

void IRAM_ATTR TriggerManager::key_present_isr_handler(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    bool pin_state = digitalRead(GpioPins::KEY_PRESENT);

    // Post event to task queue for safe processing
    ISREvent event(ISREventType::KEY_PRESENT, pin_state);
    xQueueSendFromISR(TriggerManager::get_instance()._isr_event_queue, &event, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void IRAM_ATTR TriggerManager::key_not_present_isr_handler(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    bool pin_state = digitalRead(GpioPins::KEY_NOT_PRESENT);

    // Pin HIGH means key not present bulb is ON
    ISREvent event(ISREventType::KEY_NOT_PRESENT, pin_state);
    xQueueSendFromISR(TriggerManager::get_instance()._isr_event_queue, &event, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void IRAM_ATTR TriggerManager::lock_state_isr_handler(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    bool pin_state = digitalRead(GpioPins::LOCK);

    ISREvent event(ISREventType::LOCK_STATE_CHANGE, pin_state);
    xQueueSendFromISR(TriggerManager::get_instance()._isr_event_queue, &event, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void IRAM_ATTR TriggerManager::theme_switch_isr_handler(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // For theme switch, we'll assume a different GPIO pin or logic
    // This is a placeholder for the actual implementation
    bool night_mode = false; // Read from appropriate GPIO

    ISREvent event(ISREventType::THEME_SWITCH, night_mode);
    xQueueSendFromISR(TriggerManager::get_instance()._isr_event_queue, &event, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// C-style ISR wrappers (required for ESP32)
extern "C"
{
    void IRAM_ATTR gpio_key_present_isr(void *arg)
    {
        TriggerManager::key_present_isr_handler(arg);
    }

    void IRAM_ATTR gpio_key_not_present_isr(void *arg)
    {
        TriggerManager::key_not_present_isr_handler(arg);
    }

    void IRAM_ATTR gpio_lock_state_isr(void *arg)
    {
        TriggerManager::lock_state_isr_handler(arg);
    }

    void IRAM_ATTR gpio_theme_switch_isr(void *arg)
    {
        TriggerManager::theme_switch_isr_handler(arg);
    }
}

// TODO: what is this actually doing?
void TriggerManager::get_queue_handles(QueueHandle_t *high_queue, QueueHandle_t *medium_queue, QueueHandle_t *low_queue)
{
    if (high_queue)
    {
        *high_queue = _high_priority_queue;
    }
    if (medium_queue)
    {
        *medium_queue = _medium_priority_queue;
    }
    if (low_queue)
    {
        *low_queue = _low_priority_queue;
    }
}
