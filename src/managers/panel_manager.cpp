#include "managers/panel_manager.h"
#include "managers/trigger_manager.h"
#include "triggers/key_trigger.h"
#include "triggers/lock_trigger.h"
#include "utilities/trigger_messages.h"
#include <esp32-hal-log.h>

// Static Methods

/// @brief Get the singleton instance of PanelManager
/// @return instance of PanelManager
PanelManager &PanelManager::get_instance()
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

    PanelManager::register_panels();

    // Initialize dual-core trigger system
    TriggerManager &trigger_manager = TriggerManager::get_instance();
    trigger_manager.init();

    log_d("PanelManager initialized for dual-core operation");
}

/// @brief Creates and then loads a panel based on the given name
/// @param panel_name the name of the panel to be loaded
/// @param completion_callback the function to be called when the panel load is complete
/// @param is_trigger_driven whether this panel change is triggered by an interrupt trigger
void PanelManager::create_and_load_panel(const char *panel_name, std::function<void()> completion_callback, bool is_trigger_driven)
{
    log_d("...");

    // Track this as the last non-trigger panel only for user-driven changes
    if (!is_trigger_driven)
    {
        _last_non_trigger_panel = std::string(panel_name);
    }

    // Clean up existing panel before creating new one
    if (_panel)
    {
        log_d("Cleaning up existing panel before creating new one");
        _panel.reset();
    }

    _panel = PanelManager::create_panel(panel_name);
    _panel->init();

    _is_loading = true;
    _panel->load(completion_callback);
    Ticker::handle_lv_tasks();
}

/// @brief Loads a panel based on the given name after first loading a splash screen
/// This function will create the panel and then call the load function on it.
/// @param panel_name the name of the panel to be loaded
void PanelManager::create_and_load_panel_with_splash(const char *panel_name)
{
    log_d("...");

    create_and_load_panel(PanelNames::Splash, [this, panel_name]()
                          { this->PanelManager::splash_completion_callback(panel_name); });
}

/// @brief Update the reading on the currently loaded panel and process trigger messages
void PanelManager::update_panel()
{
    log_d("...");

    // Process trigger states from Core 1 based on current UI state
    process_trigger_states();
    set_ui_state(UIState::UPDATING);
    _panel->update([this]()
                   {
                       this->PanelManager::panel_completion_callback();
                   });

    Ticker::handle_lv_tasks();
    set_ui_state(UIState::IDLE);
    process_trigger_states(); // Process any triggers that were set during the UPDATING phase
}

// Constructors and Destructors

PanelManager::~PanelManager()
{
    _panel.reset();
}

// Private Methods

/// @brief Create a panel based on the given type name
/// @param panel_name the type name of the panel to be created
/// @return Interface type representing the panel
std::shared_ptr<IPanel> PanelManager::create_panel(const char *panel_name)
{
    log_d("...");

    auto iterator = _registered_panels.find(panel_name);
    if (iterator == _registered_panels.end())
    {
        log_e("Failed to find panel %s in map", panel_name);
        return nullptr;
    }

    return iterator->second(); // Return the function stored in the map
}

/// @brief Register all available panel types with the factory
void PanelManager::register_panels()
{
    log_d("...");

    // Register all available panel types with the factory
    register_panel<SplashPanel>(PanelNames::Splash);
    register_panel<OemOilPanel>(PanelNames::Oil);
    register_panel<KeyPanel>(PanelNames::Key);
    register_panel<LockPanel>(PanelNames::Lock);
}

// Callback Methods

/// @brief callback function to be executed on splash panel show completion
void PanelManager::splash_completion_callback(const char *panel_name)
{
    log_d("...");

    _panel.reset();
    Ticker::handle_lv_tasks();

    create_and_load_panel(panel_name, [this]()
                          { this->PanelManager::panel_completion_callback(); });
}

/// @brief callback function to be executed on panel show completion
void PanelManager::panel_completion_callback()
{
    log_d("...");

    _is_loading = false;
    TriggerManager::get_instance().update_application_state(_current_panel_name.c_str(), _current_theme_name.c_str());
}

/// @brief callback function to be executed when trigger-driven panel loading is complete
void PanelManager::trigger_panel_switch_callback()
{
    _is_loading = false;
    log_d("Trigger panel load completed, _is_loading is now %i", _is_loading);
    TriggerManager::get_instance().update_application_state(_current_panel_name.c_str(), _current_theme_name.c_str());
}

