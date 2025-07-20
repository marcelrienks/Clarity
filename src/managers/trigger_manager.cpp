#include "managers/trigger_manager.h"
#include "utilities/trigger_messages.h"
#include <esp32-hal-gpio.h>
#include <esp32-hal-log.h>
#include <string.h>

// Static Methods

TriggerManager &TriggerManager::get_instance()
{
    static TriggerManager instance;
    return instance;
}

// Core Functionality Methods

void TriggerManager::init_dual_core_system()
{
    log_d("Initializing dual-core trigger system...");

    // Create priority queues
    _high_priority_queue = xQueueCreate(HIGH_PRIORITY_QUEUE_SIZE, sizeof(TriggerMessage));
    _medium_priority_queue = xQueueCreate(MEDIUM_PRIORITY_QUEUE_SIZE, sizeof(TriggerMessage));
    _low_priority_queue = xQueueCreate(LOW_PRIORITY_QUEUE_SIZE, sizeof(TriggerMessage));

    if (!_high_priority_queue || !_medium_priority_queue || !_low_priority_queue) {
        log_e("Failed to create priority queues");
        return;
    }

    // Create state mutex
    _state_mutex = xSemaphoreCreateMutex();
    if (!_state_mutex) {
        log_e("Failed to create state mutex");
        return;
    }

    // Initialize application state
    strcpy(_current_panel, PANEL_OIL); // Default panel
    strcpy(_current_theme, THEME_DAY); // Default theme

    // Setup GPIO interrupts
    setup_gpio_interrupts();

    // Create Core 1 monitoring task
    xTaskCreatePinnedToCore(
        TriggerManager::trigger_monitoring_task,    // Task function
        "TriggerMonitorTask",       // Task name
        4096,                       // Stack size
        nullptr,                    // Parameters
        configMAX_PRIORITIES - 1,   // Priority
        &_trigger_task_handle,      // Task handle
        1                           // Core 1 (PRO_CPU)
    );

    log_d("Dual-core trigger system initialized");
}

void TriggerManager::handle_key_present_interrupt(bool key_present)
{
    log_d("Key present interrupt: %s", key_present ? "TRUE" : "FALSE");

    // Thread-safe state access
    if (xSemaphoreTake(_state_mutex, pdMS_TO_TICKS(PANEL_STATE_MUTEX_TIMEOUT)) == pdTRUE) {
        
        if (key_present == true) {
            // Key inserted - check if KeyPanel is already showing
            if (strcmp(_current_panel, PANEL_KEY) != 0) {
                // Need to show KeyPanel - post message
                post_message(ACTION_LOAD_PANEL, PANEL_KEY, TRIGGER_KEY_PRESENT, TriggerPriority::CRITICAL);
                _pending_messages[TRIGGER_KEY_PRESENT] = true;
                log_d("Posted LoadPanel message for KeyPanel");
            } else {
                log_d("KeyPanel already showing, no action needed");
            }
            
        } else {
            // Key removed - check current state
            if (strcmp(_current_panel, PANEL_KEY) == 0) {
                // KeyPanel is showing - post restore message
                post_message(ACTION_RESTORE_PREVIOUS_PANEL, "", TRIGGER_KEY_PRESENT, TriggerPriority::CRITICAL);
                _pending_messages[TRIGGER_KEY_PRESENT] = false;
                log_d("Posted RestorePreviousPanel message");
            } else if (_pending_messages[TRIGGER_KEY_PRESENT]) {
                // KeyPanel not showing but message is pending - remove message
                remove_message_from_queue(TRIGGER_KEY_PRESENT);
                _pending_messages[TRIGGER_KEY_PRESENT] = false;
                log_d("Removed pending KeyPanel message");
            } else {
                log_d("KeyPanel not showing and no pending message, no action needed");
            }
        }

        // Update hardware state
        _key_present_state = key_present;
        
        xSemaphoreGive(_state_mutex);
    } else {
        log_w("Failed to acquire state mutex for key present interrupt");
    }
}

