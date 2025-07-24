#include "managers/panel_manager.h"

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

    currentPanel = panelName;

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
    log_d("...");

    SetUiState(UIState::IDLE);
    ProcessTriggerStates();
}

/// @brief callback function to be executed when trigger-driven panel loading is complete
void PanelManager::TriggerPanelSwitchCallback(const char *triggerId)
{
    log_d("...");
    
    SetUiState(UIState::IDLE);
    TriggerManager::GetInstance().ClearTriggerState(triggerId);
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
    log_d("...");

    if (triggerState.action == ACTION_LOAD_PANEL)
    {
        CreateAndLoadPanel(triggerState.target.c_str(), [this, triggerId]()
                           { this->TriggerPanelSwitchCallback(triggerId); }, true);
    }
    else if (triggerState.action == ACTION_RESTORE_PREVIOUS_PANEL)
    {
        CreateAndLoadPanel(restorationPanel, [this, triggerId]()
                           { this->TriggerPanelSwitchCallback(triggerId); }, false);
    }
    else if (triggerState.action == ACTION_CHANGE_THEME)
    {
        StyleManager::GetInstance().set_theme(triggerState.target.c_str());
        log_i("Theme changed to %s", triggerState.target.c_str());
        TriggerManager::GetInstance().ClearTriggerState(triggerId);
    }
}

/// @brief Process all triggers regardless of priority  
void PanelManager::ProcessTriggers()
{
    log_d("...");

    // Only process triggers when not already loading/updating a panel
    if (uiState_ != UIState::IDLE)
    {
        return;
    }

    auto triggerPair = TriggerManager::GetInstance().GetHighestPriorityTrigger();
    const char* triggerId = triggerPair.first;
    TriggerState* trigger = triggerPair.second;
    
    if (trigger && trigger->active && triggerId)
    {
        ExecuteTriggerAction(*trigger, triggerId);
    }
}

/// @brief Process only critical and important priority triggers
void PanelManager::ProcessCriticalAndImportantTriggers()
{
    log_d("...");

    // Only process triggers when not already loading/updating a panel
    if (uiState_ != UIState::UPDATING)
    {
        return;
    }

    auto triggerPair = TriggerManager::GetInstance().GetHighestPriorityTrigger();
    const char* triggerId = triggerPair.first;
    TriggerState* trigger = triggerPair.second;
    
    if (trigger && trigger->active && triggerId &&
        (trigger->priority == TriggerPriority::CRITICAL || trigger->priority == TriggerPriority::IMPORTANT))
    {
        ExecuteTriggerAction(*trigger, triggerId);
    }
}

