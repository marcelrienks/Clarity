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
        StyleManager::GetInstance().set_theme(triggerState.target.c_str());
        log_i("Theme changed to %s", triggerState.target.c_str());
        // Theme triggers can be cleared immediately as they don't correspond to GPIO pins
        TriggerManager::GetInstance().ClearTriggerState(triggerId);
    }
}

/// @brief Comprehensive trigger evaluation - processes all triggers with proper prioritization
void PanelManager::ProcessTriggers()
{
    log_d("...");

    // Only process triggers when not already loading/updating a panel
    if (uiState_ != UIState::IDLE)
    {
        return;
    }

    // Clean up any inactive triggers first
    TriggerManager::GetInstance().CleanupInactiveTriggers();
    
    // Get the highest priority active trigger
    auto triggerPair = TriggerManager::GetInstance().GetHighestPriorityTrigger();
    const char* triggerId = triggerPair.first;
    TriggerState* trigger = triggerPair.second;
    
    if (trigger && trigger->active && triggerId)
    {
        // Check if we need to switch panels based on the highest priority trigger
        const char* targetPanel = trigger->target.c_str();
        
        if (trigger->action == ACTION_LOAD_PANEL)
        {
            // Only switch panels if we're not already showing the target panel
            if (strcmp(currentPanel, targetPanel) != 0)
            {
                log_d("Highest priority trigger %s requires panel switch to %s", triggerId, targetPanel);
                ExecuteTriggerAction(*trigger, triggerId);
            }
            else
            {
                // Already showing the correct panel - reset processing flag and keep showing it
                log_d("Already showing panel %s for trigger %s - trigger satisfied", targetPanel, triggerId);
                trigger->processing = false;
            }
        }
        else
        {
            // Handle other actions (restore, theme change, etc.)
            ExecuteTriggerAction(*trigger, triggerId);
        }
    }
    else
    {
        // No active triggers - check if we need to restore to previous panel
        if (!TriggerManager::GetInstance().HasActiveTriggers())
        {
            // Check if we're currently showing a trigger-driven panel that should be restored
            if (strcmp(currentPanel, PanelNames::KEY) == 0 || strcmp(currentPanel, PanelNames::LOCK) == 0)
            {
                log_d("No active triggers - restoring from %s to %s", currentPanel, restorationPanel);
                CreateAndLoadPanel(restorationPanel, [this](){ this->PanelCompletionCallback(); }, false);
            }
            else
            {
                log_v("No active triggers but showing non-trigger panel %s", currentPanel);
            }
        }
        else
        {
            log_v("No processable triggers (may be debouncing or processing)");
        }
    }
}

/// @brief Process only critical and important priority triggers during updates
void PanelManager::ProcessCriticalAndImportantTriggers()
{
    log_d("...");

    // Only process triggers when updating a panel
    if (uiState_ != UIState::UPDATING)
    {
        return;
    }

    // Clean up any inactive triggers first
    TriggerManager::GetInstance().CleanupInactiveTriggers();
    
    auto triggerPair = TriggerManager::GetInstance().GetHighestPriorityTrigger();
    const char* triggerId = triggerPair.first;
    TriggerState* trigger = triggerPair.second;
    
    if (trigger && trigger->active && triggerId &&
        (trigger->priority == TriggerPriority::CRITICAL || trigger->priority == TriggerPriority::IMPORTANT))
    {
        const char* targetPanel = trigger->target.c_str();
        
        if (trigger->action == ACTION_LOAD_PANEL)
        {
            // Only interrupt updates for critical/important panel switches
            if (strcmp(currentPanel, targetPanel) != 0)
            {
                log_d("Critical/Important trigger %s interrupting update to switch to %s", triggerId, targetPanel);
                ExecuteTriggerAction(*trigger, triggerId);
            }
            else
            {
                // Already showing the correct panel
                TriggerManager::GetInstance().ClearTriggerState(triggerId);
                ProcessCriticalAndImportantTriggers(); // Re-evaluate
            }
        }
        else
        {
            ExecuteTriggerAction(*trigger, triggerId);
        }
    }
}

