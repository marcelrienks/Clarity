#include "managers/trigger_manager.h"
#include "managers/panel_manager.h"
#include "managers/style_manager.h"
#include "utilities/trigger_messages.h"
#include <esp32-hal-gpio.h>
#include <esp32-hal-log.h>
#include <string.h>
#include <utilities/types.h>

// Static Methods

TriggerManager &TriggerManager::GetInstance()
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
    triggerMutex_ = xSemaphoreCreateMutex();

    setup_gpio_interrupts();

    xTaskCreatePinnedToCore(
        TriggerManager::TriggerMonitoringTask, // Task function
        TriggerManager::GetInstance().TRIGGER_MONITOR_TASK,      // Task name
        4096,                                    // Stack size
        nullptr,                                 // Parameters
        configMAX_PRIORITIES - 1,                // Priority
        &triggerTaskHandle_,                   // Task handle
        1                                        // Core 1 (PRO_CPU)
    );
}

void TriggerManager::HandleKeyPresentInterrupt(bool keyPresentState)
{
    log_d("...");

    if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(PANEL_STATE_MUTEX_TIMEOUT)) != pdTRUE)
    {
        return;
    }

    HandlePanelStateChange(keyPresentState, PanelNames::KEY, TRIGGER_KEY_PRESENT, TriggerPriority::CRITICAL);
    keyPresentState_ = keyPresentState;
    xSemaphoreGive(stateMutex);
}

void TriggerManager::HandleKeyNotPresentInterrupt(bool keyNotPresentState)
{
    log_d("...");

    if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(PANEL_STATE_MUTEX_TIMEOUT)) != pdTRUE)
    {
        return;
    }

    HandlePanelStateChange(keyNotPresentState, PanelNames::KEY, TRIGGER_KEY_NOT_PRESENT, TriggerPriority::CRITICAL);
    keyNotPresentState_ = keyNotPresentState;
    xSemaphoreGive(stateMutex);
}

void TriggerManager::HandleLockStateInterrupt(bool lockEngaged)
{
    log_d("...");

    if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(PANEL_STATE_MUTEX_TIMEOUT)) != pdTRUE)
    {
        return;
    }

    HandlePanelStateChange(lockEngaged, PanelNames::LOCK, TRIGGER_LOCK_STATE, TriggerPriority::IMPORTANT);
    lockEngagedState_ = lockEngaged;
    xSemaphoreGive(stateMutex);
}

void TriggerManager::HandleThemeSwitchInterrupt(bool nightMode)
{
    log_d("...");

    if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(THEME_STATE_MUTEX_TIMEOUT)) != pdTRUE)
    {
        return;
    }

    const char *targetTheme = nightMode ? THEME_NIGHT : THEME_DAY;
    const char *currentTheme = StyleManager::GetInstance().THEME;
    
    if (strcmp(currentTheme, targetTheme) != 0)
    {
        SetTriggerState(TRIGGER_THEME_SWITCH, ACTION_CHANGE_THEME, targetTheme, TriggerPriority::NORMAL);
    }
    else
    {
        ClearTriggerState(TRIGGER_THEME_SWITCH);
    }

    nightModeState_ = nightMode;
    xSemaphoreGive(stateMutex);
}

void TriggerManager::NotifyApplicationStateUpdated()
{
    log_d("Application state updated - TriggerManager notified");
    // This method serves as a notification point for future functionality
    // Currently no action needed as PanelManager is the source of truth
}

// Task Methods

void TriggerManager::TriggerMonitoringTask(void *pvParameters)
{
    log_d("...");

    TriggerManager &manager = TriggerManager::GetInstance();
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
                manager.HandleKeyPresentInterrupt(event.pin_state);
                break;

            case ISREventType::KEY_NOT_PRESENT:
                manager.HandleKeyNotPresentInterrupt(event.pin_state);
                break;

            case ISREventType::LOCK_STATE_CHANGE:
                manager.HandleLockStateInterrupt(event.pin_state);
                break;

            case ISREventType::THEME_SWITCH:
                manager.HandleThemeSwitchInterrupt(event.pin_state);
                break;
            }
        }
    }
}

// Private Methods

