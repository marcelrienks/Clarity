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
 * @handlers PolledHandler for time-based evaluation, QueuedHandler for deferred execution
 */
class InterruptManager
{
public:
    // Singleton access
    static InterruptManager& Instance();
    
    // Core functionality
    void Init(IGpioProvider* gpioProvider = nullptr);
    void Process();
    
    // Pure interrupt registration system
    bool RegisterInterrupt(const Interrupt& interrupt);
    void UnregisterInterrupt(const char* id);
    void ActivateInterrupt(const char* id);
    void DeactivateInterrupt(const char* id);
    void UpdateInterruptContext(const char* id, void* context);
    void UpdateInterruptExecution(const char* id, void (*execute)(void*));
    
    // Button system - function injection
    void UpdateButtonInterrupts(void (*shortPressFunc)(void* context), 
                               void (*longPressFunc)(void* context), 
                               void* panelContext);
    
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
    
    // Handler access for context initialization
    class PolledHandler* GetPolledHandler() const { return polledHandler_.get(); }
    class QueuedHandler* GetQueuedHandler() const { return queuedHandler_.get(); }
    
    // Public effect execution for handlers
    void ExecuteEffect(const Interrupt& interrupt);
    
    // Public restoration checking for handlers
    void CheckRestoration();
    
private:
    InterruptManager() = default;
    ~InterruptManager() = default;
    InterruptManager(const InterruptManager&) = delete;
    InterruptManager& operator=(const InterruptManager&) = delete;
    
    // Interrupt processing - clean separation
    void EvaluateInterrupts();
    void ExecuteInterrupts();
    void ExecuteByEffect(const Interrupt& interrupt);
    
    // Handler coordination
    void ProcessHandlers();
    void UpdateHandlerContexts();
    
    // Interrupt flow methods
    void EvaluateQueuedInterrupts();
    void EvaluatePolledInterrupts();
    bool IsUIIdle() const;
    void ExecuteQueuedInterrupts();
    void ExecutePolledInterrupts();
    
    // Helper methods
    Interrupt* FindInterrupt(const char* id);
    
    // Effect-specific execution methods
    void LoadPanelFromInterrupt(const Interrupt& interrupt);
    void HandleRestoration();
    void ApplyThemeFromInterrupt(const Interrupt& interrupt);
    void ApplyPreferenceFromInterrupt(const Interrupt& interrupt);
    void ExecuteButtonAction(const Interrupt& interrupt);
    
    // Static storage for memory safety
    static constexpr size_t MAX_INTERRUPTS = 32;
    static constexpr size_t MAX_HANDLERS = 8;
    
    // Interrupt storage
    Interrupt interrupts_[MAX_INTERRUPTS];
    size_t interruptCount_ = 0;
    
    // Handler management
    std::vector<std::shared_ptr<IHandler>> handlers_;
    std::shared_ptr<class PolledHandler> polledHandler_;
    std::shared_ptr<class QueuedHandler> queuedHandler_;
    
    // System state
    bool initialized_ = false;
    unsigned long lastEvaluationTime_ = 0;
    
    // Performance monitoring
    mutable size_t totalEvaluations_ = 0;
    mutable size_t totalExecutions_ = 0;
    
    // Button function storage for dynamic dispatch
    void (*currentShortPressFunc_)(void* context) = nullptr;
    void (*currentLongPressFunc_)(void* context) = nullptr;
    void* currentPanelContext_ = nullptr;
    
    // GPIO provider reference
    IGpioProvider* gpioProvider_ = nullptr;
};