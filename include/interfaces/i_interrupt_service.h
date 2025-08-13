#pragma once

/**
 * @interface IInterruptService
 * @brief Interface for systems that need periodic interrupt checking during idle time
 *
 * @details This interface abstracts the concept of interrupt-style processing for
 * both trigger events and input events. It allows any system to register for
 * periodic checking during idle time, ensuring responsive event handling even
 * during animations or other blocking operations.
 *
 * @design_pattern Strategy pattern for interrupt handling
 * @evaluation_order Triggers are evaluated first, actions only if no triggers active
 * @idle_integration Called during LVGL idle time and animation gaps
 */
class IInterruptService
{
  public:
    virtual ~IInterruptService() = default;

    /**
     * @brief Check for pending interrupts and process them
     * @details This method should be lightweight and non-blocking.
     * Heavy processing should be deferred or queued.
     */
    virtual void CheckInterrupts() = 0;

    /**
     * @brief Check if there are any pending interrupts without processing them
     * @details Used to optimize interrupt checking - skip if no pending work
     * @return true if interrupts are pending, false otherwise
     */
    virtual bool HasPendingInterrupts() const = 0;
};