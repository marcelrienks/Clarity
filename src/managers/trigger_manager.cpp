#include "managers/trigger_manager.h"
#include "managers/panel_manager.h"
#include "managers/preference_manager.h"
#include "managers/style_manager.h"
#include "managers/error_manager.h"
#include "hardware/gpio_pins.h"
#include <esp32-hal-log.h>
#include <algorithm>

// Define the trigger mappings array
Trigger TriggerManager::triggers_[] = {
    {TRIGGER_KEY_PRESENT, gpio_pins::KEY_PRESENT, TriggerActionType::LoadPanel, PanelNames::KEY, PanelNames::OIL, TriggerPriority::CRITICAL},
    {TRIGGER_KEY_NOT_PRESENT, gpio_pins::KEY_NOT_PRESENT, TriggerActionType::LoadPanel, PanelNames::KEY, PanelNames::OIL, TriggerPriority::CRITICAL},
    {TRIGGER_LOCK_STATE, gpio_pins::LOCK, TriggerActionType::LoadPanel, PanelNames::LOCK, PanelNames::OIL, TriggerPriority::IMPORTANT},
    {TRIGGER_LIGHTS_STATE, gpio_pins::LIGHTS, TriggerActionType::ToggleTheme, Themes::NIGHT, Themes::DAY, TriggerPriority::NORMAL},
    {TRIGGER_ERROR_OCCURRED, -1, TriggerActionType::LoadPanel, PanelNames::ERROR, PanelNames::OIL, TriggerPriority::CRITICAL}
};

TriggerManager::TriggerManager(std::shared_ptr<KeySensor> keySensor, std::shared_ptr<LockSensor> lockSensor, 
                               std::shared_ptr<LightSensor> lightSensor, std::shared_ptr<DebugErrorSensor> debugErrorSensor,
                               IPanelService *panelService, IStyleService *styleService)
    : keySensor_(keySensor), lockSensor_(lockSensor), lightSensor_(lightSensor), debugErrorSensor_(debugErrorSensor),
      panelService_(panelService), styleService_(styleService)
{
    if (!keySensor || !lockSensor || !lightSensor || !debugErrorSensor || !panelService || !styleService) {
        log_e("TriggerManager requires all dependencies: keySensor, lockSensor, lightSensor, debugErrorSensor, panelService, and styleService");
        // In a real embedded system, you might want to handle this more gracefully
    } else {
        log_d("Creating TriggerManager with injected sensor and service dependencies");
    }
}

const char *TriggerManager::GetStartupPanelOverride() const {
    // Check key presence on startup using sensor
    if (keySensor_->GetKeyState() == KeyState::Present) {
        return PanelNames::KEY;
    }
    return nullptr; // No override needed
}

void TriggerManager::Init()
{
    // Prevent double initialization
    if (initialized_) {
        log_d("TriggerManager already initialized, skipping...");
        return;
    }
    
    log_d("Initializing sensor-based trigger system...");

    // Initialize sensors (they handle their own GPIO setup)
    keySensor_->Init();
    lockSensor_->Init();
    lightSensor_->Init();
    debugErrorSensor_->Init();
    
    // Read current sensor states and initialize triggers
    InitializeTriggersFromSensors();
    
    initialized_ = true;
}

void TriggerManager::ProcessTriggerEvents()
{
    // Sensor-based polling - check for state changes
    CheckSensorChanges();
    
    // Check for error conditions
    CheckErrorTrigger();
    
    // Check debug error sensor (it handles error generation internally)
    #ifdef CLARITY_DEBUG
    debugErrorSensor_->GetReading(); // This will trigger errors on rising edge
    #endif
}

GpioState TriggerManager::ReadAllSensorStates()
{
    // Single consolidated sensor read for all trigger states
    GpioState state;
    
    // Read key state from sensor
    KeyState keyState = keySensor_->GetKeyState();
    state.keyPresent = (keyState == KeyState::Present);
    state.keyNotPresent = (keyState == KeyState::NotPresent);
    
    // Read lock state from sensor  
    auto lockReading = lockSensor_->GetReading();
    state.lockState = std::get<bool>(lockReading);
    
    // Read lights state from sensor
    state.lightsState = lightSensor_->GetLightsState();
    
    return state;
}

void TriggerManager::CheckSensorChanges()
{
    // Read all sensor states once
    GpioState currentState = ReadAllSensorStates();
    
    // Check for changes and process them
    CheckTriggerChange(TRIGGER_KEY_PRESENT, currentState.keyPresent);
    CheckTriggerChange(TRIGGER_KEY_NOT_PRESENT, currentState.keyNotPresent);
    CheckTriggerChange(TRIGGER_LOCK_STATE, currentState.lockState);
    CheckTriggerChange(TRIGGER_LIGHTS_STATE, currentState.lightsState);
}

void TriggerManager::CheckErrorTrigger()
{
    bool shouldShowErrorPanel = ErrorManager::Instance().ShouldTriggerErrorPanel();
    CheckTriggerChange(TRIGGER_ERROR_OCCURRED, shouldShowErrorPanel);
}

