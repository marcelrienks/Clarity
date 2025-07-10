#include "managers/panel_manager.h"
#include "managers/interrupt_manager.h"
#include "triggers/key_trigger.h"

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
/// Also initializes interrupt manager with panel switching callback and registers global triggers
void PanelManager::init()
{
    log_d("...");
    Ticker::handle_lv_tasks();

    PanelManager::register_panels();
    PanelManager::register_triggers();

    // Initialize InterruptManager with panel switch callback
    InterruptManager &interrupt_manager = InterruptManager::get_instance();
    interrupt_manager.init([this](const char *panel_name) {
        this->create_and_load_panel(panel_name, [this]() { this->interrupt_panel_switch_callback(); });
    });
}

/// @brief Creates and then loads a panel based on the given type name
/// @param panel_name the type name of the panel to be loaded
/// @param completion_callback the function to be called when the panel load is complete
void PanelManager::create_and_load_panel(const char *panel_name, std::function<void()> completion_callback)
{
    log_d("...");

    InterruptManager::get_instance().clear_panel_triggers();
    InterruptManager::get_instance().set_current_panel(panel_name);
    _panel = PanelManager::create_panel(panel_name);
    
    log_i("Loading %s", _panel->get_name());

    // Lock the panel to prevent updating during loading
    _is_loading = true;
    log_v("_is_loading is now %i", _is_loading);

    // Initialize the panel
    _panel->init();
    Ticker::handle_lv_tasks();

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

/// @brief Update the reading on the currently loaded panel
void PanelManager::update_panel()
{
    log_d("...");

    // Check for interrupt conditions before normal panel update
    InterruptManager::get_instance().check_triggers();

    if (_is_loading)
        return;

    _is_loading = true;
    log_v("_is_loading is now %i", _is_loading);

    _panel->update([this]()
                   { this->PanelManager::panel_completion_callback(); });

    Ticker::handle_lv_tasks();
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
}

/// @brief Register all global triggers with the interrupt manager
void PanelManager::register_triggers()
{
    log_d("...");

    // Key trigger: Switch to key panel when key is detected (with restoration)
    register_global_trigger<KeyTrigger>(TriggerNames::Key, true);
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
    log_d("...");

    _is_loading = false;
    log_d("Panel load completed, _is_loading is now %i", _is_loading);
}

/// @brief callback function to be executed when interrupt-triggered panel loading is complete
void PanelManager::interrupt_panel_switch_callback()
{
    log_d("...");

    _is_loading = false;
    log_d("Interrupt panel load completed, _is_loading is now %i", _is_loading);
}