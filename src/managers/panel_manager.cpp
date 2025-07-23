#include "managers/panel_manager.h"
#include "managers/trigger_manager.h"
#include "managers/style_manager.h"
#include "triggers/key_trigger.h"
#include "triggers/lock_trigger.h"
#include "utilities/trigger_messages.h"
#include <esp32-hal-log.h>

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
    TriggerManager &trigger_manager = TriggerManager::GetInstance();
    trigger_manager.init();

    log_d("PanelManager initialized for dual-core operation");
}

/// @brief Creates and then loads a panel based on the given name
/// @param panel_name the name of the panel to be loaded
/// @param completion_callback the function to be called when the panel load is complete
/// @param is_trigger_driven whether this panel change is triggered by an interrupt trigger
void PanelManager::CreateAndLoadPanel(const char *panel_name, std::function<void()> completion_callback, bool is_trigger_driven)
{
    log_d("...");

    // Track this as the last non-trigger panel only for user-driven changes
    if (!is_trigger_driven)
    {
        restoration_panel = std::string(panel_name);
    }

    // Clean up existing panel before creating new one
    if (panel_)
    {
        log_d("Cleaning up existing panel before creating new one");
        panel_.reset();
    }

    panel_ = PanelManager::CreatePanel(panel_name);
    panel_->init();
    
    // Update current panel
    current_panel = panel_name;

    SetUiState(UIState::LOADING);
    panel_->load(completion_callback);
    Ticker::handle_lv_tasks();
}

/// @brief Loads a panel based on the given name after first loading a splash screen
/// This function will create the panel and then call the load function on it.
/// @param panel_name the name of the panel to be loaded
void PanelManager::CreateAndLoadPanelWithSplash(const char *panel_name)
{
    log_d("...");

    CreateAndLoadPanel(PanelNames::SPLASH, [this, panel_name]()
                          { this->PanelManager::SplashCompletionCallback(panel_name); });
}

/// @brief Update the reading on the currently loaded panel and process trigger messages
void PanelManager::UpdatePanel()
{
    log_d("...");

    // Process trigger states from Core 1 based on current UI state
    ProcessTriggerStates();
    SetUiState(UIState::UPDATING);
    panel_->update([this]()
                   { this->PanelManager::PanelCompletionCallback(); });

    Ticker::handle_lv_tasks();
    SetUiState(UIState::IDLE);
    ProcessTriggerStates(); // Process any triggers that were set during the UPDATING phase
}

// Constructors and Destructors

PanelManager::~PanelManager()
{
    panel_.reset();
}

// Private Methods

