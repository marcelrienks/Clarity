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
    trigger_manager.init_dual_core_system();
    
    // Get queue handles from TriggerManager
    get_trigger_queues();
    
    log_d("PanelManager initialized for dual-core operation");
}

/// @brief Creates and then loads a panel based on the given type name
/// @param panel_name the type name of the panel to be loaded
/// @param completion_callback the function to be called when the panel load is complete
/// @param is_trigger_driven whether this panel change is triggered by an interrupt trigger
void PanelManager::create_and_load_panel(const char *panel_name, std::function<void()> completion_callback, bool is_trigger_driven)
{
    log_d("...");

    // Track this as the last non-trigger panel only for user-driven changes
    if (!is_trigger_driven) {
        _last_non_trigger_panel = std::string(panel_name);
        log_d("Setting restoration panel to: %s", _last_non_trigger_panel.c_str());
    } else {
        log_d("Trigger-driven panel change, preserving restoration panel: %s", _last_non_trigger_panel.c_str());
    }

    // Check for trigger activations before creating panel (e.g., key already present at startup)
    // Dual-core trigger system handles this automatically

    // No need for manual trigger state management in dual-core architecture
    
    // Clean up existing panel before creating new one
    if (_panel) {
        log_d("Cleaning up existing panel before creating new one");
        _panel.reset();
    }
    
    _panel = PanelManager::create_panel(panel_name);
    
    log_i("Loading %s", _panel->get_name());

    // Lock the panel to prevent updating during loading
    _is_loading = true;
    log_v("_is_loading is now %i", _is_loading);

    // Initialize the panel
    _panel->init();
    // Skip LVGL task handling after init during panel switching to prevent hang
    // Ticker::handle_lv_tasks();

    // Use provided callback or default to panel_completion_callback
    auto callback = completion_callback ? completion_callback : [this]() { this->PanelManager::panel_completion_callback(); };
    _panel->load(callback);
    Ticker::handle_lv_tasks();
}

/// @brief Loads a panel based on the given type name after first loading a splash screen
/// This function will create the panel and then call the load function on it.
/// @param panel_name the type name of the panel to be loaded
void PanelManager::create_and_load_panel_with_splash(const char *panel_name)
{
    log_d("...");

    create_and_load_panel(PanelNames::Splash, [this, panel_name]()
                         { this->PanelManager::splash_completion_callback(panel_name); });
}

