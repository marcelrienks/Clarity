#pragma once

#include <functional>

// Forward declaration
class IActionService;

/**
 * @interface IActionManager
 * @brief Interface for action management functionality needed by panels and managers
 * 
 * @details This interface provides the contract for action management operations
 * including panel switches and input service registration. It supports proper 
 * dependency injection and testing by abstracting the concrete ActionManager 
 * implementation.
 * 
 * @design_pattern Interface Segregation Principle - expose what clients need
 * @dependency_injection Allows components to depend on interface instead of concrete class
 */
class IActionManager
{
public:
    virtual ~IActionManager() = default;

    /**
     * @brief Request a panel switch operation
     * @param targetPanel Name of the panel to switch to
     * @details This function provides panels a way to request panel switches
     * through the action system, maintaining separation of concerns
     */
    virtual void RequestPanelSwitch(const char* targetPanel) = 0;

    /**
     * @brief Set callback for panel switch requests from actions
     * @param callback Function to call when an action requests a panel switch
     * @details Allows PanelManager to register its panel switching function
     */
    virtual void SetPanelSwitchCallback(std::function<void(const char*)> callback) = 0;

    /**
     * @brief Register a panel as the current input service
     * @param service Pointer to panel implementing IActionService
     * @param panelName Name of the panel for action lookup
     * @details Called by PanelManager when panels are loaded
     */
    virtual void SetInputService(IActionService* service, const char* panelName) = 0;

    /**
     * @brief Remove current input service
     * @details Called by PanelManager when panels are unloaded
     */
    virtual void ClearInputService() = 0;
};