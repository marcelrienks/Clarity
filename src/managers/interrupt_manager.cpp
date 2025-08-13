#include "managers/interrupt_manager.h"
#include <Arduino.h>
#include <algorithm>

#ifdef CLARITY_DEBUG
    #include "esp32-hal-log.h"
    #define LOG_TAG "InterruptManager"
#else
    #define log_d(...)
    #define log_w(...)
    #define log_i(...)
#endif

InterruptManager::InterruptManager(IPanelService *panelService)
    : panelService_(panelService), initialized_(false), lastCheckTime_(0), checkCount_(0)
{
}

void InterruptManager::Init()
{
    if (initialized_)
    {
        log_w("InterruptManager already initialized");
        return;
    }

    triggerSources_.clear();
    actionSources_.clear();
    lastCheckTime_ = millis();
    checkCount_ = 0;

    initialized_ = true;
    log_i("InterruptManager initialized");
}

void InterruptManager::RegisterTriggerSource(IInterruptService *source)
{
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

void InterruptManager::CheckAllInterrupts()
{
    if (!initialized_)
    {
        return;
    }

    checkCount_++;
    unsigned long currentTime = millis();

    // Remove excessive logging - only log when there's activity

    // Quick optimization: check if any sources have pending work
    bool hasAnyPending = HasAnyPendingInterrupts();
    if (!hasAnyPending)
    {
        return; // No logging for no activity
    }

    // Always check triggers - TriggerManager is smart enough to handle
    // trigger-driven panel logic internally
    for (IInterruptService *source : triggerSources_)
    {
        if (source && source->HasPendingInterrupts())
        {
            source->CheckInterrupts();
        }
    }

    // Always check actions - button input should work regardless of panel type
    for (IInterruptService *source : actionSources_)
    {
        if (source && source->HasPendingInterrupts())
        {
            source->CheckInterrupts();
        }
    }

    lastCheckTime_ = currentTime;
}

bool InterruptManager::HasAnyPendingInterrupts() const
{
    // Check triggers first
    for (const IInterruptService *source : triggerSources_)
    {
        if (source && source->HasPendingInterrupts())
        {
            return true;
        }
    }

    // Then check actions
    for (const IInterruptService *source : actionSources_)
    {
        if (source && source->HasPendingInterrupts())
        {
            return true;
        }
    }

    return false;
}
