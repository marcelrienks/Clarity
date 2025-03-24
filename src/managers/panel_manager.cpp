#include "managers/panel_manager.h"
#include "panels/splash_panel.h"
#include "panels/demo_panel.h"

PanelManager::PanelManager(IDevice *device, PreferenceManager *preference_manager)
    : _device(device), _preference_manager(preference_manager), _current_panel(nullptr), _is_show_all_locked(false), _is_panel_locked(false) {}

PanelManager::~PanelManager()
{
    _panels_ptr.clear();
    _current_panel = nullptr;
}

void PanelManager::init()
{
    // Register all available panel types with the factory
    register_panel_types();

    // Load panel configuration from preferences
    load_panels_from_preferences();

    _panels_ptr_it = _panels_ptr.begin();
}

void PanelManager::register_panel_types()
{
    // Register all known panel types with the factory
    PanelFactory &factory = PanelFactory::get_instance();
    factory.register_panel<SplashPanel>("SplashPanel");
    factory.register_panel<DemoPanel>("DemoPanel");

    // Register more panel types here as they are added
}

void PanelManager::load_panels_from_preferences()
{
    // Clear any existing panels
    _panels_ptr.clear();

    // Attempt to load panel configuration from preferences
    std::vector<PanelConfig> configs = _preference_manager->load_panel_configs();

    // If no configurations were found, save and load defaults
    if (configs.empty())
    {
        SerialLogger().log_point("PanelManager::load_panels_from_preferences", "No panel configurations found. Using defaults.");
        _preference_manager->save_default_panel_configs();
        configs = _preference_manager->load_panel_configs();
    }

    // Create and register each panel from the configuration
    PanelFactory &factory = PanelFactory::get_instance();
    for (const auto &config : configs)
    {
        SerialLogger().log_point("PanelManager::load_panels_from_preferences", "Loading panel: " + config.panel_name);

        if (factory.is_panel_type_registered(config.panel_name))
        {
            auto panel = factory.create_panel(config.panel_name, config.iteration);
            if (panel)
                register_panel(panel);

            else
                SerialLogger().log_point("PanelManager::load_panels_from_preferences", "Failed to create panel: " + config.panel_name);
        }
        else
            SerialLogger().log_point("PanelManager::load_panels_from_preferences", "Unknown panel type: " + config.panel_name);
    }
}

void PanelManager::register_panel(std::shared_ptr<IPanel> panel_ptr)
{
    SerialLogger().log_point("PanelManager::register_panel", "Registering panel: " + panel_ptr->get_name());

    // Check if the panel already exists
    if (std::find(_panels_ptr.begin(), _panels_ptr.end(), panel_ptr) == _panels_ptr.end())
        _panels_ptr.push_back(panel_ptr); // Register the panel

    // Initialize the panel with the device
    panel_ptr->init(_device);
}

/// @brief Show all panels in the list
void PanelManager::show_all_panels()
{
    if (_is_show_all_locked)
    {
        SerialLogger().log_point("PanelManager::show_all_panels", "show all locked");
        return;
    }

    SerialLogger().log_point("PanelManager::show_all_panels", "...");
    _is_show_all_locked = true;

    // List end logic
    if (_panels_ptr_it == _panels_ptr.end())
    {
        SerialLogger().log_point("PanelManager::show_iterator_panel", "end of the list, resetting");
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
    SerialLogger().log_point("PanelManager::show_panel", "...");

    // Panel locked for loading or updating
    if (_is_panel_locked)
    {
        SerialLogger().log_point("PanelManager::show_panel", "show panel locked");
        return;
    }

    // Panel already shown logic
    if (panel == _current_panel)
    {
        SerialLogger().log_point("PanelManager::show_panel", _current_panel->get_name() + " panel is already shown");
        return;
    }

    if (panel->get_iteration() == PanelIteration::Disabled)
    {
        SerialLogger().log_point("PanelManager::show_panel", panel->get_name() + " is disabled");
        return;
    }

    // Lock the panel to prevent updating during loading
    _is_panel_locked = true;
    SerialLogger().log_value("PanelManager::show_panel", "_is_panel_locked", std::to_string(_is_panel_locked));

    _current_panel = panel;

    panel->show(show_panel_completion_callback);
}

/// @brief Update the reading on the currently loaded panel
void PanelManager::update_current_panel()
{
    SerialLogger().log_point("PanelManager::update_current_panel", "...");

    if (!_current_panel || _is_panel_locked)
    {
        SerialLogger().log_point("PanelManager::update_current_panel", "panel locked");
        return;
    }

    _is_panel_locked = true;
    SerialLogger().log_value("PanelManager::update_current_panel", "_is_panel_locked", std::to_string(_is_panel_locked));

    _current_panel->update([this]()
                           { this->update_current_panel_completion_callback(); });
}

/// @brief callback function to be executed on panel show completion
void PanelManager::show_panel_completion_callback()
{
    SerialLogger().log_point("PanelManager::show_panel_completion_callback", "...");

    _is_panel_locked = false;
    SerialLogger().log_value("PanelManager::show_panel_completion_callback", "_is_panel_locked", std::to_string(_is_panel_locked));

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
    SerialLogger().log_point("PanelManager::show_panel_completion_callback", "show_panel -> create display timer");
    lv_timer_create(PanelManager::display_timer_callback, display_time, this);
}

void PanelManager::update_current_panel_completion_callback()
{
    SerialLogger().log_point("PanelManager::update_current_panel_completion_callback", "...");

    _is_panel_locked = false;
    SerialLogger().log_value("PanelManager::update_current_panel_completion_callback", "_is_panel_locked", std::to_string(_is_panel_locked));
}

/// @brief callback function to be executed when display time of the current panel has elapsed
/// @param display_timer the timer that has elapsed
void PanelManager::display_timer_callback(lv_timer_t *display_timer)
{
    SerialLogger().log_point("PanelManager::display_timer_callback", "...");
    auto *panel_manager_instance = static_cast<PanelManager *>(lv_timer_get_user_data(display_timer));
    panel_manager_instance->_is_show_all_locked = false;

    SerialLogger().log_point("PanelManager::display_timer_callback", "completed display of panel " + panel_manager_instance->_current_panel->get_name());

    // Remove the timer after transition, this replaces having to set a repeat on the timer
    lv_timer_del(display_timer);
}