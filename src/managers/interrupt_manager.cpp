#include "managers/interrupt_manager.h"
#include "managers/error_manager.h"
#include "handlers/polled_handler.h"
#include "handlers/queued_handler.h"
#include <Arduino.h>
#include <cstring>

#include "esp32-hal-log.h"

// Singleton implementation
InterruptManager& InterruptManager::Instance()
{
    static InterruptManager instance;
    return instance;
}

void InterruptManager::Init(IGpioProvider* gpioProvider)
{
    log_v("Init() called");
    if (initialized_)
    {
        log_w("InterruptManager already initialized");
        return;
    }

    // Initialize interrupt storage and timing
    interruptCount_ = 0;
    handlers_.clear();
    lastEvaluationTime_ = millis();
    lastCheckTime_ = millis();
    checkCount_ = 0;

    // Create and register default handlers with GPIO provider
    if (gpioProvider) {
        auto polledHandler = std::make_shared<PolledHandler>(gpioProvider);
        auto queuedHandler = std::make_shared<QueuedHandler>(gpioProvider);
        
        if (polledHandler && queuedHandler)
        {
            RegisterHandler(polledHandler);
            RegisterHandler(queuedHandler);
            
            // Store references for direct access
            polledHandler_ = polledHandler;
            queuedHandler_ = queuedHandler;
            
            log_d("Registered default PolledHandler and QueuedHandler with GPIO provider");
        }
        else
        {
            log_e("Failed to create default handlers with GPIO provider");
            ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "InterruptManager", 
                                               "Failed to create default interrupt handlers");
        }
    } else {
        log_e("Cannot create handlers - GPIO provider is null");
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "InterruptManager", 
                                           "Cannot create handlers - GPIO provider is null");
    }

    initialized_ = true;
    log_i("InterruptManager initialized with polled and queued interrupt handlers");
    
    // Update interrupt contexts now that handlers have created sensors
    UpdateHandlerContexts();
}

void InterruptManager::UpdateHandlerContexts()
{
    log_d("Updating interrupt contexts with handler-owned sensors");
    
    if (polledHandler_) {
        // Set contexts to the actual sensor instances owned by handlers
        UpdateInterruptContext("key_present", polledHandler_->GetKeyPresentSensor());
        UpdateInterruptContext("key_not_present", polledHandler_->GetKeyNotPresentSensor());
        UpdateInterruptContext("lock_state", polledHandler_->GetLockSensor());
        UpdateInterruptContext("lights_state", polledHandler_->GetLightsSensor());
        UpdateInterruptContext("error_occurred", &ErrorManager::Instance());
        log_d("Updated polled interrupt contexts with actual sensor pointers");
    }
    
    if (queuedHandler_) {
        UpdateInterruptContext("universal_short_press", queuedHandler_->GetActionButtonSensor());
        UpdateInterruptContext("universal_long_press", queuedHandler_->GetActionButtonSensor());
        log_d("Updated queued interrupt contexts with ActionButtonSensor pointer");
    }
}


