#include "managers/trigger_manager.h"
#include "hardware/gpio_pins.h"
#include "managers/error_manager.h"
#include "managers/interrupt_manager.h"
#include "managers/panel_manager.h"
#include "managers/preference_manager.h"
#include "managers/style_manager.h"
#include <algorithm>
#include <esp32-hal-log.h>

// Define the trigger mappings array
Trigger TriggerManager::triggers_[] = {{TriggerIds::KEY_PRESENT, gpio_pins::KEY_PRESENT, TriggerActionType::LoadPanel,
                                        PanelNames::KEY, PanelNames::OIL, TriggerPriority::CRITICAL},
                                       {TriggerIds::KEY_NOT_PRESENT, gpio_pins::KEY_NOT_PRESENT,
                                        TriggerActionType::LoadPanel, PanelNames::KEY, PanelNames::OIL,
                                        TriggerPriority::CRITICAL},
                                       {TriggerIds::LOCK_STATE, gpio_pins::LOCK, TriggerActionType::LoadPanel,
                                        PanelNames::LOCK, PanelNames::OIL, TriggerPriority::IMPORTANT},
                                       {TriggerIds::LIGHTS_STATE, gpio_pins::LIGHTS, TriggerActionType::ToggleTheme,
                                        Themes::NIGHT, Themes::DAY, TriggerPriority::NORMAL},
                                       {TriggerIds::ERROR_OCCURRED, -1, TriggerActionType::LoadPanel, PanelNames::ERROR,
                                        PanelNames::OIL, TriggerPriority::CRITICAL}};

TriggerManager::TriggerManager(std::shared_ptr<KeySensor> keySensor, std::shared_ptr<LockSensor> lockSensor,
                               std::shared_ptr<LightsSensor> lightSensor,
                               std::shared_ptr<DebugErrorSensor> debugErrorSensor, IPanelService *panelService,
                               IStyleService *styleService)
    : keySensor_(keySensor), lockSensor_(lockSensor), lightSensor_(lightSensor), debugErrorSensor_(debugErrorSensor),
      panelService_(panelService), styleService_(styleService)
{
    log_v("TriggerManager() constructor called");
    if (!keySensor || !lockSensor || !lightSensor || !debugErrorSensor || !panelService || !styleService)
    {
        log_e("TriggerManager requires all dependencies: keySensor, lockSensor, lightSensor, debugErrorSensor, "
              "panelService, and styleService");
        // In a real embedded system, you might want to handle this more gracefully
    }
    else
    {
        log_d("Creating TriggerManager with injected sensor and service dependencies");
    }
}

const char *TriggerManager::GetStartupPanelOverride() const
{
    log_v("GetStartupPanelOverride() called");
    // Check key presence on startup using sensor
    if (keySensor_->GetKeyState() == KeyState::Present)
    {
        return PanelNames::KEY;
    }
    return nullptr; // No override needed
}

void TriggerManager::Init()
{
    log_v("Init() called");
    // Prevent double initialization
    if (initialized_)
    {
        log_d("TriggerManager already initialized, skipping...");
        return;
    }

    log_d("Initializing interrupt-based trigger system...");

    // Initialize sensors (they handle their own GPIO setup)
    keySensor_->Init();
    lockSensor_->Init();
    lightSensor_->Init();
    debugErrorSensor_->Init();

    // Register interrupts for each trigger instead of polling
    RegisterTriggerInterrupts();

    // Read current sensor states and initialize triggers
    InitializeTriggersFromSensors();

    initialized_ = true;
    log_i("TriggerManager initialization completed - interrupt-based triggers registered");
}

