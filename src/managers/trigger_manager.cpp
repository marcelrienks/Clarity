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
    log_d("Initializing direct GPIO polling trigger system...");

    // Configure GPIO pins as inputs
    setup_gpio_pins();
    
    log_d("Direct GPIO polling trigger system initialized");
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
    // Direct GPIO polling - check for pin changes
    CheckGpioChanges();
}

void TriggerManager::CheckGpioChanges()
{
    // Read current GPIO states
    bool keyPresent = digitalRead(gpio_pins::KEY_PRESENT);
    bool keyNotPresent = digitalRead(gpio_pins::KEY_NOT_PRESENT);
    bool lockState = digitalRead(gpio_pins::LOCK);
    bool lightsState = digitalRead(gpio_pins::LIGHTS);
    
    // Check for changes and process them
    CheckTriggerChange(TRIGGER_KEY_PRESENT, keyPresent);
    CheckTriggerChange(TRIGGER_KEY_NOT_PRESENT, keyNotPresent);
    CheckTriggerChange(TRIGGER_LOCK_STATE, lockState);
    CheckTriggerChange(TRIGGER_LIGHTS_STATE, lightsState);
}

void TriggerManager::CheckTriggerChange(const char* triggerId, bool currentPinState)
{
    AlertTrigger* trigger = FindTriggerById(triggerId);
    if (!trigger) return;
    
    TriggerExecutionState oldState = trigger->GetState();
    TriggerExecutionState newState = currentPinState ? TriggerExecutionState::ACTIVE : TriggerExecutionState::INACTIVE;
    
    // Only process if state actually changed
    if (oldState != newState) {
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
            // Check if there are other active panel triggers
            AlertTrigger* highestPanelTrigger = nullptr;
            for (const auto& [priority, activeTrigger] : activeTriggers_) {
                TriggerActionRequest activeRequest = activeTrigger->GetActionRequest();
                if (activeRequest.type == TriggerActionType::LoadPanel) {
                    highestPanelTrigger = activeTrigger;
                    break; // List is sorted by priority, so first match is highest
                }
            }
            
            if (highestPanelTrigger) {
                // Load panel from highest priority active trigger
                TriggerActionRequest activeRequest = highestPanelTrigger->GetActionRequest();
                log_i("Panel trigger deactivated but others active - loading panel: %s", activeRequest.panelName);
                PanelManager::GetInstance().CreateAndLoadPanel(activeRequest.panelName, []() {
                    PanelManager::GetInstance().TriggerPanelSwitchCallback("");
                }, activeRequest.isTriggerDriven);
            } else {
                // No other panel triggers active - restore to default
                log_i("No other panel triggers active - restoring to default: %s", restoreRequest.panelName);
                PanelManager::GetInstance().CreateAndLoadPanel(restoreRequest.panelName, []() {
                    PanelManager::GetInstance().TriggerPanelSwitchCallback("");
                }, restoreRequest.isTriggerDriven);
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
    log_d("Initializing trigger states from current GPIO pin states...");
    
    // Read current GPIO pin states and initialize triggers accordingly
    bool keyPresent = digitalRead(gpio_pins::KEY_PRESENT);
    bool keyNotPresent = digitalRead(gpio_pins::KEY_NOT_PRESENT);
    bool lockState = digitalRead(gpio_pins::LOCK);
    bool lightsState = digitalRead(gpio_pins::LIGHTS);
    
    // Initialize all triggers based on current pin states
    InitializeTrigger(TRIGGER_KEY_PRESENT, keyPresent);
    InitializeTrigger(TRIGGER_KEY_NOT_PRESENT, keyNotPresent);
    InitializeTrigger(TRIGGER_LOCK_STATE, lockState);
    InitializeTrigger(TRIGGER_LIGHTS_STATE, lightsState);
    
    // Sort initial active triggers list by priority
    std::stable_sort(activeTriggers_.begin(), activeTriggers_.end(),
        [](const auto& a, const auto& b) { return a.first < b.first; });
    
    log_d("All triggers initialized from GPIO states. %d triggers active at startup", 
          (int)activeTriggers_.size());
    
    // Apply initial actions from active triggers
    if (!activeTriggers_.empty()) {
        log_d("Processing initial active triggers for startup actions");
        
        // Process active triggers in priority order for startup
        for (const auto& [priority, trigger] : activeTriggers_) {
            TriggerActionRequest request = trigger->GetActionRequest();
            
            if (request.type == TriggerActionType::ToggleTheme) {
                log_i("Applying startup theme action: %s", request.panelName);
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

void TriggerManager::InitializeTrigger(const char* triggerId, bool currentPinState)
{
    AlertTrigger* trigger = FindTriggerById(triggerId);
    if (!trigger) return;
    
    TriggerExecutionState initialState = currentPinState ? TriggerExecutionState::ACTIVE : TriggerExecutionState::INACTIVE;
    trigger->SetState(initialState);
    
    // Initialize active triggers list
    if (initialState == TriggerExecutionState::ACTIVE) {
        activeTriggers_.push_back({trigger->GetPriority(), trigger});
    }
    
    log_d("Trigger %s initialized to %s based on GPIO state", 
          triggerId, currentPinState ? "ACTIVE" : "INACTIVE");
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
void TriggerManager::setup_gpio_pins()
{
    log_d("Setting up GPIO pins for direct polling...");

    // Configure GPIO pins as inputs with pull-down resistors
    pinMode(gpio_pins::KEY_PRESENT, INPUT_PULLDOWN);
    pinMode(gpio_pins::KEY_NOT_PRESENT, INPUT_PULLDOWN);
    pinMode(gpio_pins::LOCK, INPUT_PULLDOWN);
    pinMode(gpio_pins::LIGHTS, INPUT_PULLDOWN);
    
    log_d("GPIO pins configured successfully for direct polling");
}