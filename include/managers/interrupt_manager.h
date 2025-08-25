#pragma once

#include "interfaces/i_handler.h"
#include "interfaces/i_interrupt_service.h"
#include "interfaces/i_panel_service.h"
#include "utilities/types.h"
#include "utilities/constants.h"
#include <vector>
#include <memory>
#include <functional>

#ifdef CLARITY_DEBUG
    #include "esp32-hal-log.h"
    #ifndef LOG_TAG
        #define LOG_TAG "InterruptManager"
    #endif
#else
    #define log_v(...)
    #define log_d(...)
    #define log_w(...)
    #define log_e(...)
#endif

/**
 * @class InterruptManager
 * @brief Centralized interrupt coordination system
 * 
 * @details Manages interrupt registration, evaluation, and execution with priority-based
 * processing. Coordinates between polled and queued interrupt sources to prevent
 * conflicts and ensure deterministic behavior.
 * 
 * @architecture Singleton pattern with static registration for memory safety
 * @memory_optimization Static interrupt array with fixed capacity (32 interrupts max)
 * @timing Evaluation rate controlled by INTERRUPT_EVALUATION_INTERVAL_MS (50ms)
 */
class InterruptManager
{
public:
    // Singleton access
    static InterruptManager& Instance();
    
    // Core functionality
    void Init();
    void Process();
    
    // Interrupt registration (new coordinated system)
    bool RegisterInterrupt(const Interrupt& interrupt);
    void UnregisterInterrupt(const char* id);
    void ActivateInterrupt(const char* id);
    void DeactivateInterrupt(const char* id);
    
    // Handler registration (for specialized processing)
    void RegisterHandler(std::shared_ptr<IHandler> handler);
    void UnregisterHandler(std::shared_ptr<IHandler> handler);
    
    // Legacy compatibility methods (Phase 2: maintain existing interface)
    void RegisterTriggerSource(IInterruptService *source);
    void RegisterActionSource(IInterruptService *source);
    void UnregisterTriggerSource(IInterruptService *source);
    void UnregisterActionSource(IInterruptService *source);
    void CheckAllInterrupts();
    
    // Status queries
    bool HasActiveInterrupts() const;
    size_t GetInterruptCount() const;
    size_t GetTriggerSourceCount() const { return triggerSources_.size(); }
    size_t GetActionSourceCount() const { return actionSources_.size(); }
    
private:
    InterruptManager() = default;
    ~InterruptManager() = default;
    InterruptManager(const InterruptManager&) = delete;
    InterruptManager& operator=(const InterruptManager&) = delete;
    
    // New coordinated interrupt processing
    void EvaluateInterrupts();
    void ExecuteInterrupt(Interrupt& interrupt);
    void ProcessHandlers();
    
    // Legacy processing
    void ProcessLegacySources();
    
    // Helper methods
    Interrupt* FindInterrupt(const char* id);
    bool ShouldEvaluateInterrupt(const Interrupt& interrupt) const;
    void UpdateLastEvaluation(Interrupt& interrupt);
    
    // Static storage for memory safety
    static constexpr size_t MAX_INTERRUPTS = 32;
    static constexpr size_t MAX_HANDLERS = 8;
    static constexpr unsigned long INTERRUPT_EVALUATION_INTERVAL_MS = 50;
    
    // New coordinated interrupt system
    Interrupt interrupts_[MAX_INTERRUPTS];
    size_t interruptCount_ = 0;
    std::vector<std::shared_ptr<IHandler>> handlers_;
    
    // Direct references to default handlers for routing
    std::shared_ptr<class PolledHandler> polledHandler_;
    std::shared_ptr<class QueuedHandler> queuedHandler_;
    
    // Legacy compatibility
    std::vector<IInterruptService *> triggerSources_;
    std::vector<IInterruptService *> actionSources_;
    IPanelService *panelService_ = nullptr;
    
    unsigned long lastEvaluationTime_ = 0;
    bool initialized_ = false;
    
    // Statistics (for debugging)
    mutable unsigned long lastCheckTime_ = 0;
    mutable unsigned long checkCount_ = 0;
};