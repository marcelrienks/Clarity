#pragma once

/**
 * @interface IInputAction
 * @brief Interface for panel-specific input actions that InputManager can execute
 * 
 * @details This interface represents an action that can be triggered by button input.
 * Panels create concrete action objects that encapsulate their specific behavior,
 * allowing InputManager to coordinate timing and execution without knowing the
 * implementation details.
 * 
 * @design_pattern Command pattern - encapsulates actions as objects
 * @execution_context Actions are executed by InputManager when appropriate
 * @panel_autonomy Each panel defines its own action implementations
 */
class IInputAction
{
public:
    virtual ~IInputAction() = default;

    /**
     * @brief Execute the action
     * @details This method should perform the intended action immediately.
     * Long-running operations should be avoided or handled asynchronously.
     */
    virtual void Execute() = 0;

    /**
     * @brief Get a description of what this action does
     * @details Used for debugging and logging purposes
     * @return Human-readable description of the action
     */
    virtual const char* GetDescription() const = 0;

    /**
     * @brief Check if this action can be executed immediately
     * @details Some actions may need to wait for certain conditions
     * @return true if action can execute now, false to defer
     */
    virtual bool CanExecute() const { return true; }
    
    /**
     * @brief Get the type identifier for this action
     * @details Used to identify action types without RTTI/dynamic_cast
     * @return String identifier for the action type
     */
    virtual const char* GetActionType() const = 0;
};