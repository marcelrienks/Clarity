#include "handlers/polled_handler.h"
#include "managers/error_manager.h"
#include "managers/interrupt_manager.h"
#include "sensors/key_present_sensor.h"
#include "sensors/key_not_present_sensor.h"
#include "sensors/lock_sensor.h"
#include "sensors/lights_sensor.h"
#ifdef CLARITY_DEBUG
#include "sensors/debug_error_sensor.h"
#endif
#include "hardware/gpio_pins.h"
#include <Arduino.h>
#include <algorithm>
#include <cstring>

#include "esp32-hal-log.h"

PolledHandler::PolledHandler(IGpioProvider* gpioProvider) 
    : defaultInterval_(DEFAULT_EVALUATION_INTERVAL_MS),
      gpioProvider_(gpioProvider)
{
    log_v("PolledHandler() constructor called");
    polledInterrupts_.reserve(MAX_POLLED_INTERRUPTS);
    
    // Create and initialize GPIO sensors owned by this handler
    if (gpioProvider_) {
        log_d("Creating PolledHandler-owned GPIO sensors");
        
        // Create split key sensors
        keyPresentSensor_ = std::make_unique<KeyPresentSensor>(gpioProvider_);
        keyNotPresentSensor_ = std::make_unique<KeyNotPresentSensor>(gpioProvider_);
        
        // Create other GPIO sensors
        lockSensor_ = std::make_unique<LockSensor>(gpioProvider_);
        lightsSensor_ = std::make_unique<LightsSensor>(gpioProvider_);
        
#ifdef CLARITY_DEBUG
        // Create debug error sensor (debug builds only)
        debugErrorSensor_ = std::make_unique<DebugErrorSensor>(gpioProvider_);
#endif
        
        // Initialize all sensors
        keyPresentSensor_->Init();
        keyNotPresentSensor_->Init();
        lockSensor_->Init();
        lightsSensor_->Init();
        
#ifdef CLARITY_DEBUG
        debugErrorSensor_->Init();
        log_i("PolledHandler created and initialized 5 GPIO sensors: KeyPresent, KeyNotPresent, Lock, Lights, DebugError (debug build)");
#else
        log_i("PolledHandler created and initialized 4 GPIO sensors: KeyPresent, KeyNotPresent, Lock, Lights");
#endif
    } else {
        log_e("PolledHandler: GPIO provider is null - sensors not created");
    }
}

PolledHandler::~PolledHandler() 
{
#ifdef CLARITY_DEBUG
    log_d("PolledHandler destructor - cleaning up 5 GPIO sensors (including debug sensor)");
#else
    log_d("PolledHandler destructor - cleaning up 4 GPIO sensors");
#endif
    // Unique_ptr will automatically clean up sensors
}

void PolledHandler::Process()
{
    log_v("HYBRID POLLED: Process() called - using three-phase approach");
    
    // Phase 1: Evaluate all interrupts once and cache results
    EvaluateAllInterrupts();
    
    // Phase 2: Execute highest priority interrupt with exclusion rules
    ExecuteHighestPriorityInterrupt();
    
    // Phase 3: Clear cached state for next cycle
    ClearStateChanges();
}

// ===== HYBRID ARCHITECTURE IMPLEMENTATION =====

void PolledHandler::EvaluateAllInterrupts()
{
    log_v("HYBRID POLLED: EvaluateAllInterrupts() called");
    
#ifdef CLARITY_DEBUG
    // Debug: Direct check of GPIO 34 state
    if (debugErrorSensor_ && gpioProvider_) {
        bool gpio34State = gpioProvider_->DigitalRead(gpio_pins::DEBUG_ERROR);
        log_d("DEBUG: Direct GPIO 34 read = %s", gpio34State ? "HIGH" : "LOW");
    }
#endif
    
    // In debug builds, check the debug error sensor state directly
#ifdef CLARITY_DEBUG
    if (debugErrorSensor_) {
        // Check if it's been past the grace period
        if (millis() - debugErrorSensor_->GetStartupTime() >= 1000) {
            bool hasChanged = debugErrorSensor_->HasStateChanged();
            if (hasChanged) {
                log_i("DEBUG: Debug error sensor state changed - triggering OnInterruptTriggered");
                debugErrorSensor_->OnInterruptTriggered();
            }
        }
    }
#endif
    
    log_d("HYBRID POLLED: Evaluating %zu interrupts", polledInterrupts_.size());
    for (auto& interrupt : polledInterrupts_)
    {
        if (!interrupt || !interrupt->IsActive() || !interrupt->evaluationFunc)
            continue;
            
        if (ShouldEvaluateInterrupt(*interrupt))
        {
            log_v("Evaluating interrupt: %s", interrupt->id);
            // Single evaluation per cycle - cache the result
            bool shouldExecute = interrupt->evaluationFunc(interrupt->context);
            interrupt->SetStateChanged(shouldExecute);
            interrupt->SetLastEvaluation(millis());
            
            if (shouldExecute)
            {
                log_i("HYBRID POLLED: Interrupt '%s' state changed - marked for execution", interrupt->id);
            }
        }
    }
}

