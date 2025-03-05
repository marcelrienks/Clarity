#include "managers/panel_manager.h"

/**
 * @brief Construct a new Panel Manager
 * @param device Pointer to the device instance
 */
PanelManager::PanelManager(IDevice* device) : _device(device) 
{
    SerialLogger().log_point("PanelManager::PanelManager", "Creating panel manager");
    
    // Set default transition
    _default_transition.type = TransitionType::FADE_IN;
    _default_transition.duration = 500;
    _default_transition.delay = 0;
    _default_transition.delete_previous = true;
}

/**
 * @brief Destroy the Panel Manager
 */
PanelManager::~PanelManager() 
{
    SerialLogger().log_point("PanelManager::~PanelManager", "Destroying panel manager");
    _panels.clear();
    _current_panel = nullptr;
}

/**
 * @brief Register a panel with the manager
 * @param name Unique name for the panel
 * @param panel The panel to register
 * @return True if successful, false if name already exists
 */
bool PanelManager::register_panel(const std::string& name, std::shared_ptr<IPanel> panel) 
{
    SerialLogger().log_point("PanelManager::register_panel", "Registering panel: " + name);
    
    // Check if the panel name already exists
    if (_panels.find(name) != _panels.end()) {
        SerialLogger().log_point("PanelManager::register_panel", "Panel already exists: " + name);
        return false;
    }
    
    // Register the panel
    _panels[name] = panel;
    
    // Initialize the panel with the device
    panel->init(_device);
    
    SerialLogger().log_point("PanelManager::register_panel", "Panel registered successfully: " + name);
    return true;
}

/**
 * @brief Show a panel by its registered name
 * @param name The name of the panel to show
 * @param transition Optional transition config (uses default if not provided)
 * @param completion_callback Optional callback function to execute when transition completes
 * @return True if successful, false if panel not found
 */
bool PanelManager::show_panel(const std::string& name,
                             const TransitionConfig& transition,
                             std::function<void()> completion_callback) 
{
    SerialLogger().log_point("PanelManager::show_panel", "Showing panel: " + name);
    
    // Check if the panel exists
    auto it = _panels.find(name);
    if (it == _panels.end()) {
        SerialLogger().log_point("PanelManager::show_panel", "Panel not found: " + name);
        return false;
    }
    
    // Check if we're already displaying this panel
    if (_current_panel_name == name) {
        SerialLogger().log_point("PanelManager::show_panel", "Panel already showing: " + name);
        return true;
    }
    
    // If a transition is already in progress, queue this one
    if (_transition_in_progress) {
        SerialLogger().log_point("PanelManager::show_panel", "Transition in progress, queueing panel: " + name);
        _next_panel_name = name;
        _pending_transition = transition;
        _transition_callback = completion_callback;
        return true;
    }
    
    // Mark transition as in progress
    _transition_in_progress = true;
    _next_panel_name = name;
    
    // Prepare transition
    TransitionConfig effectiveTransition = transition;
    if (effectiveTransition.type == TransitionType::NONE) {
        effectiveTransition = _default_transition;
    }
    
    // Get the panel
    std::shared_ptr<IPanel> panel = it->second;
    
    // Set up completion callback on the panel
    panel->set_completion_callback([this, name, completion_callback]() {
        SerialLogger().log_point("PanelManager::on_transition_complete", "Transition complete for panel: " + name);
        
        // Update current panel
        _current_panel_name = name;
        _transition_in_progress = false;
        
        // Call the user's completion callback if provided
        if (completion_callback) {
            completion_callback();
        }
        
        // Process any pending transitions
        if (!_next_panel_name.empty() && _next_panel_name != _current_panel_name) {
            show_panel(_next_panel_name, _pending_transition, _transition_callback);
            _next_panel_name.clear();
        }
    });
    
    // Show the panel with the appropriate transition
    panel->show();
    
    // Save as current panel
    _current_panel = panel;
    
    SerialLogger().log_point("PanelManager::show_panel", "Panel show initiated: " + name);
    return true;
}

/**
 * @brief Show the next panel in a sequence
 * @param transition Optional transition config
 * @param completion_callback Optional callback function
 * @return True if successful, false if there's no next panel
 */
bool PanelManager::show_next_panel(const TransitionConfig& transition,
                                 std::function<void()> completion_callback) 
{
    SerialLogger().log_point("PanelManager::show_next_panel", "Showing next panel");
    
    // If no current panel, can't determine next
    if (_current_panel_name.empty()) {
        SerialLogger().log_point("PanelManager::show_next_panel", "No current panel, can't show next");
        return false;
    }
    
    // Find current panel in map
    auto it = _panels.find(_current_panel_name);
    if (it == _panels.end()) {
        SerialLogger().log_point("PanelManager::show_next_panel", "Current panel not found in registry");
        return false;
    }
    
    // Get next panel
    it++;
    if (it == _panels.end()) {
        // Wrap around to first panel
        it = _panels.begin();
    }
    
    // Show the next panel
    return show_panel(it->first, transition, completion_callback);
}

/**
 * @brief Set the default transition config for all panel transitions
 * @param config The transition configuration to use as default
 */
void PanelManager::set_default_transition(const TransitionConfig& config) 
{
    SerialLogger().log_point("PanelManager::set_default_transition", "Setting default transition");
    _default_transition = config;
}

/**
 * @brief Update the current panel
 * @note This should be called from the main loop
 */
void PanelManager::update() 
{
    // If no current panel, nothing to update
    if (!_current_panel) {
        return;
    }
    
    // Update the current panel
    _current_panel->update();
}

/**
 * @brief Check if a transition is currently in progress
 * @return True if a transition is in progress
 */
bool PanelManager::is_transitioning() const 
{
    return _transition_in_progress;
}

/**
 * @brief Get the current panel
 * @return Pointer to the current panel
 */
std::shared_ptr<IPanel> PanelManager::get_current_panel() const 
{
    return _current_panel;
}

/**
 * @brief Handle transition completion
 */
void PanelManager::on_transition_complete() 
{
    _transition_in_progress = false;
    
    // If there's a pending transition, process it
    if (!_next_panel_name.empty() && _next_panel_name != _current_panel_name) {
        show_panel(_next_panel_name, _pending_transition, _transition_callback);
        _next_panel_name.clear();
    }
}
