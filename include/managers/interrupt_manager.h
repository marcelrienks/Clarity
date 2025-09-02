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
    // Singleton access
    static InterruptManager& Instance();
    
    // Core functionality
    void Init(IGpioProvider* gpioProvider = nullptr);
    void Process();
    
    
    // New Trigger/Action system interface
    bool RegisterTrigger(const Trigger& trigger);
    void UnregisterTrigger(const char* id);
    bool RegisterAction(const Action& action);
    void UnregisterAction(const char* id);
    
    // Panel function injection for new ActionHandler
    void UpdatePanelFunctions(void (*shortPressFunc)(void*), void (*longPressFunc)(void*), void* context);
    
    // Handler registration for specialized processing
    void RegisterHandler(std::shared_ptr<IHandler> handler);
    void UnregisterHandler(std::shared_ptr<IHandler> handler);
    
    // System monitoring and diagnostics
    void PrintSystemStatus() const;
    size_t GetRegisteredInterruptCount() const;
    void GetInterruptStatistics(size_t& totalEvaluations, size_t& totalExecutions) const;
    
    // Performance optimization
    void OptimizeMemoryUsage();
    void CompactInterruptArray();
    
    // Status queries
    bool HasActiveInterrupts() const;
    size_t GetInterruptCount() const;
    
    
    // New handler access for Trigger/Action architecture
    class TriggerHandler* GetTriggerHandler() const { return triggerHandler_.get(); }
    class ActionHandler* GetActionHandler() const { return actionHandler_.get(); }
    
    // Public restoration checking for handlers
    void CheckRestoration();
    
private:
    InterruptManager() = default;
    ~InterruptManager() = default;
    InterruptManager(const InterruptManager&) = delete;
    InterruptManager& operator=(const InterruptManager&) = delete;
    
    
    // Internal system registration
    void RegisterSystemInterrupts();
    
    // Helper methods
    bool IsUIIdle() const;
    
    // Static storage for memory safety
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