#include "managers/trigger_manager.h"
#include "hardware/gpio_pins.h"
#include "managers/error_manager.h"
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
    // Check key presence on startup using sensor
    if (keySensor_->GetKeyState() == KeyState::Present)
    {
        return PanelNames::KEY;
    }
    return nullptr; // No override needed
}

void TriggerManager::Init()
{
    // Prevent double initialization
    if (initialized_)
    {
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
    // Temporarily remove #ifdef to ensure it's being called
    if (debugErrorSensor_)
    {
        static unsigned long debugCallCount = 0;
        debugCallCount++;
        if (debugCallCount % 5000 == 0)
        { // Log every 5000 calls to reduce noise
            log_d("TriggerManager: DebugErrorSensor called %lu times", debugCallCount);
        }
        debugErrorSensor_->GetReading(); // This will trigger errors on rising edge
    }
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
    CheckTriggerChange(TriggerIds::KEY_PRESENT, currentState.keyPresent);
    CheckTriggerChange(TriggerIds::KEY_NOT_PRESENT, currentState.keyNotPresent);
    CheckTriggerChange(TriggerIds::LOCK_STATE, currentState.lockState);
    CheckTriggerChange(TriggerIds::LIGHTS_STATE, currentState.lightsState);
}

void TriggerManager::CheckErrorTrigger()
{
    bool shouldShowErrorPanel = ErrorManager::Instance().ShouldTriggerErrorPanel();
    CheckTriggerChange(TriggerIds::ERROR_OCCURRED, shouldShowErrorPanel);
}

void TriggerManager::CheckTriggerChange(const char *triggerId, bool currentPinState)
{
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
    if (state == TriggerExecutionState::ACTIVE)
    {
        // Execute trigger action when activated - no complex state checking
        if (mapping->actionType == TriggerActionType::LoadPanel)
        {
            log_i("Executing panel action: Load %s", mapping->actionTarget);
            if (panelService_)
            {
                panelService_->CreateAndLoadPanel(
                    mapping->actionTarget,
                    []()
                    {
                        // Panel switch callback handled by service
                    },
                    true);
            }
        }
        else if (mapping->actionType == TriggerActionType::ToggleTheme)
        {
            log_i("Executing theme action: %s", mapping->actionTarget);
            if (styleService_)
            {
                styleService_->SetTheme(mapping->actionTarget);
                // Refresh current panel with new theme
                if (panelService_)
                {
                    panelService_->UpdatePanel();
                }
            }
        }
    }
    else
    {
        // Handle trigger deactivation - check for other active triggers first
        if (mapping->actionType == TriggerActionType::LoadPanel)
        {
            Trigger *activePanel = FindActivePanel();
            if (activePanel)
            {
                // Load panel from remaining active trigger
                log_i("Panel trigger deactivated but another active - loading panel: %s", activePanel->actionTarget);
                if (panelService_)
                {
                    panelService_->CreateAndLoadPanel(
                        activePanel->actionTarget,
                        []()
                        {
                            // Panel switch callback handled by service
                        },
                        true);
                }
            }
            else
            {
                // No other panel triggers active - restore to user panel
                const char *restorationPanel = panelService_->GetRestorationPanel();
                log_i("No other panel triggers active - restoring to user panel: %s", restorationPanel);
                if (panelService_)
                {
                    panelService_->CreateAndLoadPanel(
                        restorationPanel,
                        []()
                        {
                            // Panel switch callback handled by service
                        },
                        true); // Skip splash when restoring from triggers
                }
            }
        }
        else if (mapping->actionType == TriggerActionType::ToggleTheme)
        {
            log_i("Restoring theme: %s", mapping->restoreTarget);
            if (styleService_)
            {
                styleService_->SetTheme(mapping->restoreTarget);
                // Refresh current panel with restored theme
                if (panelService_)
                {
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
    InitializeTrigger(TriggerIds::KEY_PRESENT, currentState.keyPresent);
    InitializeTrigger(TriggerIds::KEY_NOT_PRESENT, currentState.keyNotPresent);
    InitializeTrigger(TriggerIds::LOCK_STATE, currentState.lockState);
    InitializeTrigger(TriggerIds::LIGHTS_STATE, currentState.lightsState);

    // Initialize error trigger based on current error state
    InitializeTrigger(TriggerIds::ERROR_OCCURRED, ErrorManager::Instance().ShouldTriggerErrorPanel());

    // Check for active triggers at startup and apply their actions
    for (auto &trigger : triggers_)
    {
        if (trigger.currentState == TriggerExecutionState::ACTIVE)
        {
            if (trigger.actionType == TriggerActionType::ToggleTheme)
            {
                log_i("Applying startup theme action: %s", trigger.actionTarget);
                if (styleService_)
                {
                    styleService_->SetTheme(trigger.actionTarget);
                }
            }
            else if (trigger.actionType == TriggerActionType::LoadPanel)
            {
                log_i("Startup panel action detected: Load %s", trigger.actionTarget);
                startupPanelOverride_ = trigger.actionTarget;
                break; // Only use the first active panel trigger
            }
        }
    }
}

void TriggerManager::InitializeTrigger(const char *triggerId, bool currentPinState)
{
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
    log_d("Adding trigger %s", triggerName.c_str());
    // Currently a no-op since triggers are statically defined
    // This interface method is part of ITriggerService interface
}

bool TriggerManager::HasTrigger(const std::string &triggerName) const
{
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

