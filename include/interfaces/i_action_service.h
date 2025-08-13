#pragma once

#include "utilities/types.h"

/**
 * @interface IActionService
 * @brief Interface for panels to provide button actions to ActionManager
 *
 * @details This interface defines the contract for panels to specify what actions
 * should be taken for button input events. Panels return action objects that
 * ActionManager can execute when appropriate, providing separation between
 * action definition and action execution.
 *
 * @design_pattern Strategy + Command patterns - Actions as strategy objects
 * @execution_flow Panel defines actions → ActionManager executes when ready
 * @timing Short press: 50ms-2000ms, Long press: 2000ms-5000ms, Timeout: >5000ms
 */
class IActionService
{
  public:
    virtual ~IActionService() = default;

    /**
     * @brief Get action to execute for short button press (50ms - 2000ms)
     * @details Called when button is pressed and released within short press window
     * @return Action struct containing function to execute
     */
    virtual Action GetShortPressAction() = 0;

    /**
     * @brief Get action to execute for long button press (2000ms - 5000ms)
     * @details Called when button is held down for longer than long press threshold
     * @return Action struct containing function to execute
     */
    virtual Action GetLongPressAction() = 0;

    /**
     * @brief Check if the panel can currently process input events
     * @details Used by ActionManager to determine if inputs should be queued
     * @return true if the panel can process inputs immediately, false to queue
     */
    virtual bool CanProcessInput() const = 0;
};