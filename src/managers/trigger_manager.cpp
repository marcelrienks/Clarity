#include "managers/trigger_manager.h"
#include "triggers/key_trigger.h"
#include "triggers/lock_trigger.h"
#include "triggers/lights_trigger.h"
#include "managers/panel_manager.h"
#include "managers/preference_manager.h"
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
    
    auto keyNotPresentTrigger = std::make_unique<KeyNotPresentTrigger>();
    keyNotPresentTrigger->init();
    RegisterTrigger(std::move(keyNotPresentTrigger));
    
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

void TriggerManager::ProcessTriggerEvents()
{
    ISREvent event;
    
    // Process ALL queued trigger events and execute actions immediately  
    while (xQueueReceive(isrEventQueue, &event, 0) == pdTRUE)
    {
        log_i("Processing trigger event: type=%d, state=%s", (int)event.eventType, event.pinState ? "HIGH" : "LOW");
        
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
                
                log_i("Trigger %s: %s -> %s", 
                      triggerId, 
                      oldState == TriggerExecutionState::ACTIVE ? "ACTIVE" : "INACTIVE",
                      newState == TriggerExecutionState::ACTIVE ? "ACTIVE" : "INACTIVE");
                
                trigger->SetState(newState);
                UpdateActiveTriggersList(trigger, newState);
                
                // Execute trigger action immediately
                ExecuteTriggerAction(trigger, newState);
            }
        }
    }
}

void TriggerManager::ExecuteTriggerAction(AlertTrigger* trigger, TriggerExecutionState state)
{
    if (state == TriggerExecutionState::ACTIVE) {
        // Execute trigger action when activated
        TriggerActionRequest request = trigger->GetActionRequest();
        
        if (request.type == TriggerActionType::LoadPanel) {
            log_i("Executing panel action: Load %s", request.panelName);
            PanelManager::GetInstance().CreateAndLoadPanel(request.panelName, []() {
                PanelManager::GetInstance().TriggerPanelSwitchCallback("");
            }, request.isTriggerDriven);
        }
        else if (request.type == TriggerActionType::ToggleTheme) {
            log_i("Executing theme action: %s", request.panelName);
            StyleManager::GetInstance().set_theme(request.panelName);
            // Refresh current panel with new theme
            PanelManager::GetInstance().UpdatePanel();
        }
    } else {
        // Handle trigger deactivation - execute restore request
        TriggerActionRequest restoreRequest = trigger->GetRestoreRequest();
        
        if (restoreRequest.type == TriggerActionType::LoadPanel) {
            // Only restore panel if no other panel-loading triggers are active
            bool hasPanelTriggers = false;
            for (const auto& [priority, activeTrigger] : activeTriggers_) {
                TriggerActionRequest activeRequest = activeTrigger->GetActionRequest();
                if (activeRequest.type == TriggerActionType::LoadPanel) {
                    hasPanelTriggers = true;
                    break;
                }
            }
            
            if (!hasPanelTriggers) {
                log_i("No panel-loading triggers active - restoring panel: %s", restoreRequest.panelName);
                PanelManager::GetInstance().CreateAndLoadPanel(restoreRequest.panelName, []() {
                    PanelManager::GetInstance().TriggerPanelSwitchCallback("");
                }, restoreRequest.isTriggerDriven);
            } else {
                log_i("Other panel-loading triggers still active - skipping panel restoration");
            }
        }
        else if (restoreRequest.type == TriggerActionType::ToggleTheme) {
            log_i("Restoring theme: %s", restoreRequest.panelName);
            StyleManager::GetInstance().set_theme(restoreRequest.panelName);
            // Refresh current panel with restored theme
            PanelManager::GetInstance().UpdatePanel();
        }
    }
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
        
        // Initialize active triggers list
        if (initialState == TriggerExecutionState::ACTIVE) {
            activeTriggers_.push_back({trigger->GetPriority(), trigger.get()});
        }
        
        log_d("Trigger %s initialized to %s based on GPIO state", 
              triggerId, currentPinState ? "ACTIVE" : "INACTIVE");
    }
    
    // Sort initial active triggers list by priority (stable sort for FIFO within same priority)
    std::stable_sort(activeTriggers_.begin(), activeTriggers_.end(),
        [](const auto& a, const auto& b) { return a.first < b.first; });
    
    log_d("All triggers initialized from GPIO states. %d triggers active at startup", 
          (int)activeTriggers_.size());
    
    // Apply initial actions from active triggers (simplified direct execution)
    if (!activeTriggers_.empty()) {
        log_d("Processing initial active triggers for startup actions");
        
        // Process active triggers in priority order for startup
        for (const auto& [priority, trigger] : activeTriggers_) {
            TriggerActionRequest request = trigger->GetActionRequest();
            
            if (request.type == TriggerActionType::ToggleTheme) {
                log_i("Applying startup theme action: %s", request.panelName);
                // Note: StyleManager should be initialized before this call
                StyleManager::GetInstance().set_theme(request.panelName);
            }
            else if (request.type == TriggerActionType::LoadPanel) {
                log_i("Startup panel action detected: Load %s", request.panelName);
                startupPanelOverride_ = request.panelName;
                break; // Only use highest priority panel action
            }
        }
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

void TriggerManager::UpdateActiveTriggersList(AlertTrigger* trigger, TriggerExecutionState newState)
{
    if (newState == TriggerExecutionState::ACTIVE) {
        // Add to active triggers list if not already present
        auto it = std::find_if(activeTriggers_.begin(), activeTriggers_.end(),
            [trigger](const auto& pair) { return pair.second == trigger; });
        
        if (it == activeTriggers_.end()) {
            activeTriggers_.push_back({trigger->GetPriority(), trigger});
            
            // Maintain sort order: lowest priority number = highest priority first
            // Use stable_sort to preserve FIFO order for same-priority triggers
            std::stable_sort(activeTriggers_.begin(), activeTriggers_.end(),
                [](const auto& a, const auto& b) { 
                    return a.first < b.first; 
                });
            
            log_i("Added trigger %s to active list (priority %d)", 
                  trigger->GetId(), (int)trigger->GetPriority());
        }
    } else {
        // Remove from active triggers list
        auto it = std::remove_if(activeTriggers_.begin(), activeTriggers_.end(),
            [trigger](const auto& pair) { return pair.second == trigger; });
        
        if (it != activeTriggers_.end()) {
            activeTriggers_.erase(it, activeTriggers_.end());
            log_i("Removed trigger %s from active list", trigger->GetId());
        }
    }
    
    log_i("Active triggers list now contains %d triggers", (int)activeTriggers_.size());
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