#pragma once

/**
 * @interface IInterruptService
 * @brief Interface for systems that need periodic processing during idle time
 *
 * @details This interface provides a simple contract for systems that need
 * to be evaluated periodically. Both triggers and actions implement this to
 * check GPIO states, button presses, and other events during idle time.
 *
 * @design_pattern Strategy pattern for interrupt handling
 * @evaluation_order Triggers are evaluated first, then actions
 * @idle_integration Called during LVGL idle time and animation gaps
 */
class IInterruptService
{
  public:
    virtual ~IInterruptService() = default;

    /**
     * @brief Process any pending events or state changes
     * @details This method should be lightweight and non-blocking.
     * Heavy processing should be deferred or scheduled.
     * Called continuously during idle time.
     */
    virtual void Process() = 0;
};