void TriggerManager::handle_lock_state_interrupt(bool lock_engaged)
{
    log_d("Lock state interrupt: %s", lock_engaged ? "ENGAGED" : "DISENGAGED");

    // Thread-safe state access
    if (xSemaphoreTake(_state_mutex, pdMS_TO_TICKS(PANEL_STATE_MUTEX_TIMEOUT)) == pdTRUE) {
        
        if (lock_engaged == true) {
            // Lock engaged - check if LockPanel is already showing
            if (strcmp(_current_panel, PANEL_LOCK) != 0) {
                // Need to show LockPanel - post message
                post_message(ACTION_LOAD_PANEL, PANEL_LOCK, TRIGGER_LOCK_STATE, TriggerPriority::IMPORTANT);
                _pending_messages[TRIGGER_LOCK_STATE] = true;
                log_d("Posted LoadPanel message for LockPanel");
            } else {
                log_d("LockPanel already showing, no action needed");
            }
            
        } else {
            // Lock disengaged - check current state
            if (strcmp(_current_panel, PANEL_LOCK) == 0) {
                // LockPanel is showing - post restore message
                post_message(ACTION_RESTORE_PREVIOUS_PANEL, "", TRIGGER_LOCK_STATE, TriggerPriority::IMPORTANT);
                _pending_messages[TRIGGER_LOCK_STATE] = false;
                log_d("Posted RestorePreviousPanel message");
            } else if (_pending_messages[TRIGGER_LOCK_STATE]) {
                // LockPanel not showing but message is pending - remove message
                remove_message_from_queue(TRIGGER_LOCK_STATE);
                _pending_messages[TRIGGER_LOCK_STATE] = false;
                log_d("Removed pending LockPanel message");
            } else {
                log_d("LockPanel not showing and no pending message, no action needed");
            }
        }

        // Update hardware state
        _lock_engaged_state = lock_engaged;
        
        xSemaphoreGive(_state_mutex);
    } else {
        log_w("Failed to acquire state mutex for lock state interrupt");
    }
}

void TriggerManager::handle_theme_switch_interrupt(bool night_mode)
{
    log_d("Theme switch interrupt: %s", night_mode ? "NIGHT" : "DAY");

    // Thread-safe state access
    if (xSemaphoreTake(_state_mutex, pdMS_TO_TICKS(THEME_STATE_MUTEX_TIMEOUT)) == pdTRUE) {
        
        const char* target_theme = night_mode ? THEME_NIGHT : THEME_DAY;
        
        if (strcmp(_current_theme, target_theme) != 0) {
            if (_pending_messages[TRIGGER_THEME_SWITCH]) {
                // Update existing message
                update_message_in_queue(TRIGGER_THEME_SWITCH, ACTION_CHANGE_THEME, target_theme);
                log_d("Updated pending theme message to %s", target_theme);
            } else {
                // Post new message
                post_message(ACTION_CHANGE_THEME, target_theme, TRIGGER_THEME_SWITCH, TriggerPriority::NORMAL);
                _pending_messages[TRIGGER_THEME_SWITCH] = true;
                log_d("Posted ChangeTheme message to %s", target_theme);
            }
        } else {
            // Target theme already active
            if (_pending_messages[TRIGGER_THEME_SWITCH]) {
                // Remove pending message - no change needed
                remove_message_from_queue(TRIGGER_THEME_SWITCH);
                _pending_messages[TRIGGER_THEME_SWITCH] = false;
                log_d("Removed pending theme message - already %s", target_theme);
            } else {
                log_d("Theme already %s, no action needed", target_theme);
            }
        }

        // Update hardware state
        _night_mode_state = night_mode;
        
        xSemaphoreGive(_state_mutex);
    } else {
        log_w("Failed to acquire state mutex for theme switch interrupt");
    }
}

void TriggerManager::update_application_state(const char* panel_name, const char* theme_name)
{
    if (xSemaphoreTake(_state_mutex, pdMS_TO_TICKS(PANEL_STATE_MUTEX_TIMEOUT)) == pdTRUE) {
        if (panel_name && strlen(panel_name) < sizeof(_current_panel)) {
            strcpy(_current_panel, panel_name);
            log_d("Updated current panel to: %s", panel_name);
        }
        
        if (theme_name && strlen(theme_name) < sizeof(_current_theme)) {
            strcpy(_current_theme, theme_name);
            log_d("Updated current theme to: %s", theme_name);
        }
        
        xSemaphoreGive(_state_mutex);
    } else {
        log_w("Failed to acquire state mutex for application state update");
    }
}

void TriggerManager::get_application_state(char* panel_name, char* theme_name)
{
    if (xSemaphoreTake(_state_mutex, pdMS_TO_TICKS(PANEL_STATE_MUTEX_TIMEOUT)) == pdTRUE) {
        if (panel_name) {
            strcpy(panel_name, _current_panel);
        }
        
        if (theme_name) {
            strcpy(theme_name, _current_theme);
        }
        
        xSemaphoreGive(_state_mutex);
    } else {
        log_w("Failed to acquire state mutex for application state get");
    }
}

// Core 1 Task Methods

void TriggerManager::trigger_monitoring_task(void* pvParameters)
{
    log_d("Core 1 trigger monitoring task started");
    
    // Task loop - mostly handled by interrupts
    while (1) {
        // Light sleep - most work done in interrupt handlers
        vTaskDelay(pdMS_TO_TICKS(100));
        
        // Optional: Periodic state validation or cleanup
        // This could include checking for stale messages or state synchronization
    }
}

// Private Methods

