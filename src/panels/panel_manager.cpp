#include "panels/panel_manager.h"

PanelManager::PanelManager(IDevice *device)
{
    // Set default transition
    _device = device;

    // Initialize other members
    _current_panel = nullptr;
    _is_recursion_locked = false;
    _is_panel_locked = false;
}

PanelManager::~PanelManager()
{
    _panels.clear();
    _current_panel = nullptr;
    _device = nullptr;
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
/// @param show_panel_completion_callback the function to be called when the panel show is complete
void PanelManager::show_panel(std::shared_ptr<IPanel> panel, std::function<void()> show_panel_completion_callback)
{
    auto name = panel->get_name();
    SerialLogger().log_point("PanelManager::show_panel", "Panel: " + name);

    // Lock the panel to prevent updating during loading
    _is_panel_locked = true;

    // Save as current panel
    _current_panel = panel.get();

    // Show the panel with the appropriate transition
    panel->show(show_panel_completion_callback);
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

    // Show the next panel in list, unless we're at the end of the list
    if (_panel_iterator != _panels.end())
        PanelManager::show_panel(*_panel_iterator, [this]()
                                 { PanelManager::show_panel_completion_callback(); });

    else
        SerialLogger().log_point("PanelManager::show_panel_from_iterator", "we've reached the end of the list");
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
    if (_is_recursion_locked && _panel_iterator == _panels.end())
    {
        SerialLogger().log_point("PanelManager::show_panel_completion_callback", "show_panel -> List end, unlock recursion");
        _is_recursion_locked = false;
    }

    else // Unlock panel for updates, and show the next panel after a configured display time
    {
        SerialLogger().log_point("PanelManager::show_panel_completion_callback", "show_panel -> create display timer");
        _is_panel_locked = false;

        // Check if current_panel is valid before accessing it
        if (_current_panel)
        {
            // Create a display timer to show the current panel for an amount of time, unless it's a splash type
            auto display_time = _current_panel->get_type() == PanelType::Splash ? 1 : PANEL_DISPLAY_TIME;
            lv_timer_create(PanelManager::display_timer_callback, display_time, this);
        }

        else // Handle the null case - perhaps try to move to the next panel directly
        {
            SerialLogger().log_point("PanelManager::show_panel_completion_callback", "Error: _current_panel is null");

            _panel_iterator++;
            show_panel_from_iterator();
        }
    }
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

    SerialLogger().log_point("PanelManager::increment", "...");
    panel_manager_instance->_panel_iterator++;
    panel_manager_instance->show_panel_from_iterator();
}