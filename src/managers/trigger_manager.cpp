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

std::map<std::string, TriggerExecutionState> TriggerManager::ProcessPendingTriggerEvents()
{
    std::map<std::string, TriggerExecutionState> consolidatedStates;
    ISREvent event;
    UBaseType_t queueCount = uxQueueMessagesWaiting(isrEventQueue);
    
    if (queueCount > 0) {
        log_i("Core 0: Processing %d pending trigger events for state consolidation", queueCount);
    }
    
    // Process ALL queued trigger events in FIFO order to build consolidated state
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
                
                // Update trigger object state
                trigger->SetState(newState);
                
                // Update persistent active triggers list immediately
                UpdateActiveTriggersList(trigger, newState);
                
                // Update consolidated state map (later messages override earlier ones)
                consolidatedStates[std::string(triggerId)] = newState;
                
                log_i("*** STATE CONSOLIDATION - %s set to %s ***", 
                      triggerId, 
                      newState == TriggerExecutionState::ACTIVE ? "ACTIVE" : "INACTIVE");
            }
        }
    }
    
    if (!consolidatedStates.empty()) {
        log_i("State consolidation complete. Final states:");
        for (const auto& [triggerId, state] : consolidatedStates) {
            log_i("  %s: %s", triggerId.c_str(), state == TriggerExecutionState::ACTIVE ? "ACTIVE" : "INACTIVE");
        }
    }
    
    return consolidatedStates;
}

ExecutionPlan TriggerManager::PlanExecutionFromStates(const std::map<std::string, TriggerExecutionState>& consolidatedStates)
{
    ExecutionPlan plan;
    
    log_i("Planning execution from consolidated states using persistent active triggers list...");
    
    // Process ONLY changed triggers for theme and non-panel actions
    std::vector<std::pair<TriggerPriority, std::pair<AlertTrigger*, TriggerExecutionState>>> changedTriggers;
    
    for (const auto& [triggerId, state] : consolidatedStates) {
        AlertTrigger* trigger = FindTriggerById(triggerId.c_str());
        if (trigger) {
            changedTriggers.push_back({trigger->GetPriority(), {trigger, state}});
            log_i("Trigger state change found: %s (priority %d, state %s)", 
                  triggerId.c_str(), (int)trigger->GetPriority(),
                  state == TriggerExecutionState::ACTIVE ? "ACTIVE" : "INACTIVE");
        }
    }
    
    // Use pre-maintained activeTriggers_ list instead of rebuilding
    log_i("Using persistent active triggers list with %d active triggers", (int)activeTriggers_.size());
    for (const auto& [priority, trigger] : activeTriggers_) {
        log_i("Active trigger: %s (priority %d)", trigger->GetId(), (int)priority);
    }
    
    // Sort changed triggers by priority for action/theme processing
    std::sort(changedTriggers.begin(), changedTriggers.end(),
        [](const auto& a, const auto& b) { return a.first > b.first; });
    
    // activeTriggers_ is already sorted by priority during maintenance
    
    // Process state changes for theme and non-panel actions
    for (const auto& [priority, triggerStatePair] : changedTriggers) {
        auto [trigger, state] = triggerStatePair;
        
        // Get appropriate request based on state
        auto request = (state == TriggerExecutionState::ACTIVE) 
                      ? trigger->GetActionRequest() 
                      : trigger->GetRestoreRequest();
        
        if (request.type == TriggerActionType::ToggleTheme) {
            log_i("Adding theme action from trigger: %s", trigger->GetId());
            plan.themeActions.push_back(request);
        } else if (request.type == TriggerActionType::RestorePanel) {
            // RestorePanel requests are stored to influence panel planning
            log_i("RestorePanel request from trigger: %s (will be considered in panel planning)", trigger->GetId());
            // Don't add to nonPanelActions - will be handled in panel planning below
        } else if (request.type != TriggerActionType::LoadPanel) {
            log_i("Adding non-panel action from trigger: %s", trigger->GetId());
            plan.nonPanelActions.push_back(request);
        }
        // Note: LoadPanel requests from changed triggers are ignored here
        // Panel planning uses ALL currently active triggers instead
    }
    
    // Determine final panel from persistent activeTriggers_ list
    AlertTrigger* highestPriorityPanelTrigger = nullptr;
    TriggerPriority highestPanelPriority = TriggerPriority::NORMAL;
    
    for (const auto& [priority, trigger] : activeTriggers_) {
        auto request = trigger->GetActionRequest(); // Active triggers use action request
        
        if (request.type == TriggerActionType::LoadPanel) {
            // Track highest priority panel trigger (lowest priority number = highest priority)
            // For same priority triggers, later triggers override (FIFO behavior)
            if (!highestPriorityPanelTrigger || priority <= highestPanelPriority) {
                highestPriorityPanelTrigger = trigger;
                highestPanelPriority = priority;
                log_i("Highest priority active panel trigger: %s (priority %d)", trigger->GetId(), (int)priority);
            }
        }
    }
    
    // Set final panel action based on active triggers
    if (highestPriorityPanelTrigger) {
        plan.finalPanelAction = highestPriorityPanelTrigger->GetActionRequest();
        log_i("Final panel action: Load %s from active trigger %s", 
              plan.finalPanelAction.panelName, 
              highestPriorityPanelTrigger->GetId());
    } else if (!plan.themeActions.empty()) {
        // Only theme actions present - no panel change needed
        plan.finalPanelAction = {TriggerActionType::None, nullptr, "system", false};
        log_i("Final panel action: None (only theme actions present)");
    } else {
        // No active panel triggers and no theme actions - restore to default panel
        plan.finalPanelAction = {TriggerActionType::RestorePanel, nullptr, "system", true};
        if (activeTriggers_.empty()) {
            log_i("Final panel action: Restore panel (no active triggers)");
        } else {
            log_i("Final panel action: Restore panel (no active panel triggers, %d non-panel triggers remain)", (int)activeTriggers_.size());
        }
    }
    
    log_i("Execution plan complete: %d theme actions, %d non-panel actions, 1 final panel action",
          (int)plan.themeActions.size(), (int)plan.nonPanelActions.size());
    
    return plan;
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
    
    // Apply initial theme actions from active triggers
    if (!activeTriggers_.empty()) {
        log_d("Processing initial active triggers for startup theme application");
        
        // Build initial states map for active triggers
        std::map<std::string, TriggerExecutionState> initialStates;
        for (const auto& [priority, trigger] : activeTriggers_) {
            initialStates[trigger->GetId()] = TriggerExecutionState::ACTIVE;
        }
        
        // Plan execution from initial states
        ExecutionPlan plan = PlanExecutionFromStates(initialStates);
        
        // Apply theme actions immediately during startup
        for (const auto& request : plan.themeActions) {
            log_i("Applying startup theme action: %s", request.panelName);
            // Note: StyleManager should be initialized before this call
            StyleManager::GetInstance().set_theme(request.panelName);
        }
        
        // Store panel action for startup loading (will be used instead of config default)
        if (plan.finalPanelAction.type == TriggerActionType::LoadPanel) {
            log_i("Startup panel action detected: Load %s", plan.finalPanelAction.panelName);
            startupPanelOverride_ = plan.finalPanelAction.panelName;
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