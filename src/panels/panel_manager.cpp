#include "panels/panel_manager.h"

PanelManager::PanelManager(IDevice *device)
{
    _device = device;

    _is_recursion_locked = false;
    _is_panel_locked = false;

    // Set current panel as the splash for initial loading
    _current_panel = new SplashPanel();
    _current_panel->init(_device);
    is_splash_locked = false;
}

PanelManager::~PanelManager()
{
    _panels.clear();

    if (_current_panel)
    {
        delete _current_panel;
        _current_panel = nullptr;
    }

    if (_device)
    {
        delete _device;
        _device = nullptr;
    }
}

void PanelManager::init() {
    // Register panels with the manager
    register_panel(new DemoPanel());
}

/// @brief Register a panel with the manager
/// @param panel The panel to register
void PanelManager::register_panel(IPanel *panel)
{
    SerialLogger().log_point("PanelManager::register_panel", "Registering panel: " + panel->get_name());

    // Check if the panel already exists
    if (std::find(_panels.begin(), _panels.end(), panel) == _panels.end())
        _panels.push_back(panel); // Register the panel

    // Initialize the panel with the device
    panel->init(_device);
}

/// @brief Show the given panel
/// @param panel the panel to be shown
/// @param show_panel_completion_callback the function to be called when the panel show is complete
void PanelManager::show_panel(IPanel *panel, std::function<void()> show_panel_completion_callback)
{
    SerialLogger().log_point("PanelManager::show_panel", "...");

    auto name = panel->get_name();
    SerialLogger().log_point("PanelManager::show_panel", "Panel: " + name);

    // Lock the panel to prevent updating during loading
    _is_panel_locked = true;

    // Save as current panel
    _current_panel = panel;

    // Show the panel with the appropriate transition
    panel->show(show_panel_completion_callback);
}

/// @brief Show all panels in the list
void PanelManager::show_all_panels_recursively()
{
    if (_is_recursion_locked)
    {
        SerialLogger().log_point("PanelManager::show_all_panels", "recursion locked");
        return;
    }

    SerialLogger().log_point("PanelManager::show_all_panels", "...");
    _is_recursion_locked = true;
    _panel_iterator = _panels.begin();

    // Show Splash panel first, unless it's been disabled
    if (!is_splash_locked)
    {
        SerialLogger().log_point("PanelManager::show_all_panels", "handling splash panel");
        is_splash_locked = true;
        PanelManager::show_panel(_current_panel, [this]()
                                 { PanelManager::show_panel_completion_callback(); });

        return;
    }

    if (_panels.size() == 0)
    {
        SerialLogger().log_point("PanelManager::show_all_panels", "panel list is empty");
        // TODO: set up defaults
        return;
    }

    // If recursion is not locked, splash has been handled, and list is not empty, start the recursion
    // note show_panel_from_iterator will only be run from here again, once recursion is unlocked
    // but it will continue to be called recursively from the display timer callback
    SerialLogger().log_point("PanelManager::show_all_panels", "showing panel from list");
    PanelManager::show_panel_from_iterator();
}

/// @brief Show the next panel in a sequence
void PanelManager::show_panel_from_iterator()
{
    SerialLogger().log_point("PanelManager::show_panel_from_iterator", "...");

    // Show the next panel in list, unless we're at the end of the list
    if (_panel_iterator == _panels.end())
    {
        SerialLogger().log_point("PanelManager::show_panel_from_iterator", "we've reached the end of the list, unlocking recursion");
        _is_recursion_locked = false;
        return;
    }

    PanelManager::show_panel(*_panel_iterator, [this]()
                             { PanelManager::show_panel_completion_callback(); });

    SerialLogger().log_point("PanelManager::show_panel_from_iterator", "incrementing panel iterator");
    _panel_iterator++;
}

/// @brief Update the reading on the currently loaded panel
void PanelManager::update_current_panel()
{
    // Return if there is no panel, or if it's locked during loading
    if (_current_panel && !_is_panel_locked)
    {
        // Update the current panel
        SerialLogger().log_point("PanelManager::update_current_panel", "...");
        _current_panel->update();
    }
}

void PanelManager::show_panel_completion_callback()
{
    SerialLogger().log_point("PanelManager::show_panel_completion_callback", "...");
    _is_panel_locked = false;

    // Create a display timer to show the current panel for an amount of time, unless it's a splash type
    SerialLogger().log_point("PanelManager::show_panel_completion_callback", "show_panel -> create display timer");
    int display_time = _current_panel->get_type() == PanelType::Splash ? 1 : PANEL_DISPLAY_TIME;
    lv_timer_create(PanelManager::display_timer_callback, display_time, this);
}

/// @brief Callback function after the display time of the current panel has elapsed
/// @param display_timer the timer that has elapsed
void PanelManager::display_timer_callback(lv_timer_t *display_timer)
{
    SerialLogger().log_point("PanelManager::display_timer_callback", "...");
    auto *panel_manager_instance = static_cast<PanelManager *>(lv_timer_get_user_data(display_timer));

    SerialLogger().log_point("PanelManager::display_timer_callback", "completed display of panel " + panel_manager_instance->_current_panel->get_name());

    // Remove the timer after transition, this replaces having to set a repeat on the timer
    lv_timer_del(display_timer);
    panel_manager_instance->show_panel_from_iterator();
}