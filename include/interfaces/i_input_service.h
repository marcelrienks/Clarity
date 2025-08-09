#pragma once

#include "interfaces/i_input_action.h"
#include <memory>

/**
 * @interface IInputService
 * @brief Interface for panels to provide input actions to InputManager
 * 
 * @details This interface defines the contract for panels to specify what actions
 * should be taken for button input events. Panels return action objects that
 * InputManager can execute when appropriate, providing separation between
 * action definition and action execution.
 * 
 * @design_pattern Strategy + Command patterns - Actions as strategy objects
 * @execution_flow Panel defines actions → InputManager executes when ready
 * @timing Short press: 50ms-500ms, Long press: >500ms
 */
class IInputService
{
public:
    virtual ~IInputService() = default;

    /**
     * @brief Get action to execute for short button press (50ms - 500ms)
     * @details Called when button is pressed and released within short press window
     * @return Action object to execute, or nullptr for no action
     */
    virtual std::unique_ptr<IInputAction> GetShortPressAction() = 0;

    /**
     * @brief Get action to execute for long button press (>500ms)
     * @details Called when button is held down for longer than long press threshold
     * @return Action object to execute, or nullptr for no action
     */
    virtual std::unique_ptr<IInputAction> GetLongPressAction() = 0;

    /**
     * @brief Check if the panel can currently process input events
     * @details Used by InputManager to determine if inputs should be queued
     * @return true if the panel can process inputs immediately, false to queue
     */
    virtual bool CanProcessInput() const = 0;
};