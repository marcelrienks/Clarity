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
     * @brief Register a panel as the current action handler
     * @param service Pointer to panel implementing IActionService
     * @param panelName Name of the panel for action lookup
     * @details Called by PanelManager when panels are loaded
     */
    virtual void RegisterPanel(IActionService *service, const char *panelName) = 0;

    /**
     * @brief Remove current panel registration
     * @details Called by PanelManager when panels are unloaded
     */
    virtual void ClearPanel() = 0;
};