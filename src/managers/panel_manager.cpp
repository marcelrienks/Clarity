#include "managers/panel_manager.h"

PanelManager::~PanelManager()
{
    _panel = nullptr;
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

    // Handle the splash panel, and then load the supplied panel
    PanelManager::load_panel(PanelNames::Splash, [this, panel_name]()
                             { PanelManager::load_panel(panel_name, [this]()
                                                        { this->PanelManager::completion_callback(); }); });
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
void PanelManager::load_panel(const char *panel_name, std::function<void()> completion_callback)
{
    // Create and register each panel from the configuration
    log_v("Loading %s", panel_name); // Also fixed format string

    // Panel locked for loading or updating
    if (_is_panel_locked)
    {
        log_d("_is_panel_locked is %i", _is_panel_locked);
        return;
    }

    // Panel already shown logic - check if _panel is not null first
    if (_panel != nullptr && panel_name == _panel->get_name())
    {
        log_d("panel %s is already shown", _panel->get_name().c_str());
        return;
    }

    _panel = PanelManager::create_panel(panel_name).get();

    // Add null check here too for safety
    if (_panel == nullptr)
    {
        log_e("Failed to create panel: %s", panel_name);
        return;
    }

    log_d("...This is the last log line I see");

    // Initialize the panel
    _panel->init();

    // Lock the panel to prevent updating during loading
    _is_panel_locked = true;
    log_d("_is_panel_locked is now %i", _is_panel_locked);

    _panel->load(completion_callback);

    Ticker::handle_lv_tasks();
}

/// @brief Update the reading on the currently loaded panel
void PanelManager::refresh_panel()
{
    log_v("_is_panel_locked is %i", _is_panel_locked);

    if (!_panel || _is_panel_locked)
        return;

    _is_panel_locked = true;
    log_v("_is_panel_locked is %i", _is_panel_locked);

    _panel->update([this]()
                   { this->PanelManager::completion_callback(); });

    Ticker::handle_lv_tasks();
}

/// @brief callback function to be executed on panel show completion
void PanelManager::completion_callback()
{
    _is_panel_locked = false;
    log_d("_is_panel_locked is %i", _is_panel_locked);
}