void TriggerManager::CheckTriggerChange(const char *triggerId, bool currentPinState)
{
    Trigger *mapping = FindTriggerMapping(triggerId);
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

void TriggerManager::ExecuteTriggerAction(Trigger *mapping, TriggerExecutionState state)
{
    if (state == TriggerExecutionState::ACTIVE) {
        // Execute trigger action when activated
        if (mapping->actionType == TriggerActionType::LoadPanel) {
            log_i("Executing panel action: Load %s", mapping->actionTarget);
            if (panelService_) {
                panelService_->CreateAndLoadPanel(mapping->actionTarget, []() {
                    // Panel switch callback handled by service
                }, true);
            }
        }
        else if (mapping->actionType == TriggerActionType::ToggleTheme) {
            log_i("Executing theme action: %s", mapping->actionTarget);
            if (styleService_) {
                styleService_->SetTheme(mapping->actionTarget);
                // Refresh current panel with new theme
                if (panelService_) {
                    panelService_->UpdatePanel();
                }
            }
        }
    } else {
        // Handle trigger deactivation - execute restore action
        if (mapping->actionType == TriggerActionType::LoadPanel) {
            if (activePanelTrigger_) {
                // Load panel from remaining active trigger
                log_i("Panel trigger deactivated but another active - loading panel: %s", activePanelTrigger_->actionTarget);
                if (panelService_) {
                    panelService_->CreateAndLoadPanel(activePanelTrigger_->actionTarget, []() {
                        // Panel switch callback handled by service
                    }, true);
                }
            } else {
                // Note: UIState checking removed - not available in IPanelService interface
                // Panel service will handle state management internally
                
                // No other panel triggers active - restore to default
                log_i("No other panel triggers active - restoring to default: %s", mapping->restoreTarget);
                if (panelService_) {
                    panelService_->CreateAndLoadPanel(mapping->restoreTarget, []() {
                        // Panel switch callback handled by service
                    }, false);
                }
            }
        }
        else if (mapping->actionType == TriggerActionType::ToggleTheme) {
            // Note: UIState checking removed - not available in IPanelService interface
            // Panel service will handle state management internally
            
            log_i("Restoring theme: %s", mapping->restoreTarget);
            if (styleService_) {
                styleService_->SetTheme(mapping->restoreTarget);
                // Refresh current panel with restored theme
                if (panelService_) {
                    panelService_->UpdatePanel();
                }
            }
        }
    }
}

void TriggerManager::InitializeTriggersFromSensors()
{
    log_d("Initializing trigger states from current sensor states...");
    
    // Read all sensor states once using consolidated method
    GpioState currentState = ReadAllSensorStates();
    
    // Initialize all triggers based on current pin states
    InitializeTrigger(TRIGGER_KEY_PRESENT, currentState.keyPresent);
    InitializeTrigger(TRIGGER_KEY_NOT_PRESENT, currentState.keyNotPresent);
    InitializeTrigger(TRIGGER_LOCK_STATE, currentState.lockState);
    InitializeTrigger(TRIGGER_LIGHTS_STATE, currentState.lightsState);
    
    // Initialize error trigger based on current error state
    InitializeTrigger(TRIGGER_ERROR_OCCURRED, ErrorManager::Instance().ShouldTriggerErrorPanel());
    
    // Apply initial actions from active triggers
    if (activeThemeTrigger_) {
        log_i("Applying startup theme action: %s", activeThemeTrigger_->actionTarget);
        if (styleService_) {
            styleService_->SetTheme(activeThemeTrigger_->actionTarget);
        }
    }
    
    if (activePanelTrigger_) {
        log_i("Startup panel action detected: Load %s", activePanelTrigger_->actionTarget);
        startupPanelOverride_ = activePanelTrigger_->actionTarget;
    }
}

void TriggerManager::InitializeTrigger(const char *triggerId, bool currentPinState)
{
    Trigger *mapping = FindTriggerMapping(triggerId);
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

Trigger *TriggerManager::FindTriggerMapping(const char *triggerId)
{
    for (auto& trigger : triggers_)
    {
        if (trigger.triggerId == triggerId)
        {
            return &trigger;
        }
    }
    return nullptr;
}

void TriggerManager::UpdateActiveTriggersSimple(Trigger *mapping, TriggerExecutionState newState)
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
            for (auto& checkMapping : triggers_) {
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

void TriggerManager::AddTrigger(const std::string& triggerName, ISensor *sensor, std::function<void()> callback) {
    log_d("Adding trigger %s", triggerName.c_str());
    // Currently a no-op since triggers are statically defined
    // This interface method is part of ITriggerService interface
}

bool TriggerManager::HasTrigger(const std::string& triggerName) const {
    log_d("Checking for trigger %s", triggerName.c_str());
    // Check if the trigger exists in our static mapping
    for (const auto& trigger : triggers_) {
        if (trigger.triggerId == triggerName) {
            return true;
        }
    }
    return false;
}