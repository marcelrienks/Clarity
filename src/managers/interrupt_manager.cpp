#include "managers/interrupt_manager.h"
#include "managers/error_manager.h"
#include <Arduino.h>
#include <algorithm>
#include <cstring>

#ifdef CLARITY_DEBUG
    #include "esp32-hal-log.h"
    #define LOG_TAG "InterruptManager"
#else
    #define log_v(...)
    #define log_d(...)
    #define log_w(...)
    #define log_i(...)
    #define log_e(...)
#endif

// Singleton implementation
InterruptManager& InterruptManager::Instance()
{
    static InterruptManager instance;
    return instance;
}

void InterruptManager::Init()
{
    log_v("Init() called");
    if (initialized_)
    {
        log_w("InterruptManager already initialized");
        return;
    }

    // Initialize new coordinated system
    interruptCount_ = 0;
    handlers_.clear();
    lastEvaluationTime_ = millis();
    
    // Initialize legacy compatibility
    triggerSources_.clear();
    actionSources_.clear();
    lastCheckTime_ = millis();
    checkCount_ = 0;

    initialized_ = true;
    log_i("InterruptManager initialized with coordinated interrupt system");
}

void InterruptManager::RegisterTriggerSource(IInterruptService *source)
{
    log_v("RegisterTriggerSource() called");
    if (!source)
    {
        log_w("Attempted to register null trigger source");
        return;
    }

    // Check if already registered
    auto it = std::find(triggerSources_.begin(), triggerSources_.end(), source);
    if (it != triggerSources_.end())
    {
        log_w("Trigger source already registered");
        return;
    }

    triggerSources_.push_back(source);

    log_d("Registered trigger source (total: %zu)", triggerSources_.size());
}

void InterruptManager::RegisterActionSource(IInterruptService *source)
{
    log_v("RegisterActionSource() called");
    if (!source)
    {
        log_w("Attempted to register null action source");
        return;
    }

    // Check if already registered
    auto it = std::find(actionSources_.begin(), actionSources_.end(), source);
    if (it != actionSources_.end())
    {
        log_w("Action source already registered");
        return;
    }

    actionSources_.push_back(source);

    log_d("Registered action source (total: %zu)", actionSources_.size());
}

void InterruptManager::UnregisterTriggerSource(IInterruptService *source)
{
    log_v("UnregisterTriggerSource() called");
    if (!source)
    {
        return;
    }

    auto it = std::find(triggerSources_.begin(), triggerSources_.end(), source);
    if (it != triggerSources_.end())
    {
        triggerSources_.erase(it);
        log_d("Unregistered trigger source (remaining: %zu)", triggerSources_.size());
    }
}

void InterruptManager::UnregisterActionSource(IInterruptService *source)
{
    log_v("UnregisterActionSource() called");
    if (!source)
    {
        return;
    }

    auto it = std::find(actionSources_.begin(), actionSources_.end(), source);
    if (it != actionSources_.end())
    {
        actionSources_.erase(it);
        log_d("Unregistered action source (remaining: %zu)", actionSources_.size());
    }
}

// New coordinated interrupt system methods
void InterruptManager::Process()
{
    log_v("Process() called");
    if (!initialized_)
    {
        return;
    }

    unsigned long currentTime = millis();
    
    // Evaluate coordinated interrupts at controlled intervals
    if (currentTime - lastEvaluationTime_ >= INTERRUPT_EVALUATION_INTERVAL_MS)
    {
        EvaluateInterrupts();
        lastEvaluationTime_ = currentTime;
    }
    
    // Process specialized handlers
    ProcessHandlers();
    
    // Process legacy sources for backward compatibility
    ProcessLegacySources();
}

bool InterruptManager::RegisterInterrupt(const Interrupt& interrupt)
{
    log_v("RegisterInterrupt() called for interrupt: %s", interrupt.id ? interrupt.id : "null");
    
    if (!interrupt.id || !interrupt.evaluationFunc || !interrupt.executionFunc)
    {
        log_e("Invalid interrupt registration - missing required fields");
        return false;
    }
    
    if (interruptCount_ >= MAX_INTERRUPTS)
    {
        log_e("Cannot register interrupt - maximum capacity reached (%d)", MAX_INTERRUPTS);
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "InterruptManager", 
                                           "Maximum interrupt capacity exceeded");
        return false;
    }
    
    // Check for duplicate ID
    if (FindInterrupt(interrupt.id) != nullptr)
    {
        log_w("Interrupt with ID '%s' already exists", interrupt.id);
        return false;
    }
    
    // Add interrupt to array
    interrupts_[interruptCount_] = interrupt;
    interrupts_[interruptCount_].active = true;
    interrupts_[interruptCount_].lastEvaluation = 0;
    interruptCount_++;
    
    log_d("Registered interrupt '%s' (total: %d)", interrupt.id, interruptCount_);
    return true;
}

void InterruptManager::UnregisterInterrupt(const char* id)
{
    log_v("UnregisterInterrupt() called for: %s", id ? id : "null");
    
    if (!id) return;
    
    for (size_t i = 0; i < interruptCount_; ++i)
    {
        if (interrupts_[i].id && strcmp(interrupts_[i].id, id) == 0)
        {
            // Move last interrupt to this position to avoid gaps
            if (i < interruptCount_ - 1)
            {
                interrupts_[i] = interrupts_[interruptCount_ - 1];
            }
            interruptCount_--;
            log_d("Unregistered interrupt '%s' (remaining: %d)", id, interruptCount_);
            return;
        }
    }
    
    log_w("Interrupt '%s' not found for unregistration", id);
}

