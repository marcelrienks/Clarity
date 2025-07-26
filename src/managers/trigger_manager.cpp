#include "managers/trigger_manager.h"
#include "triggers/key_trigger.h"
#include "triggers/lock_trigger.h"
#include "triggers/lights_trigger.h"
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

    // Create ISR event queue (no mutex needed for Core 0 exclusive ownership)
    isrEventQueue = xQueueCreate(10, sizeof(ISREvent));

    // Configure GPIO pins and attach interrupt handlers
    setup_gpio_interrupts();

    // No Core 1 task needed - GPIO ISRs handle all Core 1 work
    // ISRs queue events directly, Core 0 processes them
    
    log_d("Simplified trigger system initialized");
}

void TriggerManager::RegisterAllTriggers()
{
    log_d("Registering all triggers...");
    
    // Register concrete trigger implementations
    auto keyTrigger = std::make_unique<KeyTrigger>();
    keyTrigger->init();
    RegisterTrigger(std::move(keyTrigger));
    
    auto lockTrigger = std::make_unique<LockTrigger>();
    lockTrigger->init();
    RegisterTrigger(std::move(lockTrigger));
    
    auto lightsTrigger = std::make_unique<LightsTrigger>();
    lightsTrigger->init();
    RegisterTrigger(std::move(lightsTrigger));
    
    log_d("All triggers registered successfully");
}

void TriggerManager::RegisterTrigger(std::unique_ptr<AlertTrigger> trigger)
{
    // No mutex needed - only called during setup on Core 0
    log_d("Registering trigger: %s", trigger->GetId());
    triggers_.push_back(std::move(trigger));
}

void TriggerManager::ProcessPendingTriggerEvents()
{
    ISREvent event;
    UBaseType_t queueCount = uxQueueMessagesWaiting(isrEventQueue);
    
    if (queueCount > 0) {
        log_i("Core 0: Processing %d pending trigger events", queueCount);
    }
    
    // Process all queued trigger events (no mutex needed - Core 0 only)
    while (xQueueReceive(isrEventQueue, &event, 0) == pdTRUE)
    {
        log_i("Core 0: Processing trigger event: type=%d, state=%s", (int)event.eventType, event.pinState ? "HIGH" : "LOW");
        
        // Map trigger events to trigger IDs
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
            AlertTrigger* trigger = FindTriggerById(triggerId);
            if (trigger)
            {
                TriggerExecutionState oldState = trigger->GetState();
                TriggerExecutionState newState = event.pinState ? TriggerExecutionState::ACTIVE : TriggerExecutionState::INACTIVE;
                
                log_i("Trigger %s: %s -> %s (pin: %s)", 
                      triggerId, 
                      oldState == TriggerExecutionState::INIT ? "INIT" : (oldState == TriggerExecutionState::ACTIVE ? "ACTIVE" : "INACTIVE"),
                      newState == TriggerExecutionState::ACTIVE ? "ACTIVE" : "INACTIVE",
                      event.pinState ? "HIGH" : "LOW");
                
                trigger->SetState(newState);
            }
        }
    }
}

std::vector<TriggerActionRequest> TriggerManager::EvaluateAndGetTriggerRequests()
{
    // No mutex needed - Core 0 exclusive ownership of trigger state
    std::vector<TriggerActionRequest> requests;
    log_d("=== Evaluating all triggers ===");
        
        // Log current state of all triggers first
        for (auto& trigger : triggers_)
        {
            TriggerExecutionState state = trigger->GetState();
            const char* stateStr = (state == TriggerExecutionState::INIT) ? "INIT" : 
                                  (state == TriggerExecutionState::ACTIVE) ? "ACTIVE" : "INACTIVE";
            log_i("Trigger %s: %s (priority %d)", trigger->GetId(), stateStr, (int)trigger->GetPriority());
        }
        
        // Collect action requests from lowest to highest priority
        // This ensures highest priority action is the last one in the list (and wins)
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
        
        log_d("Collecting trigger requests in priority order (lowest to highest)...");
        
        // Collect requests from lowest to highest priority based on state
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
                    log_i("*** COLLECTING ACTION REQUEST for trigger: %s (priority %d) ***", trigger->GetId(), (int)trigger->GetPriority());
                    requests.push_back(trigger->GetActionRequest());
                    break;
                    
                case TriggerExecutionState::INACTIVE:
                    log_i("*** COLLECTING RESTORE REQUEST for trigger: %s (priority %d) ***", trigger->GetId(), (int)trigger->GetPriority());
                    requests.push_back(trigger->GetRestoreRequest());
                    break;
            }
        }
        
        log_d("=== Trigger evaluation complete, %d requests collected ===", requests.size());
    
    return requests;
}

void TriggerManager::InitializeTriggersFromGpio()
{
    // No mutex needed - only called during setup on Core 0
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
    
    log_d("All triggers initialized from GPIO states");
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
// No Core 1 task needed - removed entirely
// GPIO ISRs on Core 1 queue events directly
// Core 0 main loop processes them via ProcessPendingTriggerEvents()

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
    
    // ISR-safe logging (minimal)
    static bool lastPinState = false;
    static bool firstCall = true;
    
    if (firstCall || pinState != lastPinState)
    {
        ISREvent event(ISREventType::LOCK_STATE_CHANGE, pinState);
        xQueueSendFromISR(TriggerManager::GetInstance().isrEventQueue, &event, &xHigherPriorityTaskWoken);
        lastPinState = pinState;
        firstCall = false;
    }
    
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