/// @brief Get the panel name to restore when all triggers are inactive
/// @return Panel name for restoration, or nullptr if none set
const char *PanelManager::get_restoration_panel() const
{
    return _last_non_trigger_panel.c_str();
}

// Core 0 Dual-Core Methods

/// @brief Process trigger states from Core 1 based on UI state
void PanelManager::process_trigger_states()
{
    switch (_ui_state)
    {
    case UIState::IDLE:
        // No throttling - process all triggers
        process_triggers();
        break;

    case UIState::UPDATING:
        // State-based throttling - only high/medium priority triggers
        process_critical_and_important_triggers();
        break;

    case UIState::LOADING:
    case UIState::LVGL_BUSY:
        // No action - don't process any triggers
        // Triggers remain in shared state for later processing
        break;
    }
}

/// @brief Set current UI state for Core 1 synchronization
void PanelManager::set_ui_state(UIState state)
{
    _ui_state = state;
    log_d("UI State changed to: %d", (int)state);
}

/// @brief Execute a trigger action from shared state
void PanelManager::execute_trigger_action(const TriggerState &trigger_state, const std::string &trigger_id)
{
    log_d("...");

    if (trigger_state.action == ACTION_LOAD_PANEL)
    {
        _current_panel_name = trigger_state.target;
        create_and_load_panel(trigger_state.target.c_str(), [this, trigger_id]()
                              { 
                                  this->trigger_panel_switch_callback();
                                  // Clear trigger after successful execution
                                  TriggerManager::get_instance().clear_trigger_state_public(trigger_id.c_str()); }, true);
    }
    else if (trigger_state.action == ACTION_RESTORE_PREVIOUS_PANEL)
    {
        const char *restore_panel = get_restoration_panel();
        if (restore_panel)
        {
            _current_panel_name = std::string(restore_panel);
            create_and_load_panel(restore_panel, [this, trigger_id]()
                                  { 
                                      this->trigger_panel_switch_callback();
                                      // Clear trigger after successful execution
                                      TriggerManager::get_instance().clear_trigger_state_public(trigger_id.c_str()); }, false);
        }
    }
    else if (trigger_state.action == ACTION_CHANGE_THEME)
    {
        _current_theme_name = trigger_state.target;
        // TODO: Implement theme change logic
        log_i("Theme change to %s (implementation needed)", trigger_state.target.c_str());
        TriggerManager::get_instance().update_application_state(_current_panel_name.c_str(), _current_theme_name.c_str());
        // Clear trigger after successful execution
        TriggerManager::get_instance().clear_trigger_state_public(trigger_id.c_str());
    }
}

/// @brief Process all triggers regardless of priority
void PanelManager::process_triggers()
{
    TriggerManager &trigger_manager = TriggerManager::get_instance();
    TriggerState *trigger = trigger_manager.get_highest_priority_trigger();

    if (trigger && trigger->active)
    {
        // Find the trigger ID for this state
        std::string trigger_id = find_trigger_id_for_state(*trigger);
        if (!trigger_id.empty())
        {
            execute_trigger_action(*trigger, trigger_id);
        }
    }
}

/// @brief Process only critical and important priority triggers
void PanelManager::process_critical_and_important_triggers()
{
    TriggerManager &trigger_manager = TriggerManager::get_instance();
    TriggerState *trigger = trigger_manager.get_highest_priority_trigger();

    if (trigger && trigger->active &&
        (trigger->priority == TriggerPriority::CRITICAL || trigger->priority == TriggerPriority::IMPORTANT))
    {
        // Find the trigger ID for this state
        std::string trigger_id = find_trigger_id_for_state(*trigger);
        if (!trigger_id.empty())
        {
            execute_trigger_action(*trigger, trigger_id);
        }
    }
}

/// @brief Find trigger ID for a given trigger state (helper method)
std::string PanelManager::find_trigger_id_for_state(const TriggerState &target_state)
{
    // This is a helper to find the key (trigger_id) for a given trigger state
    // We need to access the trigger manager's map, but for now we'll use known trigger IDs

    // Check common trigger IDs
    if (target_state.action == ACTION_LOAD_PANEL && target_state.target == PanelNames::Key)
    {
        return std::string(TRIGGER_KEY_PRESENT); // or TRIGGER_KEY_NOT_PRESENT
    }
    else if (target_state.action == ACTION_LOAD_PANEL && target_state.target == PanelNames::Lock)
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