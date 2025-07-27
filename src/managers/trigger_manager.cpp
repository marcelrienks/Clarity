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
    
    // Initialize trigger mappings
    InitializeTriggerMappings();
    
    // Read current GPIO states and initialize triggers
    InitializeTriggersFromGpio();
}

void TriggerManager::InitializeTriggerMappings()
{
    log_d("Initializing trigger mappings...");
    
    // Initialize all trigger mappings with their configurations
    for (auto& mapping : triggerMappings_) {
        mapping.currentState = TriggerExecutionState::INACTIVE;
        log_d("Trigger mapping initialized: %s", mapping.triggerId);
    }
}

void TriggerManager::ProcessTriggerEvents()
{
    // Direct GPIO polling - check for pin changes
    CheckGpioChanges();
}

GpioState TriggerManager::ReadAllGpioPins()
{
    // Single consolidated GPIO read for all trigger pins
    GpioState state;
    state.keyPresent = digitalRead(gpio_pins::KEY_PRESENT);
    state.keyNotPresent = digitalRead(gpio_pins::KEY_NOT_PRESENT);
    state.lockState = digitalRead(gpio_pins::LOCK);
    state.lightsState = digitalRead(gpio_pins::LIGHTS);
    return state;
}

void TriggerManager::CheckGpioChanges()
{
    // Read all GPIO states once
    GpioState currentState = ReadAllGpioPins();
    
    // Check for changes and process them
    CheckTriggerChange(TRIGGER_KEY_PRESENT, currentState.keyPresent);
    CheckTriggerChange(TRIGGER_KEY_NOT_PRESENT, currentState.keyNotPresent);
    CheckTriggerChange(TRIGGER_LOCK_STATE, currentState.lockState);
    CheckTriggerChange(TRIGGER_LIGHTS_STATE, currentState.lightsState);
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
        UpdateActiveTriggersSimple(mapping, newState);
        
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
            if (activePanelTrigger_) {
                // Load panel from remaining active trigger
                log_i("Panel trigger deactivated but another active - loading panel: %s", activePanelTrigger_->actionTarget);
                PanelManager::GetInstance().CreateAndLoadPanel(activePanelTrigger_->actionTarget, []() {
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
    
    // Read all GPIO pin states once using consolidated method
    GpioState currentState = ReadAllGpioPins();
    
    // Initialize all triggers based on current pin states
    InitializeTrigger(TRIGGER_KEY_PRESENT, currentState.keyPresent);
    InitializeTrigger(TRIGGER_KEY_NOT_PRESENT, currentState.keyNotPresent);
    InitializeTrigger(TRIGGER_LOCK_STATE, currentState.lockState);
    InitializeTrigger(TRIGGER_LIGHTS_STATE, currentState.lightsState);
    
    // Apply initial actions from active triggers
    if (activeThemeTrigger_) {
        log_i("Applying startup theme action: %s", activeThemeTrigger_->actionTarget);
        StyleManager::GetInstance().set_theme(activeThemeTrigger_->actionTarget);
    }
    
    if (activePanelTrigger_) {
        log_i("Startup panel action detected: Load %s", activePanelTrigger_->actionTarget);
        startupPanelOverride_ = activePanelTrigger_->actionTarget;
    }
}

void TriggerManager::InitializeTrigger(const char* triggerId, bool currentPinState)
{
    TriggerMapping* mapping = FindTriggerMapping(triggerId);
    if (!mapping) return;
    
    TriggerExecutionState initialState = currentPinState ? TriggerExecutionState::ACTIVE : TriggerExecutionState::INACTIVE;
    mapping->currentState = initialState;
    
    // Update simplified active trigger tracking
    if (initialState == TriggerExecutionState::ACTIVE) {
        UpdateActiveTriggersSimple(mapping, initialState);
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

void TriggerManager::UpdateActiveTriggersSimple(TriggerMapping* mapping, TriggerExecutionState newState)
{
    if (newState == TriggerExecutionState::ACTIVE) {
        if (mapping->actionType == TriggerActionType::LoadPanel) {
            // Update active panel trigger if this is higher priority
            if (!activePanelTrigger_ || mapping->priority < activePanelTrigger_->priority) {
                activePanelTrigger_ = mapping;
                log_i("Set active panel trigger: %s (priority %d)", mapping->triggerId, (int)mapping->priority);
            }
        } else if (mapping->actionType == TriggerActionType::ToggleTheme) {
            activeThemeTrigger_ = mapping;
            log_i("Set active theme trigger: %s", mapping->triggerId);
        }
    } else {
        if (mapping->actionType == TriggerActionType::LoadPanel && activePanelTrigger_ == mapping) {
            // Find next highest priority active panel trigger
            activePanelTrigger_ = nullptr;
            for (auto& checkMapping : triggerMappings_) {
                if (checkMapping.actionType == TriggerActionType::LoadPanel && 
                    checkMapping.currentState == TriggerExecutionState::ACTIVE &&
                    &checkMapping != mapping) {
                    if (!activePanelTrigger_ || checkMapping.priority < activePanelTrigger_->priority) {
                        activePanelTrigger_ = &checkMapping;
                    }
                }
            }
            log_i("Removed panel trigger %s, new active: %s", mapping->triggerId, 
                  activePanelTrigger_ ? activePanelTrigger_->triggerId : "none");
        } else if (mapping->actionType == TriggerActionType::ToggleTheme && activeThemeTrigger_ == mapping) {
            activeThemeTrigger_ = nullptr;
            log_i("Removed theme trigger: %s", mapping->triggerId);
        }
    }
}
void TriggerManager::setup_gpio_pins()
{
    log_d("Setting up GPIO pins for direct polling...");

    // Configure GPIO pins as inputs with pull-down resistors
    pinMode(gpio_pins::KEY_PRESENT, INPUT_PULLDOWN);
    pinMode(gpio_pins::KEY_NOT_PRESENT, INPUT_PULLDOWN);
    pinMode(gpio_pins::LOCK, INPUT_PULLDOWN);
    pinMode(gpio_pins::LIGHTS, INPUT_PULLDOWN);
}