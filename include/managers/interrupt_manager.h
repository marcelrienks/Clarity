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
    void UnregisterTrigger(const char* id);
    bool RegisterAction(const Action& action);
    void UnregisterAction(const char* id);
    void UpdatePanelFunctions(void (*shortPressFunc)(void*), void (*longPressFunc)(void*), void* context);
    void RegisterHandler(std::shared_ptr<IHandler> handler);
    void UnregisterHandler(std::shared_ptr<IHandler> handler);
    size_t GetRegisteredInterruptCount() const;
    void GetInterruptStatistics(size_t& totalEvaluations, size_t& totalExecutions) const;
    void OptimizeMemoryUsage();
    void CompactInterruptArray();
    bool HasActiveInterrupts() const;
    size_t GetInterruptCount() const;
    class TriggerHandler* GetTriggerHandler() const { return triggerHandler_.get(); }
    class ActionHandler* GetActionHandler() const { return actionHandler_.get(); }
    void CheckRestoration();
    bool CheckAndExecuteHighestPriorityTrigger();
    void CheckAndExecuteActiveStyleTriggers();

private:
    // ========== Constructors and Destructor ==========
    InterruptManager() = default;

    // ========== Private Methods ==========
    void RegisterSystemInterrupts();
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

    // Performance monitoring
    mutable size_t totalEvaluations_ = 0;
    mutable size_t totalExecutions_ = 0;

    // GPIO provider reference
    IGpioProvider* gpioProvider_ = nullptr;
};