/// @brief Create a panel based on the given type name
/// @param panel_name the type name of the panel to be created
/// @return Interface type representing the panel
std::shared_ptr<IPanel> PanelManager::CreatePanel(const char *panel_name)
{
    log_d("...");

    auto iterator = registeredPanels_.find(panel_name);
    if (iterator == registeredPanels_.end())
    {
        log_e("Failed to find panel %s in map", panel_name);
        return nullptr;
    }

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
void PanelManager::SplashCompletionCallback(const char *panel_name)
{
    log_d("...");

    panel_.reset();
    Ticker::handle_lv_tasks();

    CreateAndLoadPanel(panel_name, [this]()
                          { this->PanelManager::PanelCompletionCallback(); });
}

/// @brief callback function to be executed on panel show completion
void PanelManager::PanelCompletionCallback()
{
    log_d("...");

    SetUiState(UIState::IDLE);
    TriggerManager::GetInstance().NotifyApplicationStateUpdated();
}

/// @brief callback function to be executed when trigger-driven panel loading is complete
void PanelManager::TriggerPanelSwitchCallback()
{
    SetUiState(UIState::IDLE);
    log_d("Trigger panel load completed, UI state set to IDLE");
    TriggerManager::GetInstance().NotifyApplicationStateUpdated();
}

/// @brief Get the panel name to restore when all triggers are inactive
/// @return Panel name for restoration, or nullptr if none set

// Core 0 Dual-Core Methods

/// @brief Process trigger states from Core 1 based on UI state
void PanelManager::ProcessTriggerStates()
{
    switch (uiState_)
    {
    case UIState::IDLE:
        // No throttling - process all triggers
        ProcessTriggers();
        break;

    case UIState::UPDATING:
        // State-based throttling - only high/medium priority triggers
        ProcessCriticalAndImportantTriggers();
        break;

    case UIState::LOADING:
    case UIState::LVGL_BUSY:
        // No action - don't process any triggers
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
void PanelManager::ExecuteTriggerAction(const TriggerState &trigger_state, const std::string &trigger_id)
{
    log_d("...");

    if (trigger_state.action == ACTION_LOAD_PANEL)
    {
        CreateAndLoadPanel(trigger_state.target.c_str(), [this, trigger_id]()
                              { 
                                  this->TriggerPanelSwitchCallback();
                                  // Clear trigger after successful execution
                                  TriggerManager::GetInstance().ClearTriggerStatePublic(trigger_id.c_str()); }, true);
    }
    else if (trigger_state.action == ACTION_RESTORE_PREVIOUS_PANEL)
    {
        const char *restore_panel = restoration_panel.c_str();
        if (restore_panel)
        {
            CreateAndLoadPanel(restore_panel, [this, trigger_id]()
                                  { 
                                      this->TriggerPanelSwitchCallback();
                                      // Clear trigger after successful execution
                                      TriggerManager::GetInstance().ClearTriggerStatePublic(trigger_id.c_str()); }, false);
        }
    }
    else if (trigger_state.action == ACTION_CHANGE_THEME)
    {
        StyleManager::GetInstance().set_theme(trigger_state.target.c_str());
        log_i("Theme changed to %s", trigger_state.target.c_str());
        TriggerManager::GetInstance().NotifyApplicationStateUpdated();
        // Clear trigger after successful execution
        TriggerManager::GetInstance().ClearTriggerStatePublic(trigger_id.c_str());
    }
}

/// @brief Process all triggers regardless of priority
void PanelManager::ProcessTriggers()
{
    TriggerManager &trigger_manager = TriggerManager::GetInstance();
    TriggerState *trigger = trigger_manager.GetHighestPriorityTrigger();

    if (trigger && trigger->active)
    {
        // Find the trigger ID for this state
        std::string trigger_id = FindTriggerIdForState(*trigger);
        if (!trigger_id.empty())
        {
            ExecuteTriggerAction(*trigger, trigger_id);
        }
    }
}

/// @brief Process only critical and important priority triggers
void PanelManager::ProcessCriticalAndImportantTriggers()
{
    TriggerManager &trigger_manager = TriggerManager::GetInstance();
    TriggerState *trigger = trigger_manager.GetHighestPriorityTrigger();

    if (trigger && trigger->active &&
        (trigger->priority == TriggerPriority::CRITICAL || trigger->priority == TriggerPriority::IMPORTANT))
    {
        // Find the trigger ID for this state
        std::string trigger_id = FindTriggerIdForState(*trigger);
        if (!trigger_id.empty())
        {
            ExecuteTriggerAction(*trigger, trigger_id);
        }
    }
}

/// @brief Find trigger ID for a given trigger state (helper method)
std::string PanelManager::FindTriggerIdForState(const TriggerState &target_state)
{
    // This is a helper to find the key (trigger_id) for a given trigger state
    // We need to access the trigger manager's map, but for now we'll use known trigger IDs

    // Check common trigger IDs
    if (target_state.action == ACTION_LOAD_PANEL && target_state.target == PanelNames::KEY)
    {
        return std::string(TRIGGER_KEY_PRESENT); // or TRIGGER_KEY_NOT_PRESENT
    }
    else if (target_state.action == ACTION_LOAD_PANEL && target_state.target == PanelNames::LOCK)
    {
        return std::string(TRIGGER_LOCK_STATE);
    }
    else if (target_state.action == ACTION_CHANGE_THEME)
    {
        return std::string(TRIGGER_THEME_SWITCH);
    }
    else if (target_state.action == ACTION_RESTORE_PREVIOUS_PANEL)
    {
        // Could be any of the restore triggers
        return std::string(TRIGGER_KEY_PRESENT); // Default fallback
    }

    return ""; // Not found
}