void InterruptManager::ActivateInterrupt(const char* id)
{
    log_v("ActivateInterrupt() called for: %s", id ? id : "null");
    
    Interrupt* interrupt = FindInterrupt(id);
    if (interrupt)
    {
        interrupt->active = true;
        log_d("Activated interrupt '%s'", id);
    }
    else
    {
        log_w("Interrupt '%s' not found for activation", id ? id : "null");
    }
}

void InterruptManager::DeactivateInterrupt(const char* id)
{
    log_v("DeactivateInterrupt() called for: %s", id ? id : "null");
    
    Interrupt* interrupt = FindInterrupt(id);
    if (interrupt)
    {
        interrupt->active = false;
        log_d("Deactivated interrupt '%s'", id);
    }
    else
    {
        log_w("Interrupt '%s' not found for deactivation", id ? id : "null");
    }
}

void InterruptManager::RegisterHandler(std::shared_ptr<IHandler> handler)
{
    log_v("RegisterHandler() called");
    
    if (!handler)
    {
        log_w("Attempted to register null handler");
        return;
    }
    
    if (handlers_.size() >= MAX_HANDLERS)
    {
        log_e("Cannot register handler - maximum capacity reached (%d)", MAX_HANDLERS);
        return;
    }
    
    handlers_.push_back(handler);
    log_d("Registered handler (total: %d)", handlers_.size());
}

void InterruptManager::UnregisterHandler(std::shared_ptr<IHandler> handler)
{
    log_v("UnregisterHandler() called");
    
    if (!handler) return;
    
    auto it = std::find(handlers_.begin(), handlers_.end(), handler);
    if (it != handlers_.end())
    {
        handlers_.erase(it);
        log_d("Unregistered handler (remaining: %d)", handlers_.size());
    }
}

bool InterruptManager::HasActiveInterrupts() const
{
    for (size_t i = 0; i < interruptCount_; ++i)
    {
        if (interrupts_[i].active)
        {
            return true;
        }
    }
    return false;
}

size_t InterruptManager::GetInterruptCount() const
{
    return interruptCount_;
}

// Private implementation methods
void InterruptManager::EvaluateInterrupts()
{
    log_v("EvaluateInterrupts() called");
    
    // Sort by priority (CRITICAL = 0, IMPORTANT = 1, NORMAL = 2)
    // Process in priority order
    for (int priority = 0; priority <= 2; ++priority)
    {
        for (size_t i = 0; i < interruptCount_; ++i)
        {
            Interrupt& interrupt = interrupts_[i];
            
            if (!interrupt.active || static_cast<int>(interrupt.priority) != priority)
                continue;
                
            if (ShouldEvaluateInterrupt(interrupt))
            {
                if (interrupt.evaluationFunc(interrupt.context))
                {
                    ExecuteInterrupt(interrupt);
                    // High priority interrupts can preempt lower ones
                    if (interrupt.priority == Priority::CRITICAL)
                    {
                        return;
                    }
                }
                UpdateLastEvaluation(interrupt);
            }
        }
    }
}

void InterruptManager::ExecuteInterrupt(Interrupt& interrupt)
{
    log_v("ExecuteInterrupt() called for: %s", interrupt.id ? interrupt.id : "unknown");
    
    if (interrupt.executionFunc && interrupt.context)
    {
        log_d("Executing interrupt '%s' with effect %d", interrupt.id, static_cast<int>(interrupt.effect));
        interrupt.executionFunc(interrupt.context);
    }
}

void InterruptManager::ProcessHandlers()
{
    log_v("ProcessHandlers() called");
    
    for (auto& handler : handlers_)
    {
        if (handler)
        {
            handler->Process();
        }
    }
}

void InterruptManager::ProcessLegacySources()
{
    log_v("ProcessLegacySources() called");
    
    checkCount_++;
    unsigned long currentTime = millis();

    // Process all trigger sources directly
    for (IInterruptService *source : triggerSources_)
    {
        if (source)
        {
            source->Process();
        }
    }

    // Process all action sources directly
    for (IInterruptService *source : actionSources_)
    {
        if (source)
        {
            source->Process();
        }
    }

    lastCheckTime_ = currentTime;
}

Interrupt* InterruptManager::FindInterrupt(const char* id)
{
    if (!id) return nullptr;
    
    for (size_t i = 0; i < interruptCount_; ++i)
    {
        if (interrupts_[i].id && strcmp(interrupts_[i].id, id) == 0)
        {
            return &interrupts_[i];
        }
    }
    return nullptr;
}

bool InterruptManager::ShouldEvaluateInterrupt(const Interrupt& interrupt) const
{
    // For now, evaluate all active interrupts
    // In future phases, this can be optimized with timing intervals
    return true;
}

void InterruptManager::UpdateLastEvaluation(Interrupt& interrupt)
{
    interrupt.lastEvaluation = millis();
}

// Legacy compatibility methods
void InterruptManager::CheckAllInterrupts()
{
    log_v("CheckAllInterrupts() called (legacy compatibility)");
    Process(); // Delegate to new coordinated system
}

