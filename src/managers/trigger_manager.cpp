#include "managers/trigger_manager.h"
#include "managers/panel_manager.h"
#include "managers/style_manager.h"
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
    isrEventQueue = xQueueCreate(10, sizeof(ISREvent));
    stateMutex = xSemaphoreCreateMutex();
    triggerMutex = xSemaphoreCreateMutex();

    setup_gpio_interrupts();

    xTaskCreatePinnedToCore(
        TriggerManager::trigger_monitoring_task, // Task function
        TriggerManager::get_instance().TRIGGER_MONITOR_TASK,      // Task name
        4096,                                    // Stack size
        nullptr,                                 // Parameters
        configMAX_PRIORITIES - 1,                // Priority
        &triggerTaskHandle,                   // Task handle
        1                                        // Core 1 (PRO_CPU)
    );
}

void TriggerManager::handle_key_present_interrupt(bool key_present_state)
{
    log_d("...");

    if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(PANEL_STATE_MUTEX_TIMEOUT)) != pdTRUE)
    {
        return;
    }

    handle_panel_state_change(key_present_state, PanelNames::KEY, TRIGGER_KEY_PRESENT, TriggerPriority::CRITICAL);
    keyPresentState = key_present_state;
    xSemaphoreGive(stateMutex);
}

void TriggerManager::handle_key_not_present_interrupt(bool key_not_present_state)
{
    log_d("...");

    if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(PANEL_STATE_MUTEX_TIMEOUT)) != pdTRUE)
    {
        return;
    }

    handle_panel_state_change(key_not_present_state, PanelNames::KEY, TRIGGER_KEY_NOT_PRESENT, TriggerPriority::CRITICAL);
    keyNotPresentState = key_not_present_state;
    xSemaphoreGive(stateMutex);
}

void TriggerManager::handle_lock_state_interrupt(bool lock_engaged)
{
    log_d("...");

    if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(PANEL_STATE_MUTEX_TIMEOUT)) != pdTRUE)
    {
        return;
    }

    handle_panel_state_change(lock_engaged, PanelNames::LOCK, TRIGGER_LOCK_STATE, TriggerPriority::IMPORTANT);
    lockEngagedState = lock_engaged;
    xSemaphoreGive(stateMutex);
}

void TriggerManager::handle_theme_switch_interrupt(bool night_mode)
{
    log_d("...");

    if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(THEME_STATE_MUTEX_TIMEOUT)) != pdTRUE)
    {
        return;
    }

    const char *target_theme = night_mode ? THEME_NIGHT : THEME_DAY;
    const char *current_theme = StyleManager::get_instance().THEME;
    
    if (strcmp(current_theme, target_theme) != 0)
    {
        set_trigger_state(TRIGGER_THEME_SWITCH, ACTION_CHANGE_THEME, target_theme, TriggerPriority::NORMAL);
    }
    else
    {
        clear_trigger_state(TRIGGER_THEME_SWITCH);
    }

    nightModeState = night_mode;
    xSemaphoreGive(stateMutex);
}

