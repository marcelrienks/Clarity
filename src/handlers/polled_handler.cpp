#include "handlers/polled_handler.h"
#include "managers/error_manager.h"
#include <Arduino.h>
#include <algorithm>
#include <cstring>

#include "esp32-hal-log.h"

PolledHandler::PolledHandler() 
    : defaultInterval_(DEFAULT_EVALUATION_INTERVAL_MS)
{
    log_v("PolledHandler() constructor called");
    polledInterrupts_.reserve(MAX_POLLED_INTERRUPTS);
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
    
    if (!ref.interrupt->evaluationFunc || !ref.interrupt->executionFunc)
    {
        log_w("Invalid interrupt functions for '%s'", ref.interrupt->id ? ref.interrupt->id : "unknown");
        return;
    }
    
    // Update evaluation timestamp first to prevent rapid re-evaluation
    ref.lastEvaluation = millis();
    
    // Evaluate the condition
    if (ref.interrupt->evaluationFunc(ref.interrupt->context))
    {
        log_d("Polled interrupt '%s' condition met - executing", ref.interrupt->id);
        ref.interrupt->executionFunc(ref.interrupt->context);
    }
}

bool PolledHandler::ShouldEvaluate(const PolledInterruptRef& ref) const
{
    unsigned long currentTime = millis();
    return (currentTime - ref.lastEvaluation) >= ref.evaluationInterval;
}