#pragma once

#include "interfaces/i_interrupt.h"
#include <vector>
#include <memory>

/**
 * @class InterruptManager
 * @brief Centralized manager for all interrupt sources with priority handling
 * 
 * @details This class coordinates checking of all interrupt sources (triggers, inputs, etc.)
 * during idle time. It maintains a priority-ordered list of interrupt sources and
 * provides efficient checking to minimize overhead during animations and operations.
 * 
 * @priority_system Higher priority sources are checked first
 * @idle_integration Designed to be called during LVGL idle time and animation gaps
 * @performance_optimized Skips sources that report no pending interrupts
 */
class InterruptManager
{
public:
    // Constructors and Destructors
    InterruptManager();
    ~InterruptManager() = default;
    
    // Disable copy/move to maintain singleton-like behavior
    InterruptManager(const InterruptManager&) = delete;
    InterruptManager& operator=(const InterruptManager&) = delete;

    /**
     * @brief Initialize the interrupt manager
     * @details Sets up any necessary resources and prepares for interrupt checking
     */
    void Init();

    /**
     * @brief Register an interrupt source for periodic checking
     * @param source Pointer to interrupt source (must remain valid)
     * @details Sources are automatically sorted by priority after registration
     */
    void RegisterInterruptSource(IInterrupt* source);

    /**
     * @brief Unregister an interrupt source
     * @param source Pointer to interrupt source to remove
     */
    void UnregisterInterruptSource(IInterrupt* source);

    /**
     * @brief Check all registered interrupt sources in priority order
     * @details This is the main method called during idle time. It efficiently
     * checks only sources that report pending interrupts.
     */
    void CheckAllInterrupts();

    /**
     * @brief Check if any interrupt sources have pending work
     * @details Quick check without processing - useful for optimization
     * @return true if any source has pending interrupts
     */
    bool HasAnyPendingInterrupts() const;

    /**
     * @brief Get the number of registered interrupt sources
     * @return Count of registered sources
     */
    size_t GetSourceCount() const { return interruptSources_.size(); }

private:
    // Internal methods
    void SortSourcesByPriority();
    
    // Data members
    std::vector<IInterrupt*> interruptSources_;
    bool initialized_;
    
    // Statistics (for debugging)
    mutable unsigned long lastCheckTime_;
    mutable unsigned long checkCount_;
};