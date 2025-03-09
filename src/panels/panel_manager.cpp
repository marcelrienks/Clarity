#include "panels/panel_manager.h"

#include <iostream>

/**
 * @brief Construct a new Panel Manager
 * @param device Pointer to the device instance
 */
PanelManager::PanelManager(IDevice *device)
{
    SerialLogger().log_point("PanelManager::PanelManager", "Entry...");

    // Set default transition
    _device = device;
}

/**
 * @brief Destroy the Panel Manager
 */
PanelManager::~PanelManager()
{
    SerialLogger().log_point("PanelManager::~PanelManager", "Entry...");
    _panels.clear();
}

/**
 * @brief Register a panel with the manager
 * @param panel The panel to register
 * @return True if successful, false if name already exists
 */
void PanelManager::register_panel(std::shared_ptr<IPanel> panel)
{
    SerialLogger().log_point("PanelManager::register_panel", "Registering panel: " + panel->get_name());

    // Check if the panel already exists
    if (std::find(_panels.begin(), _panels.end(), panel) == _panels.end())
        _panels.push_back(panel); // Register the panel

    // Initialize the panel with the device
    panel->init(_device);
}

/**
 * @brief Show a panel by its registered name
 * @param name The name of the panel to show
 * @param transition Optional transition config (uses default if not provided)
 * @param completion_callback Optional callback function to execute when transition completes
 * @return True if successful, false if panel not found
 */
void PanelManager::show_panel(std::shared_ptr<IPanel> panel, std::function<void()> completion_callback)
{
    auto name = panel->get_name();
    SerialLogger().log_point("PanelManager::show_panel", "Entry: " + name);

    _panel_locked = true;

    // Show the panel with the appropriate transition
    panel->show(completion_callback);

    // Save as current panel
    _current_panel = panel.get();
}

/**
 * @brief Show the next panel in a sequence
 * @param depth depth of the current iteration of the recursion
 */
void PanelManager::show_panels_recursively()
{
    if (!_recursion_locked)
    {
        SerialLogger().log_point("PanelManager::show_panels_recursively", "Entry...");
        _recursion_locked = true;

        // Initialise iterator if needed
        if (_recursion_depth == 0 && _panel_iterator == _panels.end())
            _panel_iterator = _panels.begin();

        // Show the next panel, and use a lambda rather than a separate callback function so that 'this' can be captured
        PanelManager::show_panel(*_panel_iterator,
                                 [this]()
                                 {
                                     // Unlock and return from recursion if we've reached the end of the list
                                     if (this->_recursion_depth != 0 && this->_panel_iterator == this->_panels.end())
                                     {
                                         this->_recursion_depth = 0; // reset depth
                                         _recursion_locked = false;
                                         return;
                                     }

                                     // Unlock panel for updates, and show the next panel after a configured display time
                                     _panel_locked = false;
                                     lv_timer_create(show_panel_timer_completion_callback, PANEL_DISPLAY_TIME, this);
                                 });
    }
}

/**
 * @brief Update the current panel
 * @note This should be called from the main loop
 */
void PanelManager::update_current_panel()
{
    SerialLogger().log_point("PanelManager::update_current_panel", "Entry...");

    // Return if there is no panel, or if it's locked during loading
    if (!_current_panel || _panel_locked)
        return;

    // Update the current panel
    _current_panel->update();
}

/**
 * @brief Handle transition completion
 */
void PanelManager::show_panel_timer_completion_callback(lv_timer_t *timer)
{
    SerialLogger().log_point("PanelManager::show_panel_timer_completion_callback", "Entry...");

    auto *panel_manager = static_cast<PanelManager *>(lv_timer_get_user_data(timer));
    panel_manager->_panel_iterator++;
    panel_manager->_recursion_depth++;
    panel_manager->show_panels_recursively();
}