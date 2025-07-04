#include "managers/panel_manager.h"

PanelManager::~PanelManager()
{
    _panel.reset();
}

/// @brief Get the singleton instance of PanelManager
/// @return instance of PanelManager
PanelManager &PanelManager::get_instance()
{
    static PanelManager instance; // this ensures that the instance is created only once
    return instance;
}

/// @brief Initialise the panel manager to control the flow and rendering of all panels
/// Registers all available panel types with the factory for dynamic creation
void PanelManager::init()
{
    log_d("...");
    Ticker::handle_lv_tasks();

    // Register all available panel types with the factory
    register_panel<SplashPanel>(PanelNames::Splash);
    register_panel<OemOilPanel>(PanelNames::Oil);
}

/// @brief Loads a panel based on the given type name
/// This function will create the panel and then call the load function on it.
/// @param panel_name the type name of the panel to be loaded
void PanelManager::load_panel(const char *panel_name)
{
    log_d("...");

    _panel = PanelManager::create_panel(panel_name);
    load_panel(_panel, [this]()
               { this->PanelManager::panel_completion_callback(); });
}

/// @brief Loads a panel based on the given type name after first loading a splash screen
/// This function will create the panel and then call the load function on it.
/// @param panel_name the type name of the panel to be loaded
void PanelManager::load_panel_with_Splash(const char *panel_name)
{
    log_d("...");

    _panel = PanelManager::create_panel(PanelNames::Splash);
    load_panel(_panel, [this, panel_name]()
               { this->PanelManager::splash_completion_callback(panel_name); });
}

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

/// @brief Load the given panel with initialization and callback handling
/// @param panel the panel to be loaded
/// @param completion_callback the function to be called when the panel load is complete
void PanelManager::load_panel(std::shared_ptr<IPanel> panel, std::function<void()> completion_callback)
{
    log_i("Loading %s", panel->get_name());

    // Lock the panel to prevent updating during loading
    _is_loading = true;
    log_v("_is_loading is now %i", _is_loading);

    // Initialize the panel
    panel->init();
    Ticker::handle_lv_tasks();

    panel->load(completion_callback);
    Ticker::handle_lv_tasks();
}

/// @brief Update the reading on the currently loaded panel
void PanelManager::refresh_panel()
{
    log_d("...");

    if (!_panel || _is_loading)
        return;

    _is_loading = true;
    log_v("_is_loading is now %i", _is_loading);

    _panel->update([this]()
                   { this->PanelManager::panel_completion_callback(); });

    Ticker::handle_lv_tasks();
}

/// @brief callback function to be executed on splash panel show completion
void PanelManager::splash_completion_callback(const char *panel_name)
{
    log_d("...");

    _panel.reset();
    Ticker::handle_lv_tasks();

    _panel = PanelManager::create_panel(panel_name);
    load_panel(_panel, [this]()
               { this->PanelManager::panel_completion_callback(); });
}

/// @brief callback function to be executed on panel show completion
void PanelManager::panel_completion_callback()
{
    log_d("...");

    _is_loading = false;
    log_d("Panel load completed, _is_loading is now %i", _is_loading);
}