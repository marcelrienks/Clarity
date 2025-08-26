#pragma once

#include "utilities/types.h"

/**
 * @interface IHandler
 * @brief Base interface for coordinated interrupt handlers in the Clarity system
 *
 * @details This interface defines the contract for specialized interrupt handlers
 * that process different types of interrupt sources. Handlers are coordinated by
 * InterruptManager which calls Process() during idle time to evaluate and execute
 * interrupts based on priority.
 *
 * @design_pattern Strategy pattern - Different handlers for different interrupt sources
 * @coordination InterruptManager coordinates multiple handlers by priority
 * @timing All processing occurs during LVGL idle time only
 *
 * @implementations:
 * - PolledHandler: GPIO state monitoring with change detection
 * - QueuedHandler: Button event processing with latest event handling
 *
 * @architecture Benefits of coordinated handler system:
 * - Specialized processing for POLLED vs QUEUED interrupt sources
 * - Priority-based coordination across different handler types
 * - Polymorphic processing through unified IHandler interface
 * - Clear separation of concerns between GPIO monitoring and button events
 * - Testable design with mockable handler interfaces
 *
 * @context This interface enables InterruptManager to coordinate multiple
 * specialized handlers while maintaining type safety and clear responsibilities.
 */
class IHandler
{
public:
    virtual ~IHandler() = default;

    /**
     * @brief Process interrupts for this handler type
     * @details Called by InterruptManager during idle time to evaluate
     * and mark active interrupts. Each handler processes its specific
     * interrupt sources (POLLED or QUEUED) and coordinates with
     * InterruptManager for priority-based execution.
     */
    virtual void Process() = 0;
    
    /**
     * @brief Get the highest priority active interrupt from this handler
     * @details Used by InterruptManager for cross-handler priority coordination.
     * Only returns interrupts that are currently active and whose evaluation
     * function returns true.
     * @return Pointer to highest priority active interrupt, or nullptr if none active
     */
    virtual const struct Interrupt* GetHighestPriorityActiveInterrupt() = 0;
};