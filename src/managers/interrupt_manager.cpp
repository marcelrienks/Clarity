#include "managers/interrupt_manager.h"
#include <algorithm>
#include <Arduino.h>

#ifdef CLARITY_DEBUG
#include "esp32-hal-log.h"
#define LOG_TAG "InterruptManager"
#else
#define log_d(...)
#define log_w(...)
#define log_i(...)
#endif

InterruptManager::InterruptManager()
    : initialized_(false)
    , lastCheckTime_(0)
    , checkCount_(0)
{
}

void InterruptManager::Init()
{
    if (initialized_) {
        log_w("InterruptManager already initialized");
        return;
    }
    
    interruptSources_.clear();
    lastCheckTime_ = millis();
    checkCount_ = 0;
    
    initialized_ = true;
    log_i("InterruptManager initialized");
}

void InterruptManager::RegisterInterruptSource(IInterrupt* source)
{
    if (!source) {
        log_w("Attempted to register null interrupt source");
        return;
    }
    
    // Check if already registered
    auto it = std::find(interruptSources_.begin(), interruptSources_.end(), source);
    if (it != interruptSources_.end()) {
        log_w("Interrupt source already registered");
        return;
    }
    
    interruptSources_.push_back(source);
    SortSourcesByPriority();
    
    log_d("Registered interrupt source with priority %d (total: %zu)", 
          source->GetPriority(), interruptSources_.size());
}

void InterruptManager::UnregisterInterruptSource(IInterrupt* source)
{
    if (!source) {
        return;
    }
    
    auto it = std::find(interruptSources_.begin(), interruptSources_.end(), source);
    if (it != interruptSources_.end()) {
        interruptSources_.erase(it);
        log_d("Unregistered interrupt source (remaining: %zu)", interruptSources_.size());
    }
}

void InterruptManager::CheckAllInterrupts()
{
    if (!initialized_) {
        return;
    }
    
    checkCount_++;
    unsigned long currentTime = millis();
    
    // Quick optimization: check if any sources have pending work
    if (!HasAnyPendingInterrupts()) {
        return;
    }
    
    // Process interrupt sources in priority order
    for (IInterrupt* source : interruptSources_) {
        if (source && source->HasPendingInterrupts()) {
            source->CheckInterrupts();
        }
    }
    
    lastCheckTime_ = currentTime;
}

bool InterruptManager::HasAnyPendingInterrupts() const
{
    for (const IInterrupt* source : interruptSources_) {
        if (source && source->HasPendingInterrupts()) {
            return true;
        }
    }
    return false;
}

void InterruptManager::SortSourcesByPriority()
{
    std::sort(interruptSources_.begin(), interruptSources_.end(),
              [](const IInterrupt* a, const IInterrupt* b) {
                  if (!a) return false;
                  if (!b) return true;
                  return a->GetPriority() > b->GetPriority(); // Higher priority first
              });
    
    log_d("Sorted %zu interrupt sources by priority", interruptSources_.size());
}