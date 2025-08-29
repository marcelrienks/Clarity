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
    
    // Universal button system - function injection
    void UpdateInterruptFunction(const char* id, void (*newFunc)(void* context));
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
    
    // Coordinated interrupt processing
    void ExecuteInterrupt(Interrupt& interrupt);
    void ExecuteByEffect(const Interrupt& interrupt);
    void ProcessHandlers();
    void UpdateHandlerContexts();
    
    // Interrupt flow methods
    void EvaluateQueuedInterrupts();
    void PostQueuedInterrupts();
    bool IsUIIdle() const;
    void EvaluateAndActionPolledInterrupts();
    void ProcessQueuedInterruptActions();
    
    // Legacy single-list architecture methods (to be removed)
    void EvaluateAllInterrupts();
    void ExecuteInterruptsWithRules();
    bool CanExecute(const Interrupt& interrupt) const;
    void ClearStateChanges();
    bool IsGroupExecuted(const char* group) const;
    
    // Helper methods
    Interrupt* FindInterrupt(const char* id);
    bool ShouldEvaluateInterrupt(const Interrupt& interrupt) const;
    void UpdateLastEvaluation(Interrupt& interrupt);
    
    // Context management
    void SetCurrentContext(void* context) { currentContext_ = context; }
    
    // Effect-specific execution methods
    void LoadPanelFromInterrupt(const Interrupt& interrupt);
    void CheckForRestoration(const Interrupt& interrupt);
    void HandleRestoration();
    void ApplyThemeFromInterrupt(const Interrupt& interrupt);
    void ApplyPreferenceFromInterrupt(const Interrupt& interrupt);
    void ExecuteButtonAction(const Interrupt& interrupt);
    
    // Static storage for memory safety
    static constexpr size_t MAX_INTERRUPTS = 32;
    static constexpr size_t MAX_HANDLERS = 8;
    static constexpr unsigned long INTERRUPT_EVALUATION_INTERVAL_MS = 50;
    
    // Coordinated interrupt system
    Interrupt interrupts_[MAX_INTERRUPTS];
    size_t interruptCount_ = 0;
    std::vector<std::shared_ptr<IHandler>> handlers_;
    
    // Direct references to default handlers for routing
    std::shared_ptr<class PolledHandler> polledHandler_;
    std::shared_ptr<class QueuedHandler> queuedHandler_;
    
    // Exclusion group tracking for current cycle
    std::vector<const char*> executedGroups_;
    
    // Current context for conditional execution checks
    void* currentContext_ = nullptr;
    
    unsigned long lastEvaluationTime_ = 0;
    bool initialized_ = false;
    
    // Statistics (for debugging and monitoring)
    mutable unsigned long lastCheckTime_ = 0;
    mutable unsigned long checkCount_ = 0;
    
    // Phase 6: Enhanced performance monitoring
    mutable size_t totalEvaluations_ = 0;
    mutable size_t totalExecutions_ = 0;
    mutable unsigned long lastDiagnosticTime_ = 0;
    
    // Universal button function injection storage
    void (*currentShortPressFunc_)(void* context) = nullptr;
    void (*currentLongPressFunc_)(void* context) = nullptr;
    void* currentPanelContext_ = nullptr;
    
    // GPIO provider reference (stored to avoid circular dependency during initialization)
    IGpioProvider* gpioProvider_ = nullptr;
    
    // Flow state tracking
    bool queuedInterruptsNeedProcessing_ = false;
    unsigned long buttonPressStartTime_ = 0;
    bool buttonCurrentlyPressed_ = false;
    bool isLongPress_ = false; // Track if the queued press is long or short
};