// Core interrupt processing methods
void InterruptManager::Process()
{
    log_v("Process() called");
    if (!initialized_)
    {
        return;
    }

    unsigned long currentTime = millis();
    
    // Control evaluation frequency to prevent CPU overload
    if (currentTime - lastEvaluationTime_ >= INTERRUPT_EVALUATION_INTERVAL_MS)
    {
        EvaluateInterrupts();
        lastEvaluationTime_ = currentTime;
    }
    
    // Execute queued interrupt actions through registered handlers  
    ProcessHandlers();
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
    
    // Route interrupt to appropriate handler
    Interrupt* registeredInterrupt = &interrupts_[interruptCount_];
    interruptCount_++;
    
    if (interrupt.source == InterruptSource::POLLED && polledHandler_)
    {
        polledHandler_->RegisterInterrupt(registeredInterrupt);
        log_d("Routed polled interrupt '%s' to PolledHandler", interrupt.id);
    }
    else if (interrupt.source == InterruptSource::QUEUED && queuedHandler_)
    {
        // Queued interrupts are handled differently - they get queued when triggered
        log_d("Registered queued interrupt '%s' for QueuedHandler processing", interrupt.id);
    }
    
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
            // Unregister from handlers based on source type
            if (interrupts_[i].source == InterruptSource::POLLED && polledHandler_)
            {
                polledHandler_->UnregisterInterrupt(id);
                log_d("Unregistered polled interrupt '%s' from PolledHandler", id);
            }
            // Queued interrupts don't need explicit handler unregistration
            
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

void InterruptManager::UpdateInterruptContext(const char* id, void* context)
{
    log_v("UpdateInterruptContext() called for: %s", id ? id : "null");
    
    Interrupt* interrupt = FindInterrupt(id);
    if (interrupt)
    {
        interrupt->context = context;
        log_d("Updated context for interrupt '%s'", id);
    }
    else
    {
        log_w("Interrupt '%s' not found for context update", id ? id : "null");
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
                ++totalEvaluations_; // Track evaluation count
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
    ++totalExecutions_; // Track execution count
    
    // Handle queued interrupts by routing to QueuedHandler
    if (interrupt.source == InterruptSource::QUEUED && queuedHandler_)
    {
        if (queuedHandler_->QueueInterrupt(&interrupt))
        {
            log_d("Queued interrupt '%s' for deferred execution", interrupt.id);
        }
        else
        {
            log_w("Failed to queue interrupt '%s' - executing immediately", interrupt.id);
            // Fallback to immediate execution if queueing fails
            if (interrupt.executionFunc && interrupt.context)
            {
                interrupt.executionFunc(interrupt.context);
            }
        }
        return;
    }
    
    // Handle immediate execution (polled interrupts)
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
    // Optimize evaluation frequency based on priority to reduce CPU usage
    unsigned long currentTime = millis();
    unsigned long timeSinceLastEvaluation = currentTime - interrupt.lastEvaluation;
    
    // Set evaluation intervals based on priority for CPU efficiency
    unsigned long minInterval = 0;
    
    switch (interrupt.priority)
    {
        case Priority::CRITICAL:
            minInterval = 10;  // Fast response for error conditions and security
            break;
        case Priority::IMPORTANT:
            minInterval = 25;  // Balanced response for user input and sensors
            break;
        case Priority::NORMAL:
            minInterval = 50;  // Slower response for background tasks and themes
            break;
    }
    
    // User input requires fast response for good UX
    if (interrupt.effect == InterruptEffect::BUTTON_ACTION)
    {
        minInterval = std::min(minInterval, 15UL); // Ensure responsive button handling
    }
    
    // UI changes can be evaluated less frequently to prevent flicker
    if (interrupt.effect == InterruptEffect::SET_THEME)
    {
        minInterval = std::max(minInterval, 100UL); // Prevent rapid theme switching
    }
    
    return timeSinceLastEvaluation >= minInterval;
}

void InterruptManager::UpdateLastEvaluation(Interrupt& interrupt)
{
    interrupt.lastEvaluation = millis();
}


// System monitoring and diagnostic methods
void InterruptManager::PrintSystemStatus() const
{
    log_i("=== Interrupt System Status ===");
    log_i("Total Registered Interrupts: %d/%d", interruptCount_, MAX_INTERRUPTS);
    log_i("Total Handlers: %d/%d", handlers_.size(), MAX_HANDLERS);
    log_i("Total Evaluations: %lu", totalEvaluations_);
    log_i("Total Executions: %lu", totalExecutions_);
    log_i("Last Evaluation Time: %lu ms", lastEvaluationTime_);
    
    log_i("--- Registered Interrupts ---");
    for (size_t i = 0; i < interruptCount_; ++i)
    {
        const Interrupt& interrupt = interrupts_[i];
        const char* priorityStr = interrupt.priority == Priority::CRITICAL ? "CRITICAL" :
                                 interrupt.priority == Priority::IMPORTANT ? "IMPORTANT" : "NORMAL";
        const char* sourceStr = interrupt.source == InterruptSource::POLLED ? "POLLED" : "QUEUED";
        const char* effectStr = interrupt.effect == InterruptEffect::LOAD_PANEL ? "LOAD_PANEL" :
                               interrupt.effect == InterruptEffect::SET_THEME ? "SET_THEME" :
                               interrupt.effect == InterruptEffect::SET_PREFERENCE ? "SET_PREFERENCE" : "BUTTON_ACTION";
        
        log_i("  [%d] %s: %s/%s/%s %s", 
              i, interrupt.id ? interrupt.id : "null", 
              priorityStr, sourceStr, effectStr,
              interrupt.active ? "ACTIVE" : "INACTIVE");
    }
    
    log_i("--- Handler Status ---");
    log_i("  Polled Handler: %s", polledHandler_ ? "REGISTERED" : "NOT REGISTERED");
    log_i("  Queued Handler: %s", queuedHandler_ ? "REGISTERED" : "NOT REGISTERED");
    log_i("  Total Handlers: %d", handlers_.size());
    
    log_i("================================");
}

size_t InterruptManager::GetRegisteredInterruptCount() const
{
    return interruptCount_;
}

void InterruptManager::GetInterruptStatistics(size_t& totalEvaluations, size_t& totalExecutions) const
{
    totalEvaluations = totalEvaluations_;
    totalExecutions = totalExecutions_;
}

// Memory management and performance optimization
void InterruptManager::OptimizeMemoryUsage()
{
    log_v("OptimizeMemoryUsage() called");
    
    // Remove inactive interrupts to free memory
    CompactInterruptArray();
    
    // Report optimization results for debugging
    log_d("Memory optimization complete - %d interrupts active", interruptCount_);
}

void InterruptManager::CompactInterruptArray()
{
    log_v("CompactInterruptArray() called");
    
    size_t writeIndex = 0;
    for (size_t readIndex = 0; readIndex < interruptCount_; ++readIndex)
    {
        // Only keep active interrupts with valid IDs
        if (interrupts_[readIndex].active && interrupts_[readIndex].id)
        {
            if (writeIndex != readIndex)
            {
                // Move interrupt to fill gap
                interrupts_[writeIndex] = interrupts_[readIndex];
                log_d("Moved interrupt %s from index %d to %d", 
                      interrupts_[writeIndex].id, readIndex, writeIndex);
            }
            ++writeIndex;
        }
        else
        {
            log_d("Removed inactive interrupt at index %d", readIndex);
        }
    }
    
    // Update count and clear remaining slots
    size_t oldCount = interruptCount_;
    interruptCount_ = writeIndex;
    
    // Clear unused slots
    for (size_t i = interruptCount_; i < oldCount; ++i)
    {
        memset(&interrupts_[i], 0, sizeof(Interrupt));
    }
    
    if (oldCount != interruptCount_)
    {
        log_i("Compacted interrupt array: %d -> %d interrupts", oldCount, interruptCount_);
    }
}