// Static callback functions for interrupt system
bool TriggerManager::EvaluateTriggerChange(void* context)
{
    log_v("EvaluateTriggerChange() called");
    
    if (!context)
    {
        log_e("Null context in trigger interrupt evaluation");
        return false;
    }
    
    auto* manager = static_cast<TriggerManager*>(context);
    
    // Check if any sensor states have changed
    GpioState currentState = manager->ReadAllSensorStates();
    
    // Compare with last known state (simplified check for any change)
    for (const auto& trigger : manager->triggers_)
    {
        bool currentTriggerState = false;
        
        if (strcmp(trigger.triggerId, TriggerIds::KEY_PRESENT) == 0)
        {
            currentTriggerState = currentState.keyPresent;
        }
        else if (strcmp(trigger.triggerId, TriggerIds::KEY_NOT_PRESENT) == 0)
        {
            currentTriggerState = currentState.keyNotPresent;
        }
        else if (strcmp(trigger.triggerId, TriggerIds::LOCK_STATE) == 0)
        {
            currentTriggerState = currentState.lockState;
        }
        else if (strcmp(trigger.triggerId, TriggerIds::LIGHTS_STATE) == 0)
        {
            currentTriggerState = currentState.lightsState;
        }
        else if (strcmp(trigger.triggerId, TriggerIds::ERROR_OCCURRED) == 0)
        {
            currentTriggerState = ErrorManager::Instance().ShouldTriggerErrorPanel();
        }
        
        TriggerExecutionState newState = currentTriggerState ? TriggerExecutionState::ACTIVE : TriggerExecutionState::INACTIVE;
        if (trigger.currentState != newState)
        {
            log_d("Trigger state change detected for %s", trigger.triggerId);
            return true; // State change detected
        }
    }
    
    return false; // No changes
}

void TriggerManager::ExecuteTriggerAction(void* context)
{
    log_v("ExecuteTriggerAction() called");
    
    if (!context)
    {
        log_e("Null context in trigger interrupt execution");
        return;
    }
    
    auto* manager = static_cast<TriggerManager*>(context);
    manager->CheckSensorChanges(); // Use existing logic to process changes
}

void TriggerManager::RegisterTriggerInterrupts()
{
    log_v("RegisterTriggerInterrupts() called");
    
    // Register a single polled interrupt for all trigger monitoring
    Interrupt triggerInterrupt = {};
    triggerInterrupt.id = "trigger_monitor";
    triggerInterrupt.priority = Priority::IMPORTANT;
    triggerInterrupt.source = InterruptSource::POLLED;
    triggerInterrupt.effect = InterruptEffect::LOAD_PANEL;
    triggerInterrupt.evaluationFunc = EvaluateTriggerChange;
    triggerInterrupt.executionFunc = ExecuteTriggerAction;
    triggerInterrupt.context = this;
    triggerInterrupt.active = true;
    triggerInterrupt.lastEvaluation = 0;
    
    bool registered = InterruptManager::Instance().RegisterInterrupt(triggerInterrupt);
    if (registered)
    {
        log_d("Registered trigger monitoring interrupt successfully");
    }
    else
    {
        log_e("Failed to register trigger monitoring interrupt");
    }
}

void TriggerManager::ProcessTriggerEvents()
{
    log_v("ProcessTriggerEvents() called");
    // Phase 4: ProcessTriggerEvents is now legacy - interrupts handle trigger processing
    // Keep minimal functionality for backward compatibility
    
    // Only check debug error sensor for backward compatibility
    if (debugErrorSensor_)
    {
        static unsigned long debugCallCount = 0;
        debugCallCount++;
        if (debugCallCount % 10000 == 0)
        { // Log every 10000 calls to reduce noise
            log_d("TriggerManager: Legacy ProcessTriggerEvents called %lu times", debugCallCount);
        }
        debugErrorSensor_->GetReading(); // This will trigger errors on rising edge
    }
    
    // All other trigger processing is now handled by interrupt system
}

GpioState TriggerManager::ReadAllSensorStates()
{
    log_v("ReadAllSensorStates() called");
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
    log_v("CheckSensorChanges() called");
    // Read all sensor states once
    GpioState currentState = ReadAllSensorStates();

    // Check for changes and process them
    CheckTriggerChange(TriggerIds::KEY_PRESENT, currentState.keyPresent);
    CheckTriggerChange(TriggerIds::KEY_NOT_PRESENT, currentState.keyNotPresent);
    CheckTriggerChange(TriggerIds::LOCK_STATE, currentState.lockState);
    CheckTriggerChange(TriggerIds::LIGHTS_STATE, currentState.lightsState);
}

