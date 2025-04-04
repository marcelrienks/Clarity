#include "handlers/panel_manager.h"
#include "panels/splash_panel.h"
#include "panels/demo_panel.h"

PanelManager::PanelManager()
    : _current_panel(nullptr), _is_show_all_locked(false), _is_panel_locked(false) {}

PanelManager::~PanelManager()
{
    _panels_ptr.clear();
    _current_panel = nullptr;
}

/// @brief Initialise the panel manager to control the flow and rendering of all panels
void PanelManager::init(IDevice *device)
{
    log_v("...");
    _device = device;

    // Register all available panel types with the factory
    register_panel_types();

    // Load panel configuration from preferences
    read_panels_from_preferences();

    _panels_ptr_it = _panels_ptr.begin();
}

/// @brief Register all panel types available to this application
void PanelManager::register_panel_types()
{
    // Register all known panel types with the factory
    PanelFactory &factory = PanelFactory::get_instance();
    factory.register_panel<SplashPanel>("SplashPanel");
    factory.register_panel<DemoPanel>("DemoPanel");

    // Register more panel types here as they are added
}

/// @brief Load configured panels from preferences that should be rendered in order
void PanelManager::read_panels_from_preferences()
{
    // Clear any existing panels
    _panels_ptr.clear();
    log_v("%i panels to be loaded", PreferenceManager::config.panels.size());

    // Create and register each panel from the configuration
    PanelFactory &factory = PanelFactory::get_instance();
    for (const auto &panel_config : PreferenceManager::config.panels)
    {
        log_v("Loading panel: %s", panel_config.name.c_str());

        if (factory.is_panel_type_registered(panel_config.name))
        {
            auto panel = factory.create_panel(_device, panel_config.name, panel_config.iteration);
            if (panel)
                register_panel(panel);

            else
                log_w("Failed to create panel: %s", panel_config.name);
        }
        else
            log_w("Unknown panel type: %s", panel_config.name);

    }
}

/// @brief Register panel with application
/// @param panel_ptr shared pointer to the panel
void PanelManager::register_panel(std::shared_ptr<IPanel> panel_ptr)
{
    log_d("Registering panel: %s" , panel_ptr->get_name().c_str());

    // Check if the panel already exists
    if (std::find(_panels_ptr.begin(), _panels_ptr.end(), panel_ptr) == _panels_ptr.end())
        _panels_ptr.push_back(panel_ptr); // Register the panel

    // Initialize the panel with the device
    panel_ptr->init();
}

/// @brief Show all panels in the list
void PanelManager::show_all_panels()
{
    log_v("_is_show_all_locked is %i", _is_show_all_locked);
    if (_is_show_all_locked) return;

    _is_show_all_locked = true;

    // List end logic
    if (_panels_ptr_it == _panels_ptr.end())
    {
        log_i("end of the list, resetting");
        _panels_ptr_it = _panels_ptr.begin();
    }

    // If recursion is not locked, and splash has been handled, start the recursion, this is meant for loop from main
    // Note: show_panel_from_iterator will only be run from here again, once recursion is unlocked
    // but will continue to be called recursively from the display timer callback
    PanelManager::show_panel(_panels_ptr_it->get(), [this]()
                             { this->show_panel_completion_callback(); });
}

/// @brief Show the given panel
/// @param panel the panel to be shown
/// @param show_panel_completion_callback the function to be called when the panel show is complete
void PanelManager::show_panel(IPanel *panel, std::function<void()> show_panel_completion_callback)
{
    log_d("_is_panel_locked is %i", _is_panel_locked);

    // Panel locked for loading or updating
    if (_is_panel_locked) return;

    // Panel already shown logic
    if (panel == _current_panel)
    {
        log_i("panel %s is already shown", _current_panel->get_name());
        return;
    }

    if (panel->get_iteration() == PanelIteration::Disabled)
    {
        log_i("panel %s is disabled", panel->get_name());
        return;
    }

    // Lock the panel to prevent updating during loading
    _is_panel_locked = true;
    log_d("_is_panel_locked is %i", _is_panel_locked);

    _current_panel = panel;

    panel->show(show_panel_completion_callback);
}

/// @brief Update the reading on the currently loaded panel
void PanelManager::update_current_panel()
{
    log_v("_is_panel_locked is %i", _is_panel_locked);

    if (!_current_panel || _is_panel_locked) return;

    _is_panel_locked = true;
    log_v("_is_panel_locked is %i", _is_panel_locked);

    _current_panel->update([this]()
                           { this->update_current_panel_completion_callback(); });
}

/// @brief callback function to be executed on panel show completion
void PanelManager::show_panel_completion_callback()
{
    _is_panel_locked = false;
    log_d("_is_panel_locked is %i", _is_panel_locked);

    _panels_ptr_it++;
    int display_time = PANEL_DISPLAY_TIME;

    // Handle completion of Splash screen
    if (_current_panel->get_type() == PanelType::Splash)
    {
        display_time = 100;
        if (_current_panel->get_iteration() == PanelIteration::Once)
            _current_panel->set_iteration(PanelIteration::Disabled);
    }

    // Create a display timer to show the current panel for an amount of time, unless it's a splash type
    log_d("create display timer");
    lv_timer_create(PanelManager::display_timer_callback, display_time, this);
}

/// @brief Request panel to update it's reading and render that
void PanelManager::update_current_panel_completion_callback()
{
    _is_panel_locked = false;
    log_d("_is_panel_locked is %i", _is_panel_locked);
}

/// @brief callback function to be executed when display time of the current panel has elapsed
/// @param display_timer the timer that has elapsed
void PanelManager::display_timer_callback(lv_timer_t *display_timer)
{
    auto *panel_manager_instance = static_cast<PanelManager *>(lv_timer_get_user_data(display_timer));
    panel_manager_instance->_is_show_all_locked = false;

    log_i("completed display of panel %s",  panel_manager_instance->_current_panel->get_name().c_str());

    // Remove the timer after transition, this replaces having to set a repeat on the timer
    lv_timer_del(display_timer);
}