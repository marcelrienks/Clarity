#include "managers/trigger_manager.h"

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
        TriggerManager::TriggerMonitoringTask,              // Task function
        TRIGGER_MONITOR_TASK, // Task name
        4096,                                               // Stack size
        nullptr,                                            // Parameters
        configMAX_PRIORITIES - 1,                           // Priority
        &triggerTaskHandle_,                                // Task handle
        1                                                   // Core 1 (PRO_CPU)
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
            switch (event.eventType)
            {
            case ISREventType::KEY_PRESENT:
                manager.HandleKeyPresentInterrupt(event.pinState);
                break;

            case ISREventType::KEY_NOT_PRESENT:
                manager.HandleKeyNotPresentInterrupt(event.pinState);
                break;

            case ISREventType::LOCK_STATE_CHANGE:
                manager.HandleLockStateInterrupt(event.pinState);
                break;

            case ISREventType::THEME_SWITCH:
                manager.HandleThemeSwitchInterrupt(event.pinState);
                break;
            }
        }
    }
}

// Private Methods

void TriggerManager::SetTriggerState(const char *triggerId, const char *action, const char *target, TriggerPriority priority)
{
    if (xSemaphoreTake(triggerMutex_, pdMS_TO_TICKS(100)) != pdTRUE)
    {
        return;
    }

    log_d("...");

    activeTriggers_[triggerId] = TriggerState(action, target, priority, esp_timer_get_time());
    xSemaphoreGive(triggerMutex_);
}

void TriggerManager::ClearTriggerState(const char *triggerId)
{
    if (xSemaphoreTake(triggerMutex_, pdMS_TO_TICKS(100)) != pdTRUE)
    {
        return;
    }

    log_d("Clearing trigger state for: %s", triggerId);

    auto it = activeTriggers_.find(triggerId);
    if (it != activeTriggers_.end())
    {
        log_d("Trigger %s cleared successfully", triggerId);
        activeTriggers_.erase(it);
    }
    else
    {
        log_w("Trigger %s not found for clearing", triggerId);
    }
    xSemaphoreGive(triggerMutex_);
}


std::pair<const char*, TriggerState*> TriggerManager::GetHighestPriorityTrigger()
{
    if (xSemaphoreTake(triggerMutex_, pdMS_TO_TICKS(100)) != pdTRUE)
    {
        return std::make_pair(nullptr, nullptr);
    }

    log_d("...");

    TriggerState *highest = nullptr;
    const char* highestId = nullptr;
    TriggerPriority highestPriority = TriggerPriority::NORMAL;
    uint64_t oldestTimestamp = UINT64_MAX;
    uint64_t currentTime = esp_timer_get_time();

    for (auto &pair : activeTriggers_)
    {
        const char* triggerId = pair.first;
        TriggerState &trigger = pair.second;
        
        // Skip inactive, processing, or recently processed triggers
        if (!trigger.active || trigger.processing)
            continue;
            
        // Skip if trigger was processed too recently (debouncing)
        if (trigger.lastProcessed > 0 && 
            (currentTime - trigger.lastProcessed) < TRIGGER_DEBOUNCE_TIME_US)
            continue;

        if (ShouldUpdateHighestPriority(trigger, highest, highestPriority, oldestTimestamp))
        {
            highest = &trigger;
            highestId = triggerId;
            highestPriority = trigger.priority;
            oldestTimestamp = trigger.timestamp;
        }
    }

    // Mark the selected trigger as processing to prevent reprocessing
    if (highest != nullptr)
    {
        highest->processing = true;
        highest->lastProcessed = currentTime;
    }

    xSemaphoreGive(triggerMutex_);
    return std::make_pair(highestId, highest);
}

void TriggerManager::setup_gpio_interrupts()
{
    log_d("...");

    // Configure GPIO pins
    pinMode(gpio_pins::KEY_PRESENT, INPUT_PULLDOWN);
    pinMode(gpio_pins::KEY_NOT_PRESENT, INPUT_PULLDOWN);
    pinMode(gpio_pins::LOCK, INPUT_PULLDOWN);
    pinMode(gpio_pins::LIGHTS, INPUT_PULLDOWN);

    // Setup interrupts for key present/not present (both edges)
    attachInterruptArg(gpio_pins::KEY_PRESENT, keyPresentIsrHandler, (void *)true, CHANGE);
    attachInterruptArg(gpio_pins::KEY_NOT_PRESENT, keyNotPresentIsrHandler, (void *)false, CHANGE);
    attachInterruptArg(gpio_pins::LOCK, lockStateIsrHandler, nullptr, CHANGE);
    attachInterruptArg(gpio_pins::LIGHTS, themeSwitchIsrHandler, nullptr, CHANGE);
}

// Static interrupt handlers

void IRAM_ATTR TriggerManager::keyPresentIsrHandler(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    bool pinState = digitalRead(gpio_pins::KEY_PRESENT);

    // Pin HIGH means key present bulb is ON
    ISREvent event(ISREventType::KEY_PRESENT, pinState);
    xQueueSendFromISR(TriggerManager::GetInstance().isrEventQueue, &event, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void IRAM_ATTR TriggerManager::keyNotPresentIsrHandler(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    bool pinState = digitalRead(gpio_pins::KEY_NOT_PRESENT);

    // Pin HIGH means key not present bulb is ON
    ISREvent event(ISREventType::KEY_NOT_PRESENT, pinState);
    xQueueSendFromISR(TriggerManager::GetInstance().isrEventQueue, &event, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void IRAM_ATTR TriggerManager::lockStateIsrHandler(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    bool pinState = digitalRead(gpio_pins::LOCK);

    // Pin HIGH means lock is engaged
    ISREvent event(ISREventType::LOCK_STATE_CHANGE, pinState);
    xQueueSendFromISR(TriggerManager::GetInstance().isrEventQueue, &event, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void IRAM_ATTR TriggerManager::themeSwitchIsrHandler(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    bool pinState = digitalRead(gpio_pins::LIGHTS);

    // Pin HIGH means lights are on (night mode)
    ISREvent event(ISREventType::THEME_SWITCH, pinState);
    xQueueSendFromISR(TriggerManager::GetInstance().isrEventQueue, &event, &xHigherPriorityTaskWoken); // TODO: why is this posting to a queue, when we moved away from queues to shared state

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// C-style ISR wrappers (required for ESP32)
extern "C"
{
    void IRAM_ATTR gpio_key_present_isr(void *arg)
    {
        TriggerManager::keyPresentIsrHandler(arg);
    }

    void IRAM_ATTR gpio_key_not_present_isr(void *arg)
    {
        TriggerManager::keyNotPresentIsrHandler(arg);
    }

    void IRAM_ATTR gpio_lock_state_isr(void *arg)
    {
        TriggerManager::lockStateIsrHandler(arg);
    }

    void IRAM_ATTR gpio_theme_switch_isr(void *arg)
    {
        TriggerManager::themeSwitchIsrHandler(arg);
    }
}

void TriggerManager::HandlePanelStateChange(bool state, const char *panelName, const char *triggerId, TriggerPriority priority)
{
    const char *currentPanel = PanelManager::GetInstance().currentPanel;

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

bool TriggerManager::ShouldUpdateHighestPriority(const TriggerState &trigger, TriggerState *currentHighest, TriggerPriority currentPriority, uint64_t currentTimestamp)
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
