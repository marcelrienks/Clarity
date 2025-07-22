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

    // Create ISR event queue and mutexes
    _isr_event_queue = xQueueCreate(10, sizeof(ISREvent));
    _state_mutex = xSemaphoreCreateMutex();
    _trigger_mutex = xSemaphoreCreateMutex();

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
        if (key_present_state == true) // Key present bulb is on
        {
            if (strcmp(_current_panel, PanelNames::Key) != 0)
            {
                // Load KeyPanel if not already showing
                set_trigger_state(TRIGGER_KEY_PRESENT, ACTION_LOAD_PANEL, PanelNames::Key, TriggerPriority::CRITICAL);
            }
        }
        else // Key present bulb is off
        {
            if (strcmp(_current_panel, PanelNames::Key) == 0)
            {
                // KeyPanel is showing - restore previous panel
                set_trigger_state(TRIGGER_KEY_PRESENT, ACTION_RESTORE_PREVIOUS_PANEL, "", TriggerPriority::NORMAL);
            }
            else
            {
                // KeyPanel not showing - clear any pending trigger
                clear_trigger_state(TRIGGER_KEY_PRESENT);
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
        if (key_not_present_state == true) // Key not present bulb is on
        {
            if (strcmp(_current_panel, PanelNames::Key) != 0)
            {
                // Load KeyPanel if not already showing
                set_trigger_state(TRIGGER_KEY_NOT_PRESENT, ACTION_LOAD_PANEL, PanelNames::Key, TriggerPriority::CRITICAL);
            }
        }
        else // Key not present bulb is off
        {
            if (strcmp(_current_panel, PanelNames::Key) == 0)
            {
                // KeyPanel is showing - restore previous panel
                set_trigger_state(TRIGGER_KEY_NOT_PRESENT, ACTION_RESTORE_PREVIOUS_PANEL, "", TriggerPriority::NORMAL);
            }
            else
            {
                // KeyPanel not showing - clear any pending trigger
                clear_trigger_state(TRIGGER_KEY_NOT_PRESENT);
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
        if (lock_engaged == true) // Lock is engaged
        {
            if (strcmp(_current_panel, PanelNames::Lock) != 0)
            {
                // Load LockPanel if not already showing
                set_trigger_state(TRIGGER_LOCK_STATE, ACTION_LOAD_PANEL, PanelNames::Lock, TriggerPriority::IMPORTANT);
            }
        }
        else // Lock is disengaged
        {
            if (strcmp(_current_panel, PanelNames::Lock) == 0)
            {
                // LockPanel is showing - restore previous panel
                set_trigger_state(TRIGGER_LOCK_STATE, ACTION_RESTORE_PREVIOUS_PANEL, "", TriggerPriority::NORMAL);
            }
            else
            {
                // LockPanel not showing - clear any pending trigger
                clear_trigger_state(TRIGGER_LOCK_STATE);
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

        if (strcmp(_current_theme, target_theme) != 0) // Theme change needed
        {
            // Set trigger state for theme change
            set_trigger_state(TRIGGER_THEME_SWITCH, ACTION_CHANGE_THEME, target_theme, TriggerPriority::NORMAL);
        }
        else // Target theme already active
        {
            // Clear any pending theme change trigger
            clear_trigger_state(TRIGGER_THEME_SWITCH);
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

void TriggerManager::set_trigger_state(const char *trigger_id, const char *action, const char *target, TriggerPriority priority)
{
    if (xSemaphoreTake(_trigger_mutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        _active_triggers[trigger_id] = TriggerState(action, target, priority, esp_timer_get_time());
        log_d("Set trigger %s: %s -> %s (priority %d)", trigger_id, action, target, (int)priority);
        xSemaphoreGive(_trigger_mutex);
    }
}

void TriggerManager::clear_trigger_state(const char *trigger_id)
{
    if (xSemaphoreTake(_trigger_mutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        auto it = _active_triggers.find(trigger_id);
        if (it != _active_triggers.end())
        {
            _active_triggers.erase(it);
            log_d("Cleared trigger %s", trigger_id);
        }
        xSemaphoreGive(_trigger_mutex);
    }
}

void TriggerManager::update_trigger_state(const char *trigger_id, const char *action, const char *target)
{
    if (xSemaphoreTake(_trigger_mutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        auto it = _active_triggers.find(trigger_id);
        if (it != _active_triggers.end())
        {
            it->second.action = action;
            it->second.target = target;
            it->second.timestamp = esp_timer_get_time();
            log_d("Updated trigger %s: %s -> %s", trigger_id, action, target);
        }
        xSemaphoreGive(_trigger_mutex);
    }
}

TriggerState* TriggerManager::get_highest_priority_trigger()
{
    TriggerState* highest = nullptr;
    TriggerPriority highest_priority = TriggerPriority::NORMAL;
    uint64_t oldest_timestamp = UINT64_MAX;
    
    if (xSemaphoreTake(_trigger_mutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        for (auto& pair : _active_triggers)
        {
            TriggerState& trigger = pair.second;
            if (!trigger.active) continue;
            
            // Priority comparison: CRITICAL > IMPORTANT > NORMAL
            bool is_higher_priority = false;
            if (highest == nullptr)
            {
                is_higher_priority = true;
            }
            else if (trigger.priority > highest_priority)
            {
                is_higher_priority = true;
            }
            else if (trigger.priority == highest_priority && trigger.timestamp < oldest_timestamp)
            {
                // Same priority, choose oldest
                is_higher_priority = true;
            }
            
            if (is_higher_priority)
            {
                highest = &trigger;
                highest_priority = trigger.priority;
                oldest_timestamp = trigger.timestamp;
            }
        }
        xSemaphoreGive(_trigger_mutex);
    }
    
    return highest;
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

TriggerState* TriggerManager::get_next_trigger_to_process()
{
    return get_highest_priority_trigger();
}