void TriggerManager::notify_application_state_updated()
{
    log_d("Application state updated - TriggerManager notified");
    // This method serves as a notification point for future functionality
    // Currently no action needed as PanelManager is the source of truth
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
        if (xQueueReceive(manager.isrEventQueue, &event, pdMS_TO_TICKS(100)) == pdTRUE)
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
    if (xSemaphoreTake(triggerMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        activeTriggers[trigger_id] = TriggerState(action, target, priority, esp_timer_get_time());
        log_d("Set trigger %s: %s -> %s (priority %d)", trigger_id, action, target, (int)priority);
        xSemaphoreGive(triggerMutex);
    }
}

void TriggerManager::clear_trigger_state(const char *trigger_id)
{
    if (xSemaphoreTake(triggerMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        auto it = activeTriggers.find(trigger_id);
        if (it != activeTriggers.end())
        {
            activeTriggers.erase(it);
            log_d("Cleared trigger %s", trigger_id);
        }
        xSemaphoreGive(triggerMutex);
    }
}

void TriggerManager::update_trigger_state(const char *trigger_id, const char *action, const char *target)
{
    if (xSemaphoreTake(triggerMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        auto it = activeTriggers.find(trigger_id);
        if (it != activeTriggers.end())
        {
            it->second.action = action;
            it->second.target = target;
            it->second.timestamp = esp_timer_get_time();
            log_d("Updated trigger %s: %s -> %s", trigger_id, action, target);
        }
        xSemaphoreGive(triggerMutex);
    }
}

TriggerState* TriggerManager::get_highest_priority_trigger()
{
    if (xSemaphoreTake(triggerMutex, pdMS_TO_TICKS(100)) != pdTRUE)
    {
        return nullptr;
    }

    TriggerState* highest = nullptr;
    TriggerPriority highestPriority = TriggerPriority::NORMAL;
    uint64_t oldestTimestamp = UINT64_MAX;
    
    for (auto& pair : activeTriggers)
    {
        TriggerState& trigger = pair.second;
        if (!trigger.active) continue;
        
        if (should_update_highest_priority(trigger, highest, highestPriority, oldestTimestamp))
        {
            highest = &trigger;
            highestPriority = trigger.priority;
            oldestTimestamp = trigger.timestamp;
        }
    }
    
    xSemaphoreGive(triggerMutex);
    return highest;
}

void TriggerManager::setup_gpio_interrupts()
{
    log_d("...");

    // Configure GPIO pins
    pinMode(gpio_pins::KEY_PRESENT, INPUT_PULLDOWN);
    pinMode(gpio_pins::KEY_NOT_PRESENT, INPUT_PULLDOWN);
    pinMode(gpio_pins::LOCK, INPUT_PULLDOWN);

    // Setup interrupts for key present/not present (both edges)
    attachInterruptArg(gpio_pins::KEY_PRESENT, key_present_isr_handler, (void *)true, CHANGE);
    attachInterruptArg(gpio_pins::KEY_NOT_PRESENT, key_not_present_isr_handler, (void *)false, CHANGE);
    attachInterruptArg(gpio_pins::LOCK, lock_state_isr_handler, nullptr, CHANGE);
}

// Static interrupt handlers

void IRAM_ATTR TriggerManager::key_present_isr_handler(void *arg)
{
    BaseType_t x_higher_priority_task_woken = pdFALSE;
    bool pinState = digitalRead(gpio_pins::KEY_PRESENT);

    // Post event to task queue for safe processing
    ISREvent event(ISREventType::KEY_PRESENT, pinState);
    xQueueSendFromISR(TriggerManager::get_instance().isrEventQueue, &event, &x_higher_priority_task_woken);

    portYIELD_FROM_ISR(x_higher_priority_task_woken);
}

void IRAM_ATTR TriggerManager::key_not_present_isr_handler(void *arg)
{
    BaseType_t x_higher_priority_task_woken = pdFALSE;
    bool pinState = digitalRead(gpio_pins::KEY_NOT_PRESENT);

    // Pin HIGH means key not present bulb is ON
    ISREvent event(ISREventType::KEY_NOT_PRESENT, pinState);
    xQueueSendFromISR(TriggerManager::get_instance().isrEventQueue, &event, &x_higher_priority_task_woken);

    portYIELD_FROM_ISR(x_higher_priority_task_woken);
}

void IRAM_ATTR TriggerManager::lock_state_isr_handler(void *arg)
{
    BaseType_t x_higher_priority_task_woken = pdFALSE;
    bool pinState = digitalRead(gpio_pins::LOCK);

    ISREvent event(ISREventType::LOCK_STATE_CHANGE, pinState);
    xQueueSendFromISR(TriggerManager::get_instance().isrEventQueue, &event, &x_higher_priority_task_woken);

    portYIELD_FROM_ISR(x_higher_priority_task_woken);
}

void IRAM_ATTR TriggerManager::theme_switch_isr_handler(void *arg)
{
    BaseType_t x_higher_priority_task_woken = pdFALSE;

    // For theme switch, we'll assume a different GPIO pin or logic
    // This is a placeholder for the actual implementation
    bool night_mode = false; // Read from appropriate GPIO

    ISREvent event(ISREventType::THEME_SWITCH, night_mode);
    xQueueSendFromISR(TriggerManager::get_instance().isrEventQueue, &event, &x_higher_priority_task_woken);

    portYIELD_FROM_ISR(x_higher_priority_task_woken);
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
    const char* current_panel = PanelManager::get_instance().current_panel;
    
    if (state)
    {
        // Trigger is active - load panel if not already showing
        if (strcmp(current_panel, panel_name) != 0)
        {
            set_trigger_state(trigger_id, ACTION_LOAD_PANEL, panel_name, priority);
        }
    }
    else
    {
        // Trigger is inactive - only act if we're currently showing this panel
        if (strcmp(current_panel, panel_name) == 0)
        {
            // We're showing this panel - restore previous panel
            set_trigger_state(trigger_id, ACTION_RESTORE_PREVIOUS_PANEL, "", TriggerPriority::NORMAL);
        }
        // Don't clear pending triggers when state goes inactive - let them execute
    }
}

bool TriggerManager::should_update_highest_priority(const TriggerState& trigger, TriggerState* currentHighest, TriggerPriority currentPriority, uint64_t currentTimestamp)
{
    if (currentHighest == nullptr)
    {
        return true;
    }
    
    if (trigger.priority > currentPriority)
    {
        return true;
    }
    
    if (trigger.priority == currentPriority && trigger.timestamp < currentTimestamp)
    {
        return true;
    }
    
    return false;
}

