#include "panels/panel_manager.h"

#include <iostream>

// TODO: implement logic to prevent rendering splash types more than once
PanelManager *g_panel_manager_instance = nullptr;

PanelManager::PanelManager(IDevice *device)
{
    // Set default transition
    _device = device;
}

PanelManager::~PanelManager()
{
    _panels.clear();
}

/// @brief Register a panel with the manager
/// @param panel The panel to register
void PanelManager::register_panel(std::shared_ptr<IPanel> panel)
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
/// @param completion_callback the function to be called when the panel show is complete
void PanelManager::show_panel(std::shared_ptr<IPanel> panel, std::function<void()> completion_callback)
{
    auto name = panel->get_name();
    SerialLogger().log_point("PanelManager::show_panel", "Panel: " + name);

    // Lock the panel to prevent updating during loading
    _is_panel_locked = true;

    // Show the panel with the appropriate transition
    panel->show(completion_callback);

    // Save as current panel
    _current_panel = panel.get();
}

void PanelManager::show_all_panels()
{
    if (!_is_recursion_locked && _panels.size() > 0)
    {
        SerialLogger().log_point("PanelManager::show_all_panels", "...");
        _is_recursion_locked = true;
        _panel_iterator = _panels.begin();
        PanelManager::show_panel_from_iterator();
    }
}

/// @brief Show the next panel in a sequence
void PanelManager::show_panel_from_iterator()
{
    SerialLogger().log_point("PanelManager::show_panel_from_iterator", "...");

    // Show the next panel, and use a lambda rather than a separate callback function so that 'this' can be captured
    PanelManager::show_panel(*_panel_iterator, PanelManager::show_panel_completion_callback);
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

    // Unlock and return from recursion if we've reached the end of the list
    if (g_panel_manager_instance->_is_recursion_locked && g_panel_manager_instance->_panel_iterator == g_panel_manager_instance->_panels.end())
    {
        SerialLogger().log_point("PanelManager::show_panel_completion_callback", "show_panel -> List end, unlock recursion");
        g_panel_manager_instance->_is_recursion_locked = false;
    }

    // Unlock panel for updates, and show the next panel after a configured display time
    else
    {
        SerialLogger().log_point("PanelManager::show_panel_completion_callback", "show_panel -> create display timer");
        g_panel_manager_instance->_is_panel_locked = false;

        //TODO: this causes a crash, why...
        auto type = g_panel_manager_instance->_current_panel->get_type();

        // create a display timer to show the current panel for an amount of time, unless it's a splash type
        auto display_time = type == PanelType::Splash ? 1 : PANEL_DISPLAY_TIME;

        lv_timer_create(display_timer_callback, display_time, g_panel_manager_instance);
    }
}

/// @brief Callback function after the display time of the current panel has elapsed
/// @param timer the timer that has elapsed
void PanelManager::display_timer_callback(lv_timer_t *timer)
{
    SerialLogger().log_point("PanelManager::display_timer_callback", "...");
    auto *panel_manager = static_cast<PanelManager *>(lv_timer_get_user_data(timer));

    // Remove the timer after transition, this replaces having to set a repeat on the timer
    lv_timer_del(timer);

    SerialLogger().log_point("PanelManager::increment", "...");
    panel_manager->_current_panel = nullptr;
    panel_manager->_panel_iterator++;
    panel_manager->show_panel_from_iterator();
}