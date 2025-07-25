#include "managers/trigger_manager.h"
#include "managers/panel_manager.h"
#include "managers/style_manager.h"
#include <algorithm>

TriggerManager &TriggerManager::GetInstance()
{
    static TriggerManager instance;
    return instance;
}

void TriggerManager::init()
{
    log_d("Initializing simplified trigger system...");

    // Create ISR event queue and mutex
    isrEventQueue = xQueueCreate(10, sizeof(ISREvent));
    triggerMutex_ = xSemaphoreCreateMutex();

    // Configure GPIO pins and attach interrupt handlers
    setup_gpio_interrupts();

    // Launch Core 1 task to process ISR events
    xTaskCreatePinnedToCore(
        TriggerManager::TriggerMonitoringTask,
        TRIGGER_MONITOR_TASK,
        4096,
        nullptr,
        configMAX_PRIORITIES - 1,
        &triggerTaskHandle_,
        1  // Core 1
    );
    
    log_d("Simplified trigger system initialized");
}

void TriggerManager::RegisterTrigger(std::unique_ptr<AlertTrigger> trigger)
{
    if (xSemaphoreTake(triggerMutex_, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        log_d("Registering trigger: %s", trigger->GetId());
        triggers_.push_back(std::move(trigger));
        xSemaphoreGive(triggerMutex_);
    }
}

void TriggerManager::HandleGpioStateChange(const char* triggerId, bool pinState)
{
    if (xSemaphoreTake(triggerMutex_, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        AlertTrigger* trigger = FindTriggerById(triggerId);
        if (trigger)
        {
            // Only update trigger state on Core 1 - do NOT execute actions here
            // Actions will be executed by Core 0 in EvaluateAndExecuteTriggers()
            TriggerExecutionState newState = pinState ? TriggerExecutionState::ACTIVE : TriggerExecutionState::INACTIVE;
            trigger->SetState(newState);
            log_v("Trigger %s state updated to %s on Core 1", triggerId, pinState ? "ACTIVE" : "INACTIVE");
        }
        xSemaphoreGive(triggerMutex_);
    }
}

void TriggerManager::EvaluateAndExecuteTriggers()
{
    if (xSemaphoreTake(triggerMutex_, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        // Execute actions and restores from lowest to highest priority
        // This ensures highest priority action is the last one executed (and wins)
        std::vector<AlertTrigger*> sortedTriggers;
        for (auto& trigger : triggers_)
        {
            sortedTriggers.push_back(trigger.get());
        }
        
        // Sort by priority (CRITICAL=0, IMPORTANT=1, NORMAL=2, so lower number = higher priority)
        std::sort(sortedTriggers.begin(), sortedTriggers.end(), 
                  [](AlertTrigger* a, AlertTrigger* b) {
                      return a->GetPriority() > b->GetPriority(); // Reverse sort: NORMAL -> CRITICAL
                  });
        
        // Execute from lowest to highest priority based on state
        for (auto* trigger : sortedTriggers)
        {
            TriggerExecutionState state = trigger->GetState();
            
            switch (state)
            {
                case TriggerExecutionState::INIT:
                    // No action required during initialization
                    log_v("Trigger %s in INIT state - no action", trigger->GetId());
                    break;
                    
                case TriggerExecutionState::ACTIVE:
                    log_d("Executing action for trigger: %s (priority %d)", trigger->GetId(), (int)trigger->GetPriority());
                    trigger->ExecuteAction();
                    break;
                    
                case TriggerExecutionState::INACTIVE:
                    log_d("Executing restore for trigger: %s (priority %d)", trigger->GetId(), (int)trigger->GetPriority());
                    trigger->ExecuteRestore();
                    break;
            }
        }
        
        xSemaphoreGive(triggerMutex_);
    }
}

void TriggerManager::InitializeTriggersFromGpio()
{
    if (xSemaphoreTake(triggerMutex_, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        log_d("Initializing trigger states from current GPIO pin states...");
        
        // Read current GPIO pin states and initialize triggers accordingly
        for (auto& trigger : triggers_)
        {
            const char* triggerId = trigger->GetId();
            bool currentPinState = false;
            
            // Determine current pin state based on trigger ID
            if (strcmp(triggerId, TRIGGER_KEY_PRESENT) == 0)
            {
                currentPinState = digitalRead(gpio_pins::KEY_PRESENT);
            }
            else if (strcmp(triggerId, TRIGGER_LOCK_STATE) == 0)
            {
                currentPinState = digitalRead(gpio_pins::LOCK);
            }
            else if (strcmp(triggerId, TRIGGER_LIGHTS_STATE) == 0)
            {
                currentPinState = digitalRead(gpio_pins::LIGHTS);
            }
            
            // Set trigger state based on current pin state
            TriggerExecutionState initialState = currentPinState ? TriggerExecutionState::ACTIVE : TriggerExecutionState::INACTIVE;
            trigger->SetState(initialState);
            log_d("Trigger %s initialized to %s based on GPIO state", 
                  triggerId, currentPinState ? "ACTIVE" : "INACTIVE");
        }
        
        xSemaphoreGive(triggerMutex_);
        log_d("All triggers initialized from GPIO states");
    }
}

AlertTrigger* TriggerManager::FindTriggerById(const char* triggerId)
{
    for (auto& trigger : triggers_)
    {
        if (strcmp(trigger->GetId(), triggerId) == 0)
        {
            return trigger.get();
        }
    }
    return nullptr;
}
void TriggerManager::TriggerMonitoringTask(void *pvParameters)
{
    log_d("TriggerMonitoringTask started on Core 1");

    TriggerManager &manager = TriggerManager::GetInstance();
    ISREvent event;

    while (1)
    {
        if (xQueueReceive(manager.isrEventQueue, &event, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            log_v("Processing ISR event type %d with pin state %d", (int)event.eventType, event.pinState);
            
            // Map ISR events to trigger IDs and handle state changes
            const char* triggerId = nullptr;
            switch (event.eventType)
            {
            case ISREventType::KEY_PRESENT:
                triggerId = TRIGGER_KEY_PRESENT;
                break;
            case ISREventType::KEY_NOT_PRESENT:
                triggerId = TRIGGER_KEY_NOT_PRESENT;
                break;
            case ISREventType::LOCK_STATE_CHANGE:
                triggerId = TRIGGER_LOCK_STATE;
                break;
            case ISREventType::THEME_SWITCH:
                triggerId = TRIGGER_LIGHTS_STATE;
                break;
            default:
                log_w("Unknown ISR event type: %d", (int)event.eventType);
                continue;
            }
            
            if (triggerId)
            {
                manager.HandleGpioStateChange(triggerId, event.pinState);
            }
        }
    }
}

void TriggerManager::setup_gpio_interrupts()
{
    log_d("Setting up GPIO interrupts...");

    // Configure GPIO pins as inputs with pull-down resistors
    pinMode(gpio_pins::KEY_PRESENT, INPUT_PULLDOWN);
    pinMode(gpio_pins::KEY_NOT_PRESENT, INPUT_PULLDOWN);
    pinMode(gpio_pins::LOCK, INPUT_PULLDOWN);
    pinMode(gpio_pins::LIGHTS, INPUT_PULLDOWN);

    // Attach interrupt handlers for state change detection
    attachInterruptArg(gpio_pins::KEY_PRESENT, keyPresentIsrHandler, (void *)true, CHANGE);
    attachInterruptArg(gpio_pins::KEY_NOT_PRESENT, keyNotPresentIsrHandler, (void *)false, CHANGE);
    attachInterruptArg(gpio_pins::LOCK, lockStateIsrHandler, nullptr, CHANGE);
    attachInterruptArg(gpio_pins::LIGHTS, themeSwitchIsrHandler, nullptr, CHANGE);
    
    log_d("GPIO interrupts configured successfully");
}
// ISR handlers - queue events for safe task processing
void IRAM_ATTR TriggerManager::keyPresentIsrHandler(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    bool pinState = digitalRead(gpio_pins::KEY_PRESENT);
    ISREvent event(ISREventType::KEY_PRESENT, pinState);
    xQueueSendFromISR(TriggerManager::GetInstance().isrEventQueue, &event, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void IRAM_ATTR TriggerManager::keyNotPresentIsrHandler(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    bool pinState = digitalRead(gpio_pins::KEY_NOT_PRESENT);
    ISREvent event(ISREventType::KEY_NOT_PRESENT, pinState);
    xQueueSendFromISR(TriggerManager::GetInstance().isrEventQueue, &event, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void IRAM_ATTR TriggerManager::lockStateIsrHandler(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    bool pinState = digitalRead(gpio_pins::LOCK);
    ISREvent event(ISREventType::LOCK_STATE_CHANGE, pinState);
    xQueueSendFromISR(TriggerManager::GetInstance().isrEventQueue, &event, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void IRAM_ATTR TriggerManager::themeSwitchIsrHandler(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    bool pinState = digitalRead(gpio_pins::LIGHTS);
    ISREvent event(ISREventType::THEME_SWITCH, pinState);
    xQueueSendFromISR(TriggerManager::GetInstance().isrEventQueue, &event, &xHigherPriorityTaskWoken);
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

