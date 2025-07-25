#include "managers/panel_manager.h"
#include "triggers/key_trigger.h"
#include "triggers/lock_trigger.h"
#include "triggers/lights_trigger.h"

// Static Methods

/// @brief Get the singleton instance of PanelManager
/// @return instance of PanelManager
PanelManager &PanelManager::GetInstance()
{
    static PanelManager instance; // this ensures that the instance is created only once
    return instance;
}

// Core Functionality Methods

/// @brief Initialise the panel manager to control the flow and rendering of all panels
/// Registers all available panel types with the factory for dynamic creation
/// Also initializes dual-core trigger system
void PanelManager::init()
{
    log_d("...");
    Ticker::handle_lv_tasks();

    PanelManager::RegisterPanels();

    // Initialize dual-core trigger system
    TriggerManager &triggerManager = TriggerManager::GetInstance();
    triggerManager.init();
    
    // Register concrete trigger implementations
    RegisterTriggers();
}

/// @brief Creates and then loads a panel based on the given name
/// @param panelName the name of the panel to be loaded
/// @param completionCallback the function to be called when the panel load is complete
/// @param isTriggerDriven whether this panel change is triggered by an interrupt trigger
void PanelManager::CreateAndLoadPanel(const char *panelName, std::function<void()> completionCallback, bool isTriggerDriven)
{
    log_d("...");

    // Track this as the last non-trigger panel only for user-driven changes
    if (!isTriggerDriven)
    {
        restorationPanel = panelName;
    }

    // Clean up existing panel before creating new one
    if (panel_)
    {
        log_d("Cleaning up existing panel before creating new one");
        panel_.reset();
    }

    panel_ = PanelManager::CreatePanel(panelName);
    panel_->init();

    // Make a copy of the panel name to avoid pointer issues
    strncpy(currentPanelBuffer, panelName, sizeof(currentPanelBuffer) - 1);
    currentPanelBuffer[sizeof(currentPanelBuffer) - 1] = '\0';
    currentPanel = currentPanelBuffer;

    SetUiState(UIState::LOADING);
    panel_->load(completionCallback);
    Ticker::handle_lv_tasks();
}

/// @brief Loads a panel based on the given name after first loading a splash screen
/// This function will create the panel and then call the load function on it.
/// @param panelName the name of the panel to be loaded
void PanelManager::CreateAndLoadPanelWithSplash(const char *panelName)
{
    log_d("...");

    CreateAndLoadPanel(PanelNames::SPLASH, [this, panelName]()
                       { this->PanelManager::SplashCompletionCallback(panelName); });
}

/// @brief Update the reading on the currently loaded panel and process trigger messages
void PanelManager::UpdatePanel()
{
    log_d("...");

    ProcessTriggerStates();
    SetUiState(UIState::UPDATING);
    panel_->update([this]()
                   { this->PanelManager::PanelCompletionCallback(); });

    Ticker::handle_lv_tasks();
}

// Constructors and Destructors

PanelManager::PanelManager()
{
    // Initialize the current panel buffer with the default value
    strncpy(currentPanelBuffer, PanelNames::OIL, sizeof(currentPanelBuffer) - 1);
    currentPanelBuffer[sizeof(currentPanelBuffer) - 1] = '\0';
    currentPanel = currentPanelBuffer;
}

PanelManager::~PanelManager()
{
    panel_.reset();
}

// Private Methods

/// @brief Create a panel based on the given type name
/// @param panelName the type name of the panel to be created
/// @return Interface type representing the panel
std::shared_ptr<IPanel> PanelManager::CreatePanel(const char *panelName)
{
    log_d("...");

    auto iterator = registeredPanels_.find(panelName);
    return iterator->second(); // Return the function stored in the map
}

/// @brief Register all available panel types with the factory
void PanelManager::RegisterPanels()
{
    log_d("...");

    // Register all available panel types with the factory
    register_panel<SplashPanel>(PanelNames::SPLASH);
    register_panel<OemOilPanel>(PanelNames::OIL);
    register_panel<KeyPanel>(PanelNames::KEY);
    register_panel<LockPanel>(PanelNames::LOCK);
}

/// @brief Register all available triggers with the trigger manager
void PanelManager::RegisterTriggers()
{
    log_d("Registering triggers...");

    TriggerManager& triggerManager = TriggerManager::GetInstance();
    
    // Register concrete trigger implementations
    auto keyTrigger = std::make_unique<KeyTrigger>();
    keyTrigger->init();
    triggerManager.RegisterTrigger(std::move(keyTrigger));
    
    auto lockTrigger = std::make_unique<LockTrigger>();
    lockTrigger->init();
    triggerManager.RegisterTrigger(std::move(lockTrigger));
    
    auto lightsTrigger = std::make_unique<LightsTrigger>();
    lightsTrigger->init();
    triggerManager.RegisterTrigger(std::move(lightsTrigger));
    
    log_d("Triggers registered successfully");
}

