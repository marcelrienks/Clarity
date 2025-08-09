#pragma once

/**
 * @interface IInterrupt
 * @brief Interface for systems that need periodic interrupt checking during idle time
 * 
 * @details This interface abstracts the concept of interrupt-style processing for
 * both trigger events and input events. It allows any system to register for
 * periodic checking during idle time, ensuring responsive event handling even
 * during animations or other blocking operations.
 * 
 * @design_pattern Strategy pattern for interrupt handling
 * @priority_system Higher priority interrupts are checked first
 * @idle_integration Called during LVGL idle time and animation gaps
 */
class IInterrupt
{
public:
    virtual ~IInterrupt() = default;

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

    /**
     * @brief Get the priority level for this interrupt source
     * @details Higher priority interrupts are checked first.
     * Typical priorities: Triggers=100, Input=50, Background=10
     * @return Priority value (higher = more important)
     */
    virtual int GetPriority() const = 0;
};