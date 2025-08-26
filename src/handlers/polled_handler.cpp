#include "handlers/polled_handler.h"
#include "managers/error_manager.h"
#include "sensors/key_present_sensor.h"
#include "sensors/key_not_present_sensor.h"
#include "sensors/lock_sensor.h"
#include "sensors/lights_sensor.h"
#ifdef CLARITY_DEBUG
#include "sensors/debug_error_sensor.h"
#endif
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
    log_v("Process() called");
    
    unsigned long currentTime = millis();
    
    for (auto& ref : polledInterrupts_)
    {
        if (ref.interrupt && ref.interrupt->active && ShouldEvaluate(ref))
        {
            EvaluateInterrupt(ref);
        }
    }
}

const Interrupt* PolledHandler::GetHighestPriorityActiveInterrupt()
{
    log_v("GetHighestPriorityActiveInterrupt() called");
    
    const Interrupt* highestPriority = nullptr;
    int lowestPriorityValue = 3; // Lower number = higher priority (CRITICAL=0, IMPORTANT=1, NORMAL=2)
    
    for (auto& ref : polledInterrupts_)
    {
        if (ref.interrupt && ref.interrupt->active)
        {
            // Check if interrupt's condition is currently true
            if (ref.interrupt->processFunc && 
                ref.interrupt->processFunc(ref.interrupt->context) == InterruptResult::EXECUTE_EFFECT)
            {
                int priorityValue = static_cast<int>(ref.interrupt->priority);
                if (priorityValue < lowestPriorityValue)
                {
                    lowestPriorityValue = priorityValue;
                    highestPriority = ref.interrupt;
                }
            }
        }
    }
    
    if (highestPriority)
    {
        log_d("Highest priority polled interrupt: '%s' (priority %d)", 
              highestPriority->id, static_cast<int>(highestPriority->priority));
    }
    else
    {
        log_v("No active polled interrupts found");
    }
    
    return highestPriority;
}

void PolledHandler::RegisterInterrupt(const Interrupt* interrupt)
{
    log_v("RegisterInterrupt() called");
    
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
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "PolledHandler", 
                                           "Maximum polled interrupt capacity exceeded");
        return;
    }
    
    // Check for duplicate registration
    for (const auto& ref : polledInterrupts_)
    {
        if (ref.interrupt && ref.interrupt->id && strcmp(ref.interrupt->id, interrupt->id) == 0)
        {
            log_w("Polled interrupt '%s' already registered", interrupt->id);
            return;
        }
    }
    
    polledInterrupts_.emplace_back(interrupt, defaultInterval_);
    log_d("Registered polled interrupt '%s' (total: %d)", interrupt->id, polledInterrupts_.size());
}

void PolledHandler::UnregisterInterrupt(const char* id)
{
    log_v("UnregisterInterrupt() called for: %s", id ? id : "null");
    
    if (!id) return;
    
    auto it = std::remove_if(polledInterrupts_.begin(), polledInterrupts_.end(),
        [id](const PolledInterruptRef& ref) {
            return ref.interrupt && ref.interrupt->id && strcmp(ref.interrupt->id, id) == 0;
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
    
    for (const auto& ref : polledInterrupts_)
    {
        if (ref.interrupt && ref.interrupt->active)
        {
            if (currentTime - ref.lastEvaluation >= ref.evaluationInterval)
            {
                return true;
            }
        }
    }
    
    return false;
}

void PolledHandler::EvaluateInterrupt(PolledInterruptRef& ref)
{
    log_v("EvaluateInterrupt() called for: %s", ref.interrupt->id ? ref.interrupt->id : "unknown");
    
    if (!ref.interrupt->processFunc)
    {
        log_w("Invalid interrupt process function for '%s'", ref.interrupt->id ? ref.interrupt->id : "unknown");
        return;
    }
    
    // Update evaluation timestamp first to prevent rapid re-evaluation
    ref.lastEvaluation = millis();
    
    // Process the interrupt (evaluate and potentially signal execution)
    InterruptResult result = ref.interrupt->processFunc(ref.interrupt->context);
    if (result == InterruptResult::EXECUTE_EFFECT)
    {
        log_d("Polled interrupt '%s' condition met - signaling for execution", ref.interrupt->id);
        // Note: Actual execution is now handled centrally by InterruptManager via ExecuteByEffect()
    }
}

bool PolledHandler::ShouldEvaluate(const PolledInterruptRef& ref) const
{
    unsigned long currentTime = millis();
    return (currentTime - ref.lastEvaluation) >= ref.evaluationInterval;
}