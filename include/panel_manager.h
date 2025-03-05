#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_panel.h"
#include "interfaces/i_device.h"
#include "utilities/serial_logger.h"

#include <vector>
#include <map>
#include <string>
#include <functional>
#include <memory>

/**
 * @brief Enum representing the transition types available for panel transitions
 */
enum class TransitionType 
{
    NONE,       // No transition, instant switch
    FADE_IN,    // Fade in from black
    FADE_OUT,   // Fade out to black
    SLIDE_LEFT, // Slide from right to left
    SLIDE_RIGHT,// Slide from left to right
    SLIDE_UP,   // Slide from bottom to top
    SLIDE_DOWN  // Slide from top to bottom
};

/**
 * @brief Struct to hold the configuration for a panel transition
 */
struct TransitionConfig
{
    TransitionType type = TransitionType::FADE_IN;
    uint32_t duration = 500;       // Duration in milliseconds
    uint32_t delay = 0;            // Delay before starting the transition
    bool delete_previous = true;   // Whether to delete the previous panel after transition
};

class PanelManager 
{
private:
    IDevice* _device;
    std::map<std::string, std::shared_ptr<IPanel>> _panels; // Registered panels by name
    std::shared_ptr<IPanel> _current_panel;                 // Currently active panel
    std::string _current_panel_name;                        // Name of currently active panel
    TransitionConfig _default_transition;                   // Default transition config
    
    // Internal state for transitions
    bool _transition_in_progress = false;
    std::string _next_panel_name;
    TransitionConfig _pending_transition;
    std::function<void()> _transition_callback;

public:
    PanelManager(IDevice* device);
    ~PanelManager();

    /**
     * @brief Register a panel with the manager
     * @param name Unique name for the panel
     * @param panel The panel to register
     * @return True if successful, false if name already exists
     */
    bool register_panel(const std::string& name, std::shared_ptr<IPanel> panel);

    /**
     * @brief Show a panel by its registered name
     * @param name The name of the panel to show
     * @param transition Optional transition config (uses default if not provided)
     * @param completion_callback Optional callback function to execute when transition completes
     * @return True if successful, false if panel not found
     */
    bool show_panel(const std::string& name, 
                    const TransitionConfig& transition = TransitionConfig(),
                    std::function<void()> completion_callback = nullptr);

    /**
     * @brief Show the next panel in a sequence
     * @param transition Optional transition config
     * @param completion_callback Optional callback function
     * @return True if successful, false if there's no next panel
     */
    bool show_next_panel(const TransitionConfig& transition = TransitionConfig(),
                         std::function<void()> completion_callback = nullptr);

    /**
     * @brief Set the default transition config for all panel transitions
     * @param config The transition configuration to use as default
     */
    void set_default_transition(const TransitionConfig& config);

    /**
     * @brief Update the current panel
     * @note This should be called from the main loop
     */
    void update();

    /**
     * @brief Check if a transition is currently in progress
     * @return True if a transition is in progress
     */
    bool is_transitioning() const;

    /**
     * @brief Get the current panel
     * @return Pointer to the current panel
     */
    std::shared_ptr<IPanel> get_current_panel() const;

private:
    /**
     * @brief Handle transition completion
     * @param panel_name Name of the panel that completed transition
     */
    void on_transition_complete();
};
