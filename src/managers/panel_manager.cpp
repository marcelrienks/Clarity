#include "managers/panel_manager.h"

PanelManager::PanelManager(IDevice *device)
{
    _device = device;
    _current_panel = nullptr;
    _is_show_all_locked = false;
    _is_show_panel_locked = false;
}

PanelManager::~PanelManager()
{
    _panels_ptr.clear();

    if (_current_panel)
        _current_panel = nullptr;

    if (_device)
    {
        delete _device;
        _device = nullptr;
    }
}

/// @brief initialises the Panel Manager for the first run
void PanelManager::init()
{
    // TODO: implement preferences

    if (_panels_ptr.size() == 0)
        PanelManager::init_default_panels();

    _panels_ptr_it = _panels_ptr.begin();
}

/// @brief initialises a default list of panels
void PanelManager::init_default_panels()
{
    // Register panels with the manager
    PanelManager::register_panel(std::make_shared<SplashPanel>(PanelIteration::Once));
    PanelManager::register_panel(std::make_shared<DemoPanel>(PanelIteration::Infinite));
}

/// @brief Register a panel with the manager
/// @param panel_ptr The panel to register
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
                             { PanelManager::show_panel_completion_callback(); });
}

/// @brief Show the given panel
/// @param panel the panel to be shown
/// @param show_panel_completion_callback the function to be called when the panel show is complete
void PanelManager::show_panel(IPanel *panel, std::function<void()> show_panel_completion_callback)
{
    SerialLogger().log_point("PanelManager::show_panel", "...");

    if (_is_show_panel_locked)
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

    if (panel->Panel_Iteration == PanelIteration::Disabled)
    {
        SerialLogger().log_point("PanelManager::show_panel", panel->get_name() + " is disabled");
        return;
    }

    // Lock the panel to prevent updating during loading
    _is_show_panel_locked = true;
    _current_panel = panel;

    panel->show(show_panel_completion_callback);
}

/// @brief Update the reading on the currently loaded panel
void PanelManager::update_current_panel()
{
    if (_current_panel && !_is_show_panel_locked)
    {
        SerialLogger().log_point("PanelManager::update_current_panel", "...");
        _current_panel->update();
    }
}

/// @brief callback function to be executed on panel show completion
void PanelManager::show_panel_completion_callback()
{
    SerialLogger().log_point("PanelManager::show_panel_completion_callback", "...");
    _is_show_panel_locked = false;
    _panels_ptr_it++;
    int display_time = PANEL_DISPLAY_TIME;

    // Handle completion of Splash screen
    if (_current_panel->get_type() == PanelType::Splash)
    {
        display_time = 100;
        if (_current_panel->Panel_Iteration == PanelIteration::Once)
            _current_panel->Panel_Iteration = PanelIteration::Disabled;
    }

    // Create a display timer to show the current panel for an amount of time, unless it's a splash type
    SerialLogger().log_point("PanelManager::show_panel_completion_callback", "show_panel -> create display timer");
    lv_timer_create(PanelManager::display_timer_callback, display_time, this);
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