void TriggerManager::CheckErrorTrigger()
{
    log_v("CheckErrorTrigger() called");
    bool shouldShowErrorPanel = ErrorManager::Instance().ShouldTriggerErrorPanel();
    CheckTriggerChange(TriggerIds::ERROR_OCCURRED, shouldShowErrorPanel);
}

void TriggerManager::CheckTriggerChange(const char *triggerId, bool currentPinState)
{
    log_v("CheckTriggerChange() called");
    Trigger *mapping = FindTriggerMapping(triggerId);
    if (!mapping)
        return;

    TriggerExecutionState oldState = mapping->currentState;
    TriggerExecutionState newState = currentPinState ? TriggerExecutionState::ACTIVE : TriggerExecutionState::INACTIVE;

    // Only process if state actually changed
    if (oldState != newState)
    {
        log_i("Trigger %s: %s -> %s", triggerId, oldState == TriggerExecutionState::ACTIVE ? "ACTIVE" : "INACTIVE",
              newState == TriggerExecutionState::ACTIVE ? "ACTIVE" : "INACTIVE");

        mapping->currentState = newState;

        // Execute trigger action immediately - no complex state tracking needed
        ExecuteTriggerAction(mapping, newState);
    }
}

void TriggerManager::ExecuteTriggerAction(Trigger *mapping, TriggerExecutionState state)
{
    log_v("ExecuteTriggerAction() called");
    if (!mapping) return;
    
    if (state == TriggerExecutionState::ACTIVE)
    {
        ExecuteActivation(mapping);
        return;
    }
    
    ExecuteDeactivation(mapping);
}

void TriggerManager::ExecuteActivation(Trigger *mapping)
{
    if (mapping->actionType == TriggerActionType::LoadPanel)
    {
        LoadPanel(mapping->actionTarget);
        return;
    }
    
    if (mapping->actionType == TriggerActionType::ToggleTheme)
    {
        SetTheme(mapping->actionTarget);
        return;
    }
}

void TriggerManager::ExecuteDeactivation(Trigger *mapping)
{
    if (mapping->actionType == TriggerActionType::LoadPanel)
    {
        HandlePanelDeactivation(mapping);
        return;
    }
    
    if (mapping->actionType == TriggerActionType::ToggleTheme)
    {
        SetTheme(mapping->restoreTarget);
        return;
    }
}

void TriggerManager::HandlePanelDeactivation(Trigger *mapping)
{
    // Find another active panel trigger
    Trigger *activePanel = FindActivePanel();
    if (activePanel)
    {
        log_i("Panel trigger deactivated but another active - loading panel: %s", activePanel->actionTarget);
        LoadPanel(activePanel->actionTarget);
        return;
    }
    
    // No other panel triggers active - restore to user panel
    const char *restorationPanel = panelService_ ? panelService_->GetRestorationPanel() : nullptr;
    log_i("No other panel triggers active - restoring to user panel: %s", restorationPanel);
    LoadPanel(restorationPanel);
}

void TriggerManager::LoadPanel(const char *panelName)
{
    if (!panelService_ || !panelName) return;
    
    log_i("Loading panel: %s", panelName);
    panelService_->CreateAndLoadPanel(panelName, true);
}

void TriggerManager::SetTheme(const char *themeName)
{
    if (!styleService_ || !themeName) return;
    
    log_i("Setting theme: %s", themeName);
    styleService_->SetTheme(themeName);
    
    if (panelService_)
    {
        panelService_->UpdatePanel();
    }
}

void TriggerManager::InitializeTriggersFromSensors()
{
    log_v("InitializeTriggersFromSensors() called");
    log_d("Initializing trigger states from current sensor states...");

    if (!ValidateSensors())
    {
        log_e("Cannot initialize triggers - missing sensors");
        return;
    }

    GpioState currentState = ReadAllSensorStates();
    InitializeAllTriggers(currentState);
    ApplyStartupActions();
}