/// @brief Update the reading on the currently loaded panel and process trigger messages
void PanelManager::update_panel()
{
    log_d("Core 0 panel update cycle...");

    // Process trigger messages from Core 1 based on current UI state
    process_trigger_messages();

    if (!_panel)
        return;

    // Set UI state before panel update
    set_ui_state(UIState::UPDATING);
    
    _is_loading = true;
    log_v("_is_loading is now %i", _is_loading);

    _panel->update([this]()
                   { this->PanelManager::panel_completion_callback(); });

    Ticker::handle_lv_tasks();
    
    // Return to idle state after update
    set_ui_state(UIState::IDLE);
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

/// @brief Register all global triggers with the trigger manager
void PanelManager::register_triggers()
{
    log_d("Registering triggers for dual-core system...");
    
    // Triggers are now handled by Core 1 TriggerManager
    // No need to register old trigger system
    log_d("Trigger registration completed for dual-core system");
}

// Callback Methods

/// @brief callback function to be executed on splash panel show completion
void PanelManager::splash_completion_callback(const char *panel_name)
{
    log_d("...");

    _panel.reset();
    Ticker::handle_lv_tasks();

    create_and_load_panel(panel_name);
}

/// @brief callback function to be executed on panel show completion
void PanelManager::panel_completion_callback()
{
    log_d("Panel load completed...");

    _is_loading = false;
    log_d("Panel load completed, _is_loading is now %i", _is_loading);
    
    // Notify Core 1 of state change
    notify_core1_state_change(_current_panel_name.c_str(), _current_theme_name.c_str());
}

/// @brief callback function to be executed when trigger-driven panel loading is complete
void PanelManager::trigger_panel_switch_callback()
{
    log_d("Trigger panel load completed...");

    _is_loading = false;
    log_d("Trigger panel load completed, _is_loading is now %i", _is_loading);
    
    // Notify Core 1 of state change
    notify_core1_state_change(_current_panel_name.c_str(), _current_theme_name.c_str());
}

/// @brief Get the panel name to restore when all triggers are inactive
/// @return Panel name for restoration, or nullptr if none set
const char* PanelManager::get_restoration_panel() const
{
    if (_last_non_trigger_panel.empty()) {
        return nullptr;
    }
    return _last_non_trigger_panel.c_str();
}

// Core 0 Dual-Core Methods

/// @brief Process trigger messages from Core 1 based on UI state
void PanelManager::process_trigger_messages()
{
    switch (_ui_state) {
        case UIState::IDLE:
            // No throttling - process all priority queues
            process_all_priority_queues();
            break;
            
        case UIState::UPDATING:
            // State-based throttling - only high/medium priority queues
            process_high_priority_queue();
            process_medium_priority_queue();
            // Skip low priority queue during updates
            break;
            
        case UIState::LOADING:
        case UIState::LVGL_BUSY:
            // No action - don't process any queues
            // Messages remain queued for later processing
            break;
    }
}

/// @brief Set current UI state for Core 1 synchronization
void PanelManager::set_ui_state(UIState state)
{
    _ui_state = state;
    log_d("UI State changed to: %d", (int)state);
}

/// @brief Get available queues from TriggerManager for processing
void PanelManager::get_trigger_queues()
{
    TriggerManager::get_instance().get_queue_handles(&_high_priority_queue, &_medium_priority_queue, &_low_priority_queue);
    log_d("Obtained queue handles from TriggerManager");
}

/// @brief Execute a trigger message action
void PanelManager::execute_message_action(const TriggerMessage& msg)
{
    log_d("Executing message - Action: %s, Target: %s", msg.action, msg.target);
    
    if (strcmp(msg.action, ACTION_LOAD_PANEL) == 0) {
        _current_panel_name = std::string(msg.target);
        create_and_load_panel(msg.target, [this]() { this->trigger_panel_switch_callback(); }, true);
        
    } else if (strcmp(msg.action, ACTION_RESTORE_PREVIOUS_PANEL) == 0) {
        const char* restore_panel = get_restoration_panel();
        if (restore_panel) {
            _current_panel_name = std::string(restore_panel);
            create_and_load_panel(restore_panel, [this]() { this->trigger_panel_switch_callback(); }, false);
        }
        
    } else if (strcmp(msg.action, ACTION_CHANGE_THEME) == 0) {
        _current_theme_name = std::string(msg.target);
        // TODO: Implement theme change logic
        log_i("Theme change to %s (implementation needed)", msg.target);
        
        // Notify Core 1 immediately for theme changes
        notify_core1_state_change(_current_panel_name.c_str(), _current_theme_name.c_str());
    }
}

/// @brief Process all priority queues based on UI state
void PanelManager::process_all_priority_queues()
{
    process_high_priority_queue();
    process_medium_priority_queue();
    process_low_priority_queue();
}

/// @brief Process high priority queue only
void PanelManager::process_high_priority_queue()
{
    TriggerMessage msg;
    
    // Note: In real implementation, we'd have direct queue access
    // For now, this is a placeholder showing the intended logic
    if (_high_priority_queue && xQueueReceive(_high_priority_queue, &msg, 0) == pdTRUE) {
        execute_message_action(msg);
    }
}

/// @brief Process medium priority queue only
void PanelManager::process_medium_priority_queue()
{
    TriggerMessage msg;
    
    if (_medium_priority_queue && xQueueReceive(_medium_priority_queue, &msg, 0) == pdTRUE) {
        execute_message_action(msg);
    }
}

/// @brief Process low priority queue only
void PanelManager::process_low_priority_queue()
{
    TriggerMessage msg;
    
    if (_low_priority_queue && xQueueReceive(_low_priority_queue, &msg, 0) == pdTRUE) {
        execute_message_action(msg);
    }
}

/// @brief Notify Core 1 of state changes
void PanelManager::notify_core1_state_change(const char* panel_name, const char* theme_name)
{
    TriggerManager::get_instance().update_application_state(panel_name, theme_name);
    log_d("Notified Core 1 - Panel: %s, Theme: %s", panel_name, theme_name);
}