void TriggerManager::post_message(const char* action, const char* target, const char* trigger_id, TriggerPriority priority)
{
    TriggerMessage msg;
    strcpy(msg.trigger_id, trigger_id);
    strcpy(msg.action, action);
    strcpy(msg.target, target);
    msg.priority = priority;
    msg.timestamp = esp_timer_get_time();

    QueueHandle_t target_queue = get_target_queue(priority);
    
    if (xQueueSend(target_queue, &msg, 0) == pdTRUE) {
        log_d("Posted message - Action: %s, Target: %s, Priority: %d", action, target, (int)priority);
    } else {
        log_w("Failed to post message - queue full. Action: %s, Target: %s", action, target);
    }
}

void TriggerManager::remove_message_from_queue(const char* trigger_id)
{
    // Note: FreeRTOS doesn't have built-in message removal by ID
    // This is a simplified implementation - a more sophisticated approach
    // would involve draining the queue, filtering, and re-posting
    log_d("Marked trigger %s for message removal", trigger_id);
    
    // For now, we rely on the pending_messages tracking to prevent
    // processing of messages that should be removed
    // A full implementation would drain and filter the queues
}

void TriggerManager::update_message_in_queue(const char* trigger_id, const char* action, const char* target)
{
    // Note: Similar to remove_message_from_queue, this is simplified
    // A full implementation would locate and update the specific message
    log_d("Marked trigger %s for message update - Action: %s, Target: %s", trigger_id, action, target);
    
    // For now, post a new message (which will be the latest)
    // The pending_messages tracking helps prevent duplicate processing
    if (strcmp(action, ACTION_CHANGE_THEME) == 0) {
        post_message(action, target, trigger_id, TriggerPriority::NORMAL);
    }
}

QueueHandle_t TriggerManager::get_target_queue(TriggerPriority priority)
{
    switch (priority) {
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
    log_d("Setting up GPIO interrupts...");

    // Configure GPIO pins
    pinMode(GpioPins::KEY_PRESENT, INPUT_PULLDOWN);
    pinMode(GpioPins::KEY_NOT_PRESENT, INPUT_PULLDOWN);
    pinMode(GpioPins::LOCK, INPUT_PULLDOWN);
    
    // Setup interrupts for key present/not present (both edges)
    attachInterruptArg(GpioPins::KEY_PRESENT, key_present_isr_handler, (void*)true, CHANGE);
    attachInterruptArg(GpioPins::KEY_NOT_PRESENT, key_not_present_isr_handler, (void*)false, CHANGE);
    attachInterruptArg(GpioPins::LOCK, lock_state_isr_handler, nullptr, CHANGE);
    
    log_d("GPIO interrupts configured");
}

// Static interrupt handlers

void IRAM_ATTR TriggerManager::key_present_isr_handler(void* arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    bool pin_state = digitalRead(GpioPins::KEY_PRESENT);
    
    // In real interrupt context, we would defer to a task
    // For now, call the handler directly
    TriggerManager::get_instance().handle_key_present_interrupt(pin_state);
    
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void IRAM_ATTR TriggerManager::key_not_present_isr_handler(void* arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    bool pin_state = digitalRead(GpioPins::KEY_NOT_PRESENT);
    
    // Key not present pin active means key is NOT present
    TriggerManager::get_instance().handle_key_present_interrupt(!pin_state);
    
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void IRAM_ATTR TriggerManager::lock_state_isr_handler(void* arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    bool pin_state = digitalRead(GpioPins::LOCK);
    
    TriggerManager::get_instance().handle_lock_state_interrupt(pin_state);
    
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void IRAM_ATTR TriggerManager::theme_switch_isr_handler(void* arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    // For theme switch, we'll assume a different GPIO pin or logic
    // This is a placeholder for the actual implementation
    bool night_mode = false; // Read from appropriate GPIO
    
    TriggerManager::get_instance().handle_theme_switch_interrupt(night_mode);
    
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// C-style ISR wrappers (required for ESP32)
extern "C" {
    void IRAM_ATTR gpio_key_present_isr(void* arg) {
        TriggerManager::key_present_isr_handler(arg);
    }
    
    void IRAM_ATTR gpio_key_not_present_isr(void* arg) {
        TriggerManager::key_not_present_isr_handler(arg);
    }
    
    void IRAM_ATTR gpio_lock_state_isr(void* arg) {
        TriggerManager::lock_state_isr_handler(arg);
    }
    
    void IRAM_ATTR gpio_theme_switch_isr(void* arg) {
        TriggerManager::theme_switch_isr_handler(arg);
    }
}

void TriggerManager::get_queue_handles(QueueHandle_t* high_queue, QueueHandle_t* medium_queue, QueueHandle_t* low_queue)
{
    if (high_queue) {
        *high_queue = _high_priority_queue;
    }
    if (medium_queue) {
        *medium_queue = _medium_priority_queue;
    }
    if (low_queue) {
        *low_queue = _low_priority_queue;
    }
    log_d("Provided queue handles to Core 0");
}
