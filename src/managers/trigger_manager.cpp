#include "managers/trigger_manager.h"
#include "managers/panel_manager.h"
#include "managers/preference_manager.h"
#include "managers/style_manager.h"
#include "hardware/gpio_pins.h"
#include <algorithm>

// Define the trigger mappings array
TriggerMapping TriggerManager::triggerMappings_[] = {
    {TRIGGER_KEY_PRESENT, gpio_pins::KEY_PRESENT, TriggerActionType::LoadPanel, PanelNames::KEY, PanelNames::OIL, TriggerPriority::CRITICAL},
    {TRIGGER_KEY_NOT_PRESENT, gpio_pins::KEY_NOT_PRESENT, TriggerActionType::LoadPanel, PanelNames::KEY, PanelNames::OIL, TriggerPriority::CRITICAL},
    {TRIGGER_LOCK_STATE, gpio_pins::LOCK, TriggerActionType::LoadPanel, PanelNames::LOCK, PanelNames::OIL, TriggerPriority::IMPORTANT},
    {TRIGGER_LIGHTS_STATE, gpio_pins::LIGHTS, TriggerActionType::ToggleTheme, Themes::NIGHT, Themes::DAY, TriggerPriority::NORMAL}
};

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

void TriggerManager::InitializeTriggerMappings()
{
    log_d("Initializing trigger mappings...");
    
    // Initialize all trigger mappings with their configurations
    for (auto& mapping : triggerMappings_) {
        mapping.currentState = TriggerExecutionState::INACTIVE;
        log_d("Trigger mapping initialized: %s", mapping.triggerId);
    }
    
    log_d("All trigger mappings initialized successfully");
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
    TriggerMapping* mapping = FindTriggerMapping(triggerId);
    if (!mapping) return;
    
    TriggerExecutionState oldState = mapping->currentState;
    TriggerExecutionState newState = currentPinState ? TriggerExecutionState::ACTIVE : TriggerExecutionState::INACTIVE;
    
    // Only process if state actually changed
    if (oldState != newState) {
        log_i("Trigger %s: %s -> %s", 
              triggerId, 
              oldState == TriggerExecutionState::ACTIVE ? "ACTIVE" : "INACTIVE",
              newState == TriggerExecutionState::ACTIVE ? "ACTIVE" : "INACTIVE");
        
        mapping->currentState = newState;
        UpdateActiveTriggersList(mapping, newState);
        
        // Execute trigger action immediately
        ExecuteTriggerAction(mapping, newState);
    }
}

void TriggerManager::ExecuteTriggerAction(TriggerMapping* mapping, TriggerExecutionState state)
{
    if (state == TriggerExecutionState::ACTIVE) {
        // Execute trigger action when activated
        if (mapping->actionType == TriggerActionType::LoadPanel) {
            log_i("Executing panel action: Load %s", mapping->actionTarget);
            PanelManager::GetInstance().CreateAndLoadPanel(mapping->actionTarget, []() {
                PanelManager::GetInstance().TriggerPanelSwitchCallback("");
            }, true);
        }
        else if (mapping->actionType == TriggerActionType::ToggleTheme) {
            log_i("Executing theme action: %s", mapping->actionTarget);
            StyleManager::GetInstance().set_theme(mapping->actionTarget);
            // Refresh current panel with new theme
            PanelManager::GetInstance().UpdatePanel();
        }
    } else {
        // Handle trigger deactivation - execute restore action
        if (mapping->actionType == TriggerActionType::LoadPanel) {
            // Check if there are other active panel triggers
            TriggerMapping* highestPanelMapping = nullptr;
            for (const auto& [priority, activeMapping] : activeTriggers_) {
                if (activeMapping->actionType == TriggerActionType::LoadPanel) {
                    highestPanelMapping = activeMapping;
                    break; // List is sorted by priority, so first match is highest
                }
            }
            
            if (highestPanelMapping) {
                // Load panel from highest priority active trigger
                log_i("Panel trigger deactivated but others active - loading panel: %s", highestPanelMapping->actionTarget);
                PanelManager::GetInstance().CreateAndLoadPanel(highestPanelMapping->actionTarget, []() {
                    PanelManager::GetInstance().TriggerPanelSwitchCallback("");
                }, true);
            } else {
                // No other panel triggers active - restore to default
                log_i("No other panel triggers active - restoring to default: %s", mapping->restoreTarget);
                PanelManager::GetInstance().CreateAndLoadPanel(mapping->restoreTarget, []() {
                    PanelManager::GetInstance().TriggerPanelSwitchCallback("");
                }, false);
            }
        }
        else if (mapping->actionType == TriggerActionType::ToggleTheme) {
            log_i("Restoring theme: %s", mapping->restoreTarget);
            StyleManager::GetInstance().set_theme(mapping->restoreTarget);
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
        for (const auto& [priority, mapping] : activeTriggers_) {
            if (mapping->actionType == TriggerActionType::ToggleTheme) {
                log_i("Applying startup theme action: %s", mapping->actionTarget);
                StyleManager::GetInstance().set_theme(mapping->actionTarget);
            }
            else if (mapping->actionType == TriggerActionType::LoadPanel) {
                log_i("Startup panel action detected: Load %s", mapping->actionTarget);
                startupPanelOverride_ = mapping->actionTarget;
                break; // Only use highest priority panel action
            }
        }
    }
}

void TriggerManager::InitializeTrigger(const char* triggerId, bool currentPinState)
{
    TriggerMapping* mapping = FindTriggerMapping(triggerId);
    if (!mapping) return;
    
    TriggerExecutionState initialState = currentPinState ? TriggerExecutionState::ACTIVE : TriggerExecutionState::INACTIVE;
    mapping->currentState = initialState;
    
    // Initialize active triggers list
    if (initialState == TriggerExecutionState::ACTIVE) {
        activeTriggers_.push_back({mapping->priority, mapping});
    }
    
    log_d("Trigger %s initialized to %s based on GPIO state", 
          triggerId, currentPinState ? "ACTIVE" : "INACTIVE");
}

TriggerMapping* TriggerManager::FindTriggerMapping(const char* triggerId)
{
    for (auto& mapping : triggerMappings_)
    {
        if (strcmp(mapping.triggerId, triggerId) == 0)
        {
            return &mapping;
        }
    }
    return nullptr;
}

void TriggerManager::UpdateActiveTriggersList(TriggerMapping* mapping, TriggerExecutionState newState)
{
    if (newState == TriggerExecutionState::ACTIVE) {
        // Add to active triggers list if not already present
        auto it = std::find_if(activeTriggers_.begin(), activeTriggers_.end(),
            [mapping](const auto& pair) { return pair.second == mapping; });
        
        if (it == activeTriggers_.end()) {
            activeTriggers_.push_back({mapping->priority, mapping});
            
            // Maintain sort order: lowest priority number = highest priority first
            // Use stable_sort to preserve FIFO order for same-priority triggers
            std::stable_sort(activeTriggers_.begin(), activeTriggers_.end(),
                [](const auto& a, const auto& b) { 
                    return a.first < b.first; 
                });
            
            log_i("Added trigger %s to active list (priority %d)", 
                  mapping->triggerId, (int)mapping->priority);
        }
    } else {
        // Remove from active triggers list
        auto it = std::remove_if(activeTriggers_.begin(), activeTriggers_.end(),
            [mapping](const auto& pair) { return pair.second == mapping; });
        
        if (it != activeTriggers_.end()) {
            activeTriggers_.erase(it, activeTriggers_.end());
            log_i("Removed trigger %s from active list", mapping->triggerId);
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