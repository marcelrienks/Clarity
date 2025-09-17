#pragma once

#include "interfaces/i_handler.h"
#include "interfaces/i_gpio_provider.h"
#include "utilities/types.h"
#include "utilities/constants.h"
#include <vector>
#include <memory>
#include <functional>

#include "esp32-hal-log.h"

/**
 * @class InterruptManager
 * @brief Pure coordinated interrupt system for ESP32 automotive applications
 * 
 * @details Complete event-driven interrupt coordination replacing all polling-based
 * architecture. Manages interrupt registration, evaluation, and execution with 
 * priority-based processing and optimized evaluation intervals.
 * 
 * @architecture Singleton pattern with static memory allocation for ESP32 safety
 * @memory_optimization Static interrupt array (32 max), 29 bytes per interrupt
 * @performance Smart evaluation intervals: Critical 10ms, Important 25ms, Normal 50ms
 * @handlers TriggerHandler for state-based evaluation, ActionHandler for event-based execution
 */
class InterruptManager
{
public:
    // ========== Constructors and Destructor ==========
    InterruptManager(const InterruptManager&) = delete;
    InterruptManager& operator=(const InterruptManager&) = delete;
    ~InterruptManager() = default;

    // ========== Static Methods ==========
    static InterruptManager& Instance();

    // ========== Public Interface Methods ==========
    void Init(IGpioProvider* gpioProvider = nullptr);
    void Process();
    bool RegisterTrigger(const Trigger& trigger);
    bool RegisterAction(const Action& action);
    void UpdatePanelFunctions(void (*shortPressFunc)(void*), void (*longPressFunc)(void*), void* context);
    void RegisterHandler(std::shared_ptr<IHandler> handler);

    /**
     * @brief Get total count of registered interrupts (triggers + actions)
     * @return Total number of registered interrupts
     */
    size_t GetRegisteredInterruptCount() const;

    /**
     * @brief Check if any triggers or actions are currently active
     * @return true if active interrupts exist, false otherwise
     */
    bool HasActiveInterrupts() const;

    /**
     * @brief Get direct access to trigger handler
     * @return Pointer to TriggerHandler instance
     */
    class TriggerHandler* GetTriggerHandler() const { return triggerHandler_.get(); }

    /**
     * @brief Get direct access to action handler
     * @return Pointer to ActionHandler instance
     */
    class ActionHandler* GetActionHandler() const { return actionHandler_.get(); }

    /**
     * @brief Check if panel restoration is needed after interrupt deactivation
     * @details Coordinates with PanelManager for seamless panel restoration
     */
    void CheckRestoration();

    /**
     * @brief Find and execute highest priority PANEL trigger
     * @return true if trigger was found and executed, false otherwise
     */
    bool CheckAndExecuteHighestPriorityTrigger();

    /**
     * @brief Find and execute highest priority STYLE trigger
     * @details Handles theme and styling triggers separately from panel triggers
     */
    void CheckAndExecuteActiveStyleTriggers();

private:
    // ========== Constructors and Destructor ==========
    InterruptManager() = default;

    // ========== Private Methods ==========
    /**
     * @brief Register all system-level triggers and actions
     * @details Creates sensors and registers standard system interrupts
     */
    void RegisterSystemInterrupts();

    /**
     * @brief Check if UI is idle for trigger processing optimization
     * @return true if UI has been idle for sufficient time
     * @details Uses cached result with 5ms timeout to reduce LVGL query overhead
     */
    bool IsUIIdle() const;

    // ========== Private Data Members ==========
    static constexpr size_t MAX_HANDLERS = 8;

    // Handler management - Legacy and new handlers
    std::vector<std::shared_ptr<IHandler>> handlers_;

    // New Trigger/Action architecture handlers
    std::shared_ptr<class TriggerHandler> triggerHandler_;
    std::shared_ptr<class ActionHandler> actionHandler_;

    // System state
    bool initialized_ = false;
    unsigned long lastEvaluationTime_ = 0;

    // GPIO provider reference
    IGpioProvider* gpioProvider_ = nullptr;
};