#include "handlers/queued_handler.h"
#include "managers/error_manager.h"
#include "sensors/action_button_sensor.h"
#include <Arduino.h>
#include <algorithm>
#include <cstring>

#include "esp32-hal-log.h"

// QueuedInterruptEntry constructor implementation
QueuedHandler::QueuedInterruptEntry::QueuedInterruptEntry(const Interrupt* intr, unsigned long timeout)
    : interrupt(intr), queuedTime(millis()), timeoutMs(timeout)
{
}

QueuedHandler::QueuedHandler(IGpioProvider* gpioProvider) 
    : maxQueueSize_(DEFAULT_MAX_QUEUE_SIZE),
      gpioProvider_(gpioProvider)
{
    log_v("QueuedHandler() constructor called");
    queuedInterrupts_.reserve(maxQueueSize_);
    
    // Create and initialize button sensor owned by this handler
    if (gpioProvider_) {
        log_d("Creating QueuedHandler-owned ActionButtonSensor");
        
        actionButtonSensor_ = std::make_unique<ActionButtonSensor>(gpioProvider_);
        actionButtonSensor_->Init();
        
        log_i("QueuedHandler created and initialized ActionButtonSensor for button input");
    } else {
        log_e("QueuedHandler: GPIO provider is null - ActionButtonSensor not created");
    }
}

QueuedHandler::~QueuedHandler() 
{
    log_d("QueuedHandler destructor - cleaning up ActionButtonSensor");
    // Unique_ptr will automatically clean up sensor
}