void TriggerManager::SetTriggerState(const char* triggerId, const char *action, const char *target, TriggerPriority priority)
{
    if (xSemaphoreTake(triggerMutex_, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        activeTriggers_[triggerId] = TriggerState(action, target, priority, esp_timer_get_time());
        log_d("Set trigger %s: %s -> %s (priority %d)", triggerId, action, target, (int)priority);
        xSemaphoreGive(triggerMutex_);
    }
}

void TriggerManager::ClearTriggerState(const char* triggerId)
{
    if (xSemaphoreTake(triggerMutex_, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        auto it = activeTriggers_.find(triggerId);
        if (it != activeTriggers_.end())
        {
            activeTriggers_.erase(it);
            log_d("Cleared trigger %s", triggerId);
        }
        xSemaphoreGive(triggerMutex_);
    }
}

void TriggerManager::UpdateTriggerState(const char* triggerId, const char *action, const char *target)
{
    if (xSemaphoreTake(triggerMutex_, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        auto it = activeTriggers_.find(triggerId);
        if (it != activeTriggers_.end())
        {
            it->second.action = action;
            it->second.target = target;
            it->second.timestamp = esp_timer_get_time();
            log_d("Updated trigger %s: %s -> %s", triggerId, action, target);
        }
        xSemaphoreGive(triggerMutex_);
    }
}

TriggerState* TriggerManager::GetHighestPriorityTrigger()
{
    if (xSemaphoreTake(triggerMutex_, pdMS_TO_TICKS(100)) != pdTRUE)
    {
        return nullptr;
    }

    TriggerState* highest = nullptr;
    TriggerPriority highestPriority = TriggerPriority::NORMAL;
    uint64_t oldestTimestamp = UINT64_MAX;
    
    for (auto& pair : activeTriggers_)
    {
        TriggerState& trigger = pair.second;
        if (!trigger.active) continue;
        
        if (ShouldUpdateHighestPriority(trigger, highest, highestPriority, oldestTimestamp))
        {
            highest = &trigger;
            highestPriority = trigger.priority;
            oldestTimestamp = trigger.timestamp;
        }
    }
    
    xSemaphoreGive(triggerMutex_);
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
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    bool pinState = digitalRead(gpio_pins::KEY_PRESENT);

    // Post event to task queue for safe processing
    ISREvent event(ISREventType::KEY_PRESENT, pinState);
    xQueueSendFromISR(TriggerManager::GetInstance().isrEventQueue, &event, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void IRAM_ATTR TriggerManager::key_not_present_isr_handler(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    bool pinState = digitalRead(gpio_pins::KEY_NOT_PRESENT);

    // Pin HIGH means key not present bulb is ON
    ISREvent event(ISREventType::KEY_NOT_PRESENT, pinState);
    xQueueSendFromISR(TriggerManager::GetInstance().isrEventQueue, &event, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void IRAM_ATTR TriggerManager::lock_state_isr_handler(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    bool pinState = digitalRead(gpio_pins::LOCK);

    ISREvent event(ISREventType::LOCK_STATE_CHANGE, pinState);
    xQueueSendFromISR(TriggerManager::GetInstance().isrEventQueue, &event, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void IRAM_ATTR TriggerManager::theme_switch_isr_handler(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // For theme switch, we'll assume a different GPIO pin or logic
    // This is a placeholder for the actual implementation
    bool nightMode = false; // Read from appropriate GPIO

    ISREvent event(ISREventType::THEME_SWITCH, nightMode);
    xQueueSendFromISR(TriggerManager::GetInstance().isrEventQueue, &event, &xHigherPriorityTaskWoken);

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

void TriggerManager::HandlePanelStateChange(bool state, const char* panelName, const char* triggerId, TriggerPriority priority)
{
    const char* currentPanel = PanelManager::GetInstance().currentPanel;
    
    if (state)
    {
        // Trigger is active - load panel if not already showing
        if (strcmp(currentPanel, panelName) != 0)
        {
            SetTriggerState(triggerId, ACTION_LOAD_PANEL, panelName, priority);
        }
    }
    else
    {
        // Trigger is inactive - only act if we're currently showing this panel
        if (strcmp(currentPanel, panelName) == 0)
        {
            // We're showing this panel - restore previous panel
            SetTriggerState(triggerId, ACTION_RESTORE_PREVIOUS_PANEL, "", TriggerPriority::NORMAL);
        }
        // Don't clear pending triggers when state goes inactive - let them execute
    }
}

bool TriggerManager::ShouldUpdateHighestPriority(const TriggerState& trigger, TriggerState* currentHighest, TriggerPriority currentPriority, uint64_t currentTimestamp)
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