// Callback Methods

/// @brief callback function to be executed on splash panel show completion
void PanelManager::SplashCompletionCallback(const char *panelName)
{
    log_d("...");

    panel_.reset();
    Ticker::handle_lv_tasks();

    ProcessTriggerStates();

    CreateAndLoadPanel(panelName, [this]()
                       { this->PanelManager::PanelCompletionCallback(); });
}

/// @brief callback function to be executed on panel show completion
void PanelManager::PanelCompletionCallback()
{
    log_d("Panel operation complete - evaluating all triggers");

    SetUiState(UIState::IDLE);
    
    // System initialization complete - triggers will remain in INIT state
    // until actual GPIO pin changes occur
    static bool systemInitialized = false;
    if (!systemInitialized)
    {
        systemInitialized = true;
        log_d("System initialization complete - triggers remain in INIT state until GPIO changes");
    }
    
    // Always re-evaluate all triggers after panel operations complete
    // This ensures continuous evaluation of concurrent triggers
    ProcessTriggerStates();
}

/// @brief callback function to be executed when trigger-driven panel loading is complete
void PanelManager::TriggerPanelSwitchCallback(const char *triggerId)
{
    log_d("Trigger panel switch complete for: %s", triggerId);
    
    SetUiState(UIState::IDLE);
    // No need to clear triggers - GPIO state manages trigger active/inactive status
    
    // Re-evaluate all triggers to handle any concurrent triggers
    log_d("Re-evaluating all triggers after panel switch");
    ProcessTriggerStates();
}

/// @brief Process trigger states from Core 1 based on UI state
void PanelManager::ProcessTriggerStates()
{
    switch (uiState_)
    {
    case UIState::IDLE: // No throttling - process all triggers
        ProcessTriggers();
        break;

    case UIState::UPDATING: // State-based throttling - only high/medium priority triggers
        ProcessCriticalAndImportantTriggers();
        break;

    case UIState::LOADING:
    case UIState::LVGL_BUSY: // No action - don't process any triggers
        // Triggers remain in shared state for later processing
        break;
    }
}

/// @brief Set current UI state for Core 1 synchronization
void PanelManager::SetUiState(UIState state)
{
    uiState_ = state;
    log_d("UI State changed to: %d", (int)state);
}

/// @brief Execute a trigger action from shared state
void PanelManager::ExecuteTriggerAction(const TriggerState &triggerState, const char *triggerId)
{
    log_d("Executing trigger action: %s for trigger: %s", triggerState.action.c_str(), triggerId);

    if (triggerState.action == ACTION_LOAD_PANEL)
    {
        CreateAndLoadPanel(triggerState.target.c_str(), [this, triggerId]()
                           { this->TriggerPanelSwitchCallback(triggerId); }, true);
    }
    else if (triggerState.action == ACTION_CHANGE_THEME)
    {
        // Only change theme if it's actually different to prevent infinite loops
        const char* currentTheme = StyleManager::GetInstance().THEME;
        const char* targetTheme = triggerState.target.c_str();
        
        if (strcmp(currentTheme, targetTheme) != 0)
        {
            StyleManager::GetInstance().set_theme(targetTheme);
            log_i("Theme changed from %s to %s", currentTheme, targetTheme);
            
            // Update current panel to refresh components with new theme colors
            if (panel_)
            {
                log_d("Refreshing panel components with new theme");
                panel_->update([this]()
                              { this->PanelCompletionCallback(); });
            }
        }
        else
        {
            log_d("Theme already set to %s, skipping change", targetTheme);
        }
        // GPIO system will handle trigger state clearing based on hardware state
    }
}

/// @brief Simplified trigger processing - let TriggerManager handle all logic
void PanelManager::ProcessTriggers()
{
    log_d("Processing triggers...");

    // Only process triggers when not already loading/updating a panel
    if (uiState_ != UIState::IDLE)
    {
        return;
    }

    // Delegate to TriggerManager for simplified priority-based evaluation
    TriggerManager::GetInstance().EvaluateAndExecuteTriggers();
}

/// @brief Process only critical and important priority triggers during updates
void PanelManager::ProcessCriticalAndImportantTriggers()
{
    log_d("Processing critical/important triggers during update...");

    // Only process triggers when updating a panel
    if (uiState_ != UIState::UPDATING)
    {
        return;
    }

    // Process all triggers - the priority system will handle conflicts
    TriggerManager::GetInstance().EvaluateAndExecuteTriggers();
}

