#pragma once

#include "interfaces/i_handler.h"
#include "interfaces/i_gpio_provider.h"
#include "utilities/types.h"
#include <vector>
#include <memory>

// Forward declarations for sensors
class ActionButtonSensor;

#include "esp32-hal-log.h"

/**
 * @class QueuedHandler
 * @brief Handles queued interrupts with deferred execution
 * 
 * @details Processes interrupts that can be queued for later execution when conditions
 * are appropriate. Useful for panel transitions, theme changes, and other operations
 * that should be deferred during animations or busy states.
 * 
 * @queue_management FIFO queue with priority-based ordering
 * @memory_safe Fixed-size queue to prevent memory fragmentation
 * @execution_control Respects UI state and timing constraints
 */
class QueuedHandler : public IHandler
{
public:
    QueuedHandler(IGpioProvider* gpioProvider);
    ~QueuedHandler();  // Non-default destructor needed for unique_ptr with incomplete types
    
    // IHandler interface
    void Process() override;
    
    // Queue management
    bool QueueInterrupt(const Interrupt* interrupt);
    void ClearQueue();
    void SetMaxQueueSize(size_t maxSize);
    
    // Sensor access for interrupt context
    ActionButtonSensor* GetActionButtonSensor() const { return actionButtonSensor_.get(); }
    
    // Status
    size_t GetQueuedCount() const;
    bool HasQueuedInterrupts() const;
    bool IsQueueFull() const;
    
private:
    struct QueuedInterruptEntry
    {
        const Interrupt* interrupt;
        unsigned long queuedTime;
        unsigned long timeoutMs;
        
        QueuedInterruptEntry(const Interrupt* intr, unsigned long timeout = 5000);
    };
    
    void ProcessQueuedInterrupt(const QueuedInterruptEntry& entry);
    bool HasExpired(const QueuedInterruptEntry& entry) const;
    void RemoveExpiredEntries();
    bool CanExecuteInterrupt(const Interrupt* interrupt) const;
    
    static constexpr size_t DEFAULT_MAX_QUEUE_SIZE = 8;
    static constexpr unsigned long DEFAULT_TIMEOUT_MS = 5000;
    
    std::vector<QueuedInterruptEntry> queuedInterrupts_;
    size_t maxQueueSize_;
    
    // Handler-owned sensor for button input
    IGpioProvider* gpioProvider_;
    std::unique_ptr<ActionButtonSensor> actionButtonSensor_;
};