void PolledHandler::ExecuteHighestPriorityInterrupt()
{
    log_v("HYBRID POLLED: ExecuteHighestPriorityInterrupt() called");
    
    // Clear exclusion group tracking for this cycle
    executedGroups_.clear();
    
    // Process interrupts by priority order
    for (int priority = 0; priority <= 2; ++priority) // CRITICAL=0, IMPORTANT=1, NORMAL=2
    {
        for (auto& interrupt : polledInterrupts_)
        {
            if (!interrupt || !interrupt->HasStateChanged())
                continue;
                
            if (static_cast<int>(interrupt->priority) == priority && CanExecute(*interrupt))
            {
                log_i("HYBRID POLLED: Executing interrupt '%s' (priority %d)", 
                      interrupt->id, priority);
                      
                ExecuteInterrupt(*interrupt);
                
                // Only execute one interrupt per priority level per cycle
                break;
            }
        }
    }
}

void PolledHandler::ClearStateChanges()
{
    log_v("HYBRID POLLED: ClearStateChanges() called");
    
    for (auto& interrupt : polledInterrupts_)
    {
        if (interrupt)
        {
            interrupt->SetStateChanged(false);
        }
    }
}

void PolledHandler::RegisterInterrupt(struct Interrupt* interrupt)
{
    log_v("HYBRID POLLED: RegisterInterrupt() called");
    
    if (!interrupt)
    {
        log_w("Cannot register null interrupt");
        return;
    }
    
    if (!interrupt->id)
    {
        log_w("Cannot register interrupt with null ID");
        return;
    }
    
    if (interrupt->source != InterruptSource::POLLED)
    {
        log_w("Interrupt '%s' is not a POLLED interrupt", interrupt->id);
        return;
    }
    
    if (polledInterrupts_.size() >= MAX_POLLED_INTERRUPTS)
    {
        log_e("Cannot register interrupt - maximum polled capacity reached (%d)", MAX_POLLED_INTERRUPTS);
        return;
    }
    
    // Check for duplicate registration
    for (const auto& existing : polledInterrupts_)
    {
        if (existing && existing->id && strcmp(existing->id, interrupt->id) == 0)
        {
            log_w("Polled interrupt '%s' already registered", interrupt->id);
            return;
        }
    }
    
    // Take exclusive ownership of this interrupt
    polledInterrupts_.push_back(interrupt);
    log_d("HYBRID POLLED: Registered interrupt '%s' (total: %d)", interrupt->id, polledInterrupts_.size());
}

void PolledHandler::UnregisterInterrupt(const char* id)
{
    log_v("UnregisterInterrupt() called for: %s", id ? id : "null");
    
    if (!id) return;
    
    auto it = std::remove_if(polledInterrupts_.begin(), polledInterrupts_.end(),
        [id](const Interrupt* interrupt) {
            return interrupt && interrupt->id && strcmp(interrupt->id, id) == 0;
        });
    
    if (it != polledInterrupts_.end())
    {
        polledInterrupts_.erase(it, polledInterrupts_.end());
        log_d("Unregistered polled interrupt '%s' (remaining: %d)", id, polledInterrupts_.size());
    }
    else
    {
        log_w("Polled interrupt '%s' not found for unregistration", id);
    }
}

void PolledHandler::SetEvaluationInterval(unsigned long intervalMs)
{
    log_v("SetEvaluationInterval() called with interval: %lu ms", intervalMs);
    defaultInterval_ = intervalMs;
    log_d("Set default evaluation interval to %lu ms", intervalMs);
}

size_t PolledHandler::GetInterruptCount() const
{
    return polledInterrupts_.size();
}

bool PolledHandler::HasPendingEvaluations() const
{
    unsigned long currentTime = millis();
    
    for (const auto& interrupt : polledInterrupts_)
    {
        if (interrupt && interrupt->IsActive())
        {
            if (ShouldEvaluateInterrupt(*interrupt))
            {
                return true;
            }
        }
    }
    
    return false;
}

// ===== HYBRID ARCHITECTURE HELPER METHODS =====

bool PolledHandler::ShouldEvaluateInterrupt(const Interrupt& interrupt) const
{
    // Optimize evaluation frequency based on priority to reduce CPU usage
    unsigned long currentTime = millis();
    unsigned long timeSinceLastEvaluation = currentTime - interrupt.GetLastEvaluation();
    
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

bool PolledHandler::CanExecute(const Interrupt& interrupt) const
{
    log_d("CanExecute() called for interrupt '%s'", interrupt.id);
    
    // Check if interrupt has ALWAYS_EXECUTE flag
    if ((interrupt.flags & InterruptFlags::ALWAYS_EXECUTE) != InterruptFlags::NONE)
    {
        log_d("Interrupt '%s' has ALWAYS_EXECUTE flag - can execute", interrupt.id);
        return true;
    }
    
    return true;
}


void PolledHandler::ExecuteInterrupt(Interrupt& interrupt)
{
    log_i("ExecuteInterrupt() called for: %s", interrupt.id);
    
    if (interrupt.executionFunc)
    {
        interrupt.executionFunc(interrupt.context);
        log_d("Execution completed for interrupt '%s'", interrupt.id);
    }
    else
    {
        log_w("No execution function for interrupt '%s'", interrupt.id);
    }
}