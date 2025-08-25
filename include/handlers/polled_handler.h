#pragma once

#include "interfaces/i_handler.h"
#include "utilities/types.h"
#include <vector>

#include "esp32-hal-log.h"

/**
 * @class PolledHandler
 * @brief Handles polled interrupts with timing-based evaluation
 * 
 * @details Processes interrupts that need periodic evaluation at controlled intervals.
 * Optimized for sensors, button debouncing, and other time-sensitive operations.
 * 
 * @timing Configurable evaluation intervals per interrupt
 * @memory_safe Uses references to external interrupt array (no duplication)
 * @performance Skips evaluation if minimum interval hasn't elapsed
 */
class PolledHandler : public IHandler
{
public:
    PolledHandler();
    ~PolledHandler() = default;
    
    // IHandler interface
    void Process() override;
    
    // Configuration
    void RegisterInterrupt(const Interrupt* interrupt);
    void UnregisterInterrupt(const char* id);
    void SetEvaluationInterval(unsigned long intervalMs);
    
    // Status
    size_t GetInterruptCount() const;
    bool HasPendingEvaluations() const;
    
private:
    struct PolledInterruptRef
    {
        const Interrupt* interrupt;
        unsigned long lastEvaluation;
        unsigned long evaluationInterval;
        
        PolledInterruptRef(const Interrupt* intr, unsigned long interval = 100) 
            : interrupt(intr), lastEvaluation(0), evaluationInterval(interval) {}
    };
    
    void EvaluateInterrupt(PolledInterruptRef& ref);
    bool ShouldEvaluate(const PolledInterruptRef& ref) const;
    
    static constexpr size_t MAX_POLLED_INTERRUPTS = 16;
    static constexpr unsigned long DEFAULT_EVALUATION_INTERVAL_MS = 100;
    
    std::vector<PolledInterruptRef> polledInterrupts_;
    unsigned long defaultInterval_;
};