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

    if (xSemaphoreTake(_state_mutex, pdMS_TO_TICKS(PANEL_STATE_MUTEX_TIMEOUT)) != pdTRUE)
    {
        return;
    }

    handle_panel_state_change(key_present_state, PanelNames::Key, TRIGGER_KEY_PRESENT, TriggerPriority::CRITICAL);
    _key_present_state = key_present_state;
    xSemaphoreGive(_state_mutex);
}

void TriggerManager::handle_key_not_present_interrupt(bool key_not_present_state)
{
    log_d("...");

    if (xSemaphoreTake(_state_mutex, pdMS_TO_TICKS(PANEL_STATE_MUTEX_TIMEOUT)) != pdTRUE)
    {
        return;
    }

    handle_panel_state_change(key_not_present_state, PanelNames::Key, TRIGGER_KEY_NOT_PRESENT, TriggerPriority::CRITICAL);
    _key_not_present_state = key_not_present_state;
    xSemaphoreGive(_state_mutex);
}

void TriggerManager::handle_lock_state_interrupt(bool lock_engaged)
{
    log_d("...");

    if (xSemaphoreTake(_state_mutex, pdMS_TO_TICKS(PANEL_STATE_MUTEX_TIMEOUT)) != pdTRUE)
    {
        return;
    }

    handle_panel_state_change(lock_engaged, PanelNames::Lock, TRIGGER_LOCK_STATE, TriggerPriority::IMPORTANT);
    _lock_engaged_state = lock_engaged;
    xSemaphoreGive(_state_mutex);
}

void TriggerManager::handle_theme_switch_interrupt(bool night_mode)
{
    log_d("...");

    if (xSemaphoreTake(_state_mutex, pdMS_TO_TICKS(THEME_STATE_MUTEX_TIMEOUT)) != pdTRUE)
    {
        return;
    }

    const char *target_theme = night_mode ? THEME_NIGHT : THEME_DAY;
    
    if (strcmp(_current_theme, target_theme) != 0)
    {
        set_trigger_state(TRIGGER_THEME_SWITCH, ACTION_CHANGE_THEME, target_theme, TriggerPriority::NORMAL);
    }
    else
    {
        clear_trigger_state(TRIGGER_THEME_SWITCH);
    }

    _night_mode_state = night_mode;
    xSemaphoreGive(_state_mutex);
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
    if (xSemaphoreTake(_trigger_mutex, pdMS_TO_TICKS(100)) != pdTRUE)
    {
        return nullptr;
    }

    TriggerState* highest = nullptr;
    TriggerPriority highest_priority = TriggerPriority::NORMAL;
    uint64_t oldest_timestamp = UINT64_MAX;
    
    for (auto& pair : _active_triggers)
    {
        TriggerState& trigger = pair.second;
        if (!trigger.active) continue;
        
        if (should_update_highest_priority(trigger, highest, highest_priority, oldest_timestamp))
        {
            highest = &trigger;
            highest_priority = trigger.priority;
            oldest_timestamp = trigger.timestamp;
        }
    }
    
    xSemaphoreGive(_trigger_mutex);
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

void TriggerManager::handle_panel_state_change(bool state, const char* panel_name, const char* trigger_id, TriggerPriority priority)
{
    if (state)
    {
        // Trigger is active - load panel if not already showing
        if (strcmp(_current_panel, panel_name) != 0)
        {
            set_trigger_state(trigger_id, ACTION_LOAD_PANEL, panel_name, priority);
        }
    }
    else
    {
        // Trigger is inactive - only act if we're currently showing this panel
        if (strcmp(_current_panel, panel_name) == 0)
        {
            // We're showing this panel - restore previous panel
            set_trigger_state(trigger_id, ACTION_RESTORE_PREVIOUS_PANEL, "", TriggerPriority::NORMAL);
        }
        // Don't clear pending triggers when state goes inactive - let them execute
    }
}

bool TriggerManager::should_update_highest_priority(const TriggerState& trigger, TriggerState* current_highest, TriggerPriority current_priority, uint64_t current_timestamp)
{
    if (current_highest == nullptr)
    {
        return true;
    }
    
    if (trigger.priority > current_priority)
    {
        return true;
    }
    
    if (trigger.priority == current_priority && trigger.timestamp < current_timestamp)
    {
        return true;
    }
    
    return false;
}