void QueuedHandler::Process()
{
    log_v("Process() called");
    
    if (queuedInterrupts_.empty())
    {
        return;
    }
    
    // Remove expired entries first
    RemoveExpiredEntries();
    
    // Process interrupts in queue order (FIFO), but respect priority
    auto it = queuedInterrupts_.begin();
    while (it != queuedInterrupts_.end())
    {
        const auto& entry = *it;
        
        if (CanExecuteInterrupt(entry.interrupt))
        {
            log_d("Processing queued interrupt '%s'", entry.interrupt->id ? entry.interrupt->id : "unknown");
            ProcessQueuedInterrupt(entry);
            it = queuedInterrupts_.erase(it);
        }
        else
        {
            // Check for critical interrupts that should be processed immediately
            if (entry.interrupt->priority == Priority::CRITICAL)
            {
                log_d("Processing critical queued interrupt '%s' immediately", 
                      entry.interrupt->id ? entry.interrupt->id : "unknown");
                ProcessQueuedInterrupt(entry);
                it = queuedInterrupts_.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
}

const Interrupt* QueuedHandler::GetHighestPriorityActiveInterrupt()
{
    log_v("GetHighestPriorityActiveInterrupt() called");
    
    if (queuedInterrupts_.empty())
    {
        log_v("No queued interrupts found");
        return nullptr;
    }
    
    const Interrupt* highestPriority = nullptr;
    int lowestPriorityValue = 3; // Lower number = higher priority (CRITICAL=0, IMPORTANT=1, NORMAL=2)
    
    // Remove expired entries first to get accurate results
    const_cast<QueuedHandler*>(this)->RemoveExpiredEntries();
    
    for (const auto& entry : queuedInterrupts_)
    {
        if (entry.interrupt && entry.interrupt->active)
        {
            // For queued interrupts, check if they're ready to execute
            if (CanExecuteInterrupt(entry.interrupt))
            {
                int priorityValue = static_cast<int>(entry.interrupt->priority);
                if (priorityValue < lowestPriorityValue)
                {
                    lowestPriorityValue = priorityValue;
                    highestPriority = entry.interrupt;
                }
            }
        }
    }
    
    if (highestPriority)
    {
        log_d("Highest priority queued interrupt: '%s' (priority %d)", 
              highestPriority->id, static_cast<int>(highestPriority->priority));
    }
    else
    {
        log_v("No active queued interrupts ready to execute");
    }
    
    return highestPriority;
}

bool QueuedHandler::QueueInterrupt(const Interrupt* interrupt)
{
    log_v("QueueInterrupt() called");
    
    if (!interrupt)
    {
        log_w("Cannot queue null interrupt");
        return false;
    }
    
    if (!interrupt->id)
    {
        log_w("Cannot queue interrupt with null ID");
        return false;
    }
    
    if (interrupt->source != InterruptSource::QUEUED)
    {
        log_w("Interrupt '%s' is not a QUEUED interrupt", interrupt->id);
        return false;
    }
    
    if (IsQueueFull())
    {
        log_e("Cannot queue interrupt - queue is full (%d)", maxQueueSize_);
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "QueuedHandler", 
                                           "Interrupt queue is full");
        return false;
    }
    
    // Check for duplicate in queue
    for (const auto& entry : queuedInterrupts_)
    {
        if (entry.interrupt && entry.interrupt->id && 
            strcmp(entry.interrupt->id, interrupt->id) == 0)
        {
            log_w("Interrupt '%s' is already queued", interrupt->id);
            return false;
        }
    }
    
    queuedInterrupts_.emplace_back(interrupt, DEFAULT_TIMEOUT_MS);
    
    // Sort by priority (CRITICAL = 0 comes first)
    std::sort(queuedInterrupts_.begin(), queuedInterrupts_.end(),
        [](const QueuedInterruptEntry& a, const QueuedInterruptEntry& b) {
            return static_cast<int>(a.interrupt->priority) < static_cast<int>(b.interrupt->priority);
        });
    
    log_d("Queued interrupt '%s' with priority %d (queue size: %d)", 
          interrupt->id, static_cast<int>(interrupt->priority), queuedInterrupts_.size());
    
    return true;
}

void QueuedHandler::ClearQueue()
{
    log_v("ClearQueue() called");
    
    size_t clearedCount = queuedInterrupts_.size();
    queuedInterrupts_.clear();
    
    if (clearedCount > 0)
    {
        log_d("Cleared %d queued interrupts", clearedCount);
    }
}

void QueuedHandler::SetMaxQueueSize(size_t maxSize)
{
    log_v("SetMaxQueueSize() called with size: %d", maxSize);
    
    maxQueueSize_ = maxSize;
    
    // If current queue exceeds new size, trim from the end (lowest priority)
    if (queuedInterrupts_.size() > maxQueueSize_)
    {
        size_t toRemove = queuedInterrupts_.size() - maxQueueSize_;
        queuedInterrupts_.erase(queuedInterrupts_.end() - toRemove, queuedInterrupts_.end());
        log_d("Trimmed %d interrupts to fit new queue size %d", toRemove, maxQueueSize_);
    }
    
    log_d("Set maximum queue size to %d", maxQueueSize_);
}

size_t QueuedHandler::GetQueuedCount() const
{
    return queuedInterrupts_.size();
}

bool QueuedHandler::HasQueuedInterrupts() const
{
    return !queuedInterrupts_.empty();
}

bool QueuedHandler::IsQueueFull() const
{
    return queuedInterrupts_.size() >= maxQueueSize_;
}

void QueuedHandler::ProcessQueuedInterrupt(const QueuedInterruptEntry& entry)
{
    log_v("ProcessQueuedInterrupt() called for: %s", 
          entry.interrupt->id ? entry.interrupt->id : "unknown");
    
    if (!entry.interrupt->processFunc)
    {
        log_w("Invalid interrupt process function for '%s'", 
              entry.interrupt->id ? entry.interrupt->id : "unknown");
        return;
    }
    
    // Process the interrupt (evaluate and potentially signal execution)
    InterruptResult result = entry.interrupt->processFunc(entry.interrupt->context);
    if (result == InterruptResult::EXECUTE_EFFECT)
    {
        log_d("Queued interrupt '%s' condition met - signaling for execution", entry.interrupt->id);
        // Note: Actual execution is now handled centrally by InterruptManager via ExecuteByEffect()
    }
    else
    {
        log_d("Queued interrupt '%s' condition not met - skipping execution", entry.interrupt->id);
    }
}

bool QueuedHandler::HasExpired(const QueuedInterruptEntry& entry) const
{
    unsigned long currentTime = millis();
    return (currentTime - entry.queuedTime) > entry.timeoutMs;
}

void QueuedHandler::RemoveExpiredEntries()
{
    log_v("RemoveExpiredEntries() called");
    
    auto originalSize = queuedInterrupts_.size();
    
    auto it = std::remove_if(queuedInterrupts_.begin(), queuedInterrupts_.end(),
        [this](const QueuedInterruptEntry& entry) {
            bool expired = HasExpired(entry);
            if (expired)
            {
                log_d("Removing expired queued interrupt '%s'", 
                      entry.interrupt->id ? entry.interrupt->id : "unknown");
            }
            return expired;
        });
    
    queuedInterrupts_.erase(it, queuedInterrupts_.end());
    
    if (queuedInterrupts_.size() < originalSize)
    {
        log_d("Removed %d expired interrupts from queue", originalSize - queuedInterrupts_.size());
    }
}

bool QueuedHandler::CanExecuteInterrupt(const Interrupt* interrupt) const
{
    log_v("CanExecuteInterrupt() called");
    
    // Phase 2: Simple implementation - allow all interrupts
    // In future phases, this can check UI state, panel state, etc.
    return true;
}