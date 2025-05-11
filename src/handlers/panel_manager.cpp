#include "handlers/panel_manager.h"

PanelManager::PanelManager()
    : _panel(nullptr), _is_panel_locked(false) {}

PanelManager::~PanelManager()
{
    _panel = nullptr;
}

/// @brief Initialise the panel manager to control the flow and rendering of all panels
void PanelManager::init(PreferenceManager *preference_manager, IDevice *device)
{
    log_v("...");
    _preference_manager = _preference_manager;
    _device = device;

    // Register all available panel types with the factory
    register_panel_types();

    // Load panel from configuration
    read_panel_from_preferences();
}

/// @brief Register all panel types available to this application
void PanelManager::register_panel_types()
{
    // Register all known panel types with the factory
    PanelFactory &factory = PanelFactory::get_instance();
    factory.register_panel<SplashPanel>(SPLASH_PANEL);
    factory.register_panel<DemoPanel>(DEMO_PANEL);
    factory.register_panel<OilPanel>(OIL_PANEL);

    // Register more panel types here as they are added
}

/// @brief Load configured panels from preferences that should be rendered in order
void PanelManager::read_panel_from_preferences()
{
    // Create and register each panel from the configuration
    std::string panel = _preference_manager->config.panel;
    log_v("Loading panel %s from config", panel);

    auto &factory = PanelFactory::get_instance();
    auto _panel = factory.create_panel(_device, panel);
    _panel->init();
}

/// @brief Show the given panel
/// @param panel the panel to be shown
/// @param show_panel_completion_callback the function to be called when the panel show is complete
void PanelManager::show_panel(IPanel *panel, std::function<void()> show_panel_completion_callback)
{
    // Panel locked for loading or updating
    if (_is_panel_locked)
    {
        log_d("_is_panel_locked is %i", _is_panel_locked);
        return;
    }

    // Panel already shown logic
    if (panel == _panel)
    {
        log_d("panel %s is already shown", _panel->get_name());
        return;
    }

    // Lock the panel to prevent updating during loading
    _is_panel_locked = true;
    log_d("_is_panel_locked is now %i", _is_panel_locked);

    _panel = panel;

    panel->show(show_panel_completion_callback);
}

/// @brief Update the reading on the currently loaded panel
void PanelManager::update_current_panel()
{
    log_v("_is_panel_locked is %i", _is_panel_locked);

    if (!_panel || _is_panel_locked)
        return;

    _is_panel_locked = true;
    log_v("_is_panel_locked is %i", _is_panel_locked);

    _panel->update([this]()
                           { this->update_current_panel_completion_callback(); });
}

/// @brief callback function to be executed on panel show completion
void PanelManager::show_panel_completion_callback()
{
    _is_panel_locked = false;
    log_d("_is_panel_locked is %i", _is_panel_locked);
}

/// @brief Request panel to update it's reading and render that
void PanelManager::update_current_panel_completion_callback()
{
    _is_panel_locked = false;
    log_d("_is_panel_locked is %i", _is_panel_locked);
}