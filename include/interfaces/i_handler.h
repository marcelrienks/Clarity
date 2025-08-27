#pragma once

#include "utilities/types.h"

/**
 * @interface IHandler
 * @brief Base interface for hybrid interrupt handlers with single-evaluation processing
 *
 * @details This interface defines the contract for specialized interrupt handlers
 * that use a three-phase processing approach: evaluate all → execute highest priority → reset state.
 * This eliminates race conditions and double-evaluation issues while maintaining handler separation.
 *
 * @design_pattern Hybrid Strategy + Template Method pattern
 * @architecture Each handler implements single-evaluation priority processing:
 * 1. EvaluateAllInterrupts() - Call processFunc once per interrupt and cache results
 * 2. ExecuteHighestPriorityInterrupt() - Execute based on priority and exclusion rules  
 * 3. ClearStateChanges() - Reset cached state for next cycle
 *
 * @implementations:
 * - PolledHandler: GPIO state monitoring with cached evaluation
 * - QueuedHandler: Button event processing with queued execution
 *
 * @benefits:
 * - No race conditions: each handler owns its interrupts exclusively
 * - Single evaluation: sensors evaluated exactly once per cycle
 * - Priority processing: highest priority interrupt executed first
 * - Exclusion rules: ALWAYS, EXCLUSIVE, CONDITIONAL execution modes
 * - Clean state management: reset between cycles
 *
 * @context This hybrid approach combines the handler separation of the old system
 * with the single-evaluation logic of the new architecture.
 */
class IHandler
{
public:
    virtual ~IHandler() = default;

    /**
     * @brief Process interrupts using hybrid three-phase approach
     * @details Implements: evaluate all → execute highest priority → reset state
     * This eliminates race conditions and ensures single evaluation per cycle.
     * Each handler processes only its registered interrupts exclusively.
     */
    virtual void Process() = 0;
    
    /**
     * @brief Register an interrupt with this handler
     * @param interrupt Pointer to interrupt structure (handler takes ownership)
     * @details Handler will process this interrupt using the three-phase approach.
     * Interrupt will be exclusively owned by this handler to prevent race conditions.
     */
    virtual void RegisterInterrupt(struct Interrupt* interrupt) = 0;
    
    /**
     * @brief Unregister an interrupt from this handler
     * @param id Interrupt ID to remove
     * @details Removes interrupt from handler's processing list.
     */
    virtual void UnregisterInterrupt(const char* id) = 0;
};