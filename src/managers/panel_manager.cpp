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
/// @param device the device to be used for panel rendering
/// @param previous_panel the name of the panel to be shown
void PanelManager::init(const char *panel_name)
{
    log_d("...");
    Ticker::handle_lv_tasks();

    // Register all available panel types with the factory
    register_panel<SplashPanel>(PanelNames::Splash);
    register_panel<DemoPanel>(PanelNames::Demo);
    register_panel<OilPanel>(PanelNames::Oil);

    _panel = PanelManager::create_panel(panel_name);
}

void PanelManager::load_panels()
{
    log_d("...");

    if (!_panel)
    {
        log_e("No panel is currently loaded");
        return;
    }

    // load_panel(_panel, [this]()
    //            { this->PanelManager::completion_callback(); });

    std::shared_ptr<IPanel> splash_panel = PanelManager::create_panel(PanelNames::Splash);
    load_panel(splash_panel, [this]()
               { this->PanelManager::splash_completion_callback(); });
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

/// @brief Show the given panel
/// @param panel the panel to be shown
/// @param show_panel_completion_callback the function to be called when the panel show is complete
void PanelManager::load_panel(std::shared_ptr<IPanel> panel, std::function<void()> completion_callback)
{
    // Create and register each panel from the configuration
    log_v("Loading %s", panel->get_name());

    // Lock the panel to prevent updating during loading
    _is_loading = true;
    log_d("_is_loading is now %i", _is_loading);

    // Initialize the panel
    panel->init();
    panel->load(completion_callback);
    Ticker::handle_lv_tasks();
}

/// @brief Update the reading on the currently loaded panel
void PanelManager::refresh_panel()
{
    log_v("_is_loading is %i", _is_loading);

    if (!_panel || _is_loading)
        return;

    _is_loading = true;
    log_v("_is_loading is %i", _is_loading);

    _panel->update([this]()
                   { this->PanelManager::completion_callback(); });

    Ticker::handle_lv_tasks();
}

/// @brief callback function to be executed on splash panel show completion
void PanelManager::splash_completion_callback()
{
    log_d("...");
    load_panel(_panel, [this]()
               { this->PanelManager::completion_callback(); });
}

/// @brief callback function to be executed on panel show completion
void PanelManager::completion_callback()
{
    log_d("...");

    _is_loading = false;
    log_d("Panel load completed, _is_loading is now %i", _is_loading);
}