bool TriggerManager::ValidateSensors()
{
    return keySensor_ && lockSensor_ && lightSensor_ && debugErrorSensor_;
}

void TriggerManager::InitializeAllTriggers(const GpioState& state)
{
    InitializeTrigger(TriggerIds::KEY_PRESENT, state.keyPresent);
    InitializeTrigger(TriggerIds::KEY_NOT_PRESENT, state.keyNotPresent);
    InitializeTrigger(TriggerIds::LOCK_STATE, state.lockState);
    InitializeTrigger(TriggerIds::LIGHTS_STATE, state.lightsState);
    InitializeTrigger(TriggerIds::ERROR_OCCURRED, ErrorManager::Instance().ShouldTriggerErrorPanel());
}

void TriggerManager::ApplyStartupActions()
{
    for (auto &trigger : triggers_)
    {
        if (trigger.currentState != TriggerExecutionState::ACTIVE) continue;
        
        if (trigger.actionType == TriggerActionType::ToggleTheme)
        {
            ApplyStartupTheme(trigger.actionTarget);
            continue;
        }
        
        if (trigger.actionType == TriggerActionType::LoadPanel)
        {
            ApplyStartupPanel(trigger.actionTarget);
            return; // Only use the first active panel trigger
        }
    }
}

void TriggerManager::ApplyStartupTheme(const char* themeName)
{
    if (!styleService_ || !themeName) return;
    
    log_i("Applying startup theme action: %s", themeName);
    styleService_->SetTheme(themeName);
}

void TriggerManager::ApplyStartupPanel(const char* panelName)
{
    if (!panelName) return;
    
    log_i("Startup panel action detected: Load %s", panelName);
    startupPanelOverride_ = panelName;
}

void TriggerManager::InitializeTrigger(const char *triggerId, bool currentPinState)
{
    log_v("InitializeTrigger() called");
    Trigger *mapping = FindTriggerMapping(triggerId);
    if (!mapping)
        return;

    TriggerExecutionState initialState =
        currentPinState ? TriggerExecutionState::ACTIVE : TriggerExecutionState::INACTIVE;
    mapping->currentState = initialState;

    // No complex trigger tracking needed - simplified approach

    log_v("Trigger %s initialized to %s based on GPIO state", triggerId, currentPinState ? "ACTIVE" : "INACTIVE");
}

Trigger *TriggerManager::FindTriggerMapping(const char *triggerId)
{
    log_v("FindTriggerMapping() called");
    for (auto &trigger : triggers_)
    {
        if (trigger.triggerId == triggerId)
        {
            return &trigger;
        }
    }
    return nullptr;
}

Trigger *TriggerManager::FindActivePanel()
{
    log_v("FindActivePanel() called");
    Trigger *activePanel = nullptr;
    for (auto &trigger : triggers_)
    {
        if (trigger.actionType == TriggerActionType::LoadPanel && trigger.currentState == TriggerExecutionState::ACTIVE)
        {
            // Find highest priority active panel trigger (lower priority value = higher priority)
            if (!activePanel || trigger.priority < activePanel->priority)
            {
                activePanel = &trigger;
            }
        }
    }
    return activePanel;
}

void TriggerManager::AddTrigger(const std::string &triggerName, ISensor *sensor, std::function<void()> callback)
{
    log_v("AddTrigger() called");
    log_d("Adding trigger %s", triggerName.c_str());
    // Currently a no-op since triggers are statically defined
    // This interface method is part of ITriggerService interface
}

bool TriggerManager::HasTrigger(const std::string &triggerName) const
{
    log_v("HasTrigger() called");
    log_d("Checking for trigger %s", triggerName.c_str());
    // Check if the trigger exists in our static mapping
    for (const auto &trigger : triggers_)
    {
        if (trigger.triggerId == triggerName)
        {
            return true;
        }
    }
    return false;
}

