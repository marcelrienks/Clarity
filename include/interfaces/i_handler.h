#pragma once

#include "definitions/types.h"

/**
 * @interface IHandler
 * @brief Base interface for new Trigger/Action interrupt handlers
 *
 * @details This interface defines the contract for specialized interrupt handlers
 * in the new Trigger/Action architecture. Each handler is responsible for processing
 * its own type of interrupts (Triggers or Actions).
 *
 * @implementations:
 * - TriggerHandler: State-based GPIO triggers with dual functions
 * - ActionHandler: Event-based button actions with timing detection
 *
 * @architecture Simplified processing model:
 * - Each handler owns its sensors exclusively
 * - Handlers process only during appropriate timing (idle for triggers, always for actions)
 * - Clear separation between state-based and event-based processing
 */
class IHandler
{
public:
    virtual ~IHandler() = default;

    /**
     * @brief Process the handler's interrupts
     * @details Each handler implements its own processing logic:
     * - TriggerHandler: Evaluate GPIO state changes during UI idle
     * - ActionHandler: Evaluate button events continuously, execute during idle
     */
    virtual void Process() = 0;
};