#include "handlers/queued_handler.h"
#include "managers/error_manager.h"
#include "sensors/button_sensor.h"
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
        log_d("Creating QueuedHandler-owned ButtonSensor");
        
        buttonSensor_ = std::make_unique<ButtonSensor>(gpioProvider_);
        buttonSensor_->Init();
        
        log_i("QueuedHandler created and initialized ButtonSensor for button input");
    } else {
        log_e("QueuedHandler: GPIO provider is null - ButtonSensor not created");
    }
}

QueuedHandler::~QueuedHandler() 
{
    log_d("QueuedHandler destructor - cleaning up ButtonSensor");
    // Unique_ptr will automatically clean up sensor
}

void QueuedHandler::Process()
{
    log_v("SIMPLIFIED QUEUED: Process() called");
    
    // In simplified system, check button sensor for state changes
    CheckButtonSensorStateChange();
}

void QueuedHandler::CheckButtonSensorStateChange()
{
    log_v("CheckButtonSensorStateChange() called");
    
    if (buttonSensor_ && buttonSensor_->HasStateChanged())
    {
        const char* triggerId = buttonSensor_->GetTriggerInterruptId();
        if (triggerId)
        {
            log_d("Button sensor changed - triggering interrupt: %s", triggerId);
            TriggerInterruptByID(triggerId);
        }
    }
}

void QueuedHandler::TriggerInterruptByID(const char* interruptId)
{
    log_v("TriggerInterruptByID(%s) called", interruptId);
    
    // Find the interrupt with matching ID in our registered interrupts
    for (auto& interrupt : queuedInterrupts_)
    {
        if (interrupt.interrupt && interrupt.interrupt->IsActive() && 
            interrupt.interrupt->id && strcmp(interrupt.interrupt->id, interruptId) == 0)
        {
            log_i("SIMPLIFIED QUEUED: Executing interrupt '%s' directly", interruptId);
            
            // Execute the interrupt directly
            if (interrupt.interrupt->execute)
            {
                interrupt.interrupt->execute(interrupt.interrupt->context);
            }
            return;
        }
    }
    
    log_w("Could not find active interrupt with ID: %s", interruptId);
}

void QueuedHandler::RegisterInterrupt(struct Interrupt* interrupt)
{
    log_v("HYBRID QUEUED: RegisterInterrupt() called");
    
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
    
    if (interrupt->source != InterruptSource::QUEUED)
    {
        log_w("Interrupt '%s' is not a QUEUED interrupt", interrupt->id);
        return;
    }
    
    // For QueuedHandler, we don't store interrupts permanently like PolledHandler
    // Instead, interrupts are queued dynamically when they need to be processed
    log_d("HYBRID QUEUED: Interrupt '%s' registered with QueuedHandler (queued on demand)", interrupt->id);
}

void QueuedHandler::UnregisterInterrupt(const char* id)
{
    log_v("HYBRID QUEUED: UnregisterInterrupt() called for: %s", id ? id : "null");
    
    if (!id) return;
    
    // Remove any queued instances of this interrupt
    auto it = std::remove_if(queuedInterrupts_.begin(), queuedInterrupts_.end(),
        [id](const QueuedInterruptEntry& entry) {
            return entry.interrupt && entry.interrupt->id && strcmp(entry.interrupt->id, id) == 0;
        });
    
    if (it != queuedInterrupts_.end())
    {
        queuedInterrupts_.erase(it, queuedInterrupts_.end());
        log_d("HYBRID QUEUED: Removed queued instances of interrupt '%s'", id);
    }
    else
    {
        log_d("HYBRID QUEUED: No queued instances of interrupt '%s' found", id);
    }
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
    
    if (!entry.interrupt->execute)
    {
        log_w("Invalid interrupt execute function for '%s'", 
              entry.interrupt->id ? entry.interrupt->id : "unknown");
        return;
    }
    
    // Execute the interrupt directly in simplified system
    log_d("Executing queued interrupt '%s'", entry.interrupt->id);
    entry.interrupt->execute(entry.interrupt->context);
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