#pragma once

#include "interfaces/i_interrupt_service.h"
#include "interfaces/i_panel_service.h"
#include <vector>
#include <memory>

/**
 * @class InterruptManager
 * @brief Centralized manager for interrupt sources with ordered evaluation
 * 
 * @details This class coordinates checking of interrupt sources (triggers, inputs, etc.)
 * during idle time. It evaluates triggers first, then actions only if no triggers
 * are active, ensuring proper precedence in the interrupt handling system.
 * 
 * @evaluation_order Triggers first, then actions if no triggers active
 * @idle_integration Designed to be called during LVGL idle time and animation gaps
 * @performance_optimized Skips sources that report no pending interrupts
 */
class InterruptManager
{
public:
    // Constructors and Destructors
    InterruptManager(IPanelService* panelService = nullptr);
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
     * @brief Register a trigger interrupt source
     * @param source Pointer to trigger source (must remain valid)
     * @details Triggers are evaluated before actions
     */
    void RegisterTriggerSource(IInterruptService* source);
    
    /**
     * @brief Register an action interrupt source
     * @param source Pointer to action source (must remain valid)
     * @details Actions are evaluated only if no triggers are active
     */
    void RegisterActionSource(IInterruptService* source);

    /**
     * @brief Unregister a trigger interrupt source
     * @param source Pointer to trigger source to remove
     */
    void UnregisterTriggerSource(IInterruptService* source);
    
    /**
     * @brief Unregister an action interrupt source
     * @param source Pointer to action source to remove
     */
    void UnregisterActionSource(IInterruptService* source);

    /**
     * @brief Check interrupt sources in ordered evaluation
     * @details Checks triggers first, then actions only if no triggers active.
     * This is the main method called during idle time.
     */
    void CheckAllInterrupts();

    /**
     * @brief Check if any interrupt sources have pending work
     * @details Quick check without processing - useful for optimization
     * @return true if any source has pending interrupts
     */
    bool HasAnyPendingInterrupts() const;

    /**
     * @brief Get the number of registered trigger sources
     * @return Count of registered trigger sources
     */
    size_t GetTriggerSourceCount() const { return triggerSources_.size(); }
    
    /**
     * @brief Get the number of registered action sources
     * @return Count of registered action sources
     */
    size_t GetActionSourceCount() const { return actionSources_.size(); }

private:
    // Data members
    std::vector<IInterruptService*> triggerSources_;
    std::vector<IInterruptService*> actionSources_;
    IPanelService* panelService_;
    bool initialized_;
    
    // Statistics (for debugging)
    mutable unsigned long lastCheckTime_;
    mutable unsigned long checkCount_;
};