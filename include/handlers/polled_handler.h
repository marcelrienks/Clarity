#pragma once

#include "interfaces/i_handler.h"
#include "interfaces/i_gpio_provider.h"
#include "utilities/types.h"
#include <vector>
#include <memory>

// Forward declarations for sensors
class KeyPresentSensor;
class KeyNotPresentSensor;
class LockSensor;
class LightsSensor;
#ifdef CLARITY_DEBUG
class DebugErrorSensor;
#endif

#include "esp32-hal-log.h"

/**
 * @class PolledHandler
 * @brief Handles polled interrupts using hybrid single-evaluation architecture
 * 
 * @details Processes polled interrupts with three-phase approach:
 * 1. EvaluateAllInterrupts() - Call processFunc once per interrupt and cache results
 * 2. ExecuteHighestPriorityInterrupt() - Execute based on priority and exclusion rules
 * 3. ClearStateChanges() - Reset cached state for next cycle
 * 
 * @architecture Eliminates race conditions by owning interrupts exclusively
 * @timing Configurable evaluation intervals per interrupt  
 * @execution_rules Supports ALWAYS, EXCLUSIVE, and CONDITIONAL interrupt modes
 */
class PolledHandler : public IHandler
{
public:
    PolledHandler(IGpioProvider* gpioProvider);
    ~PolledHandler();  // Non-default destructor needed for unique_ptr with incomplete types
    
    // IHandler interface
    void Process() override;
    // Legacy methods - not part of current IHandler interface
    void RegisterInterrupt(struct Interrupt* interrupt);
    void UnregisterInterrupt(const char* id);
    void SetEvaluationInterval(unsigned long intervalMs);
    
    // Sensor access for interrupt context
    KeyPresentSensor* GetKeyPresentSensor() const { return keyPresentSensor_.get(); }
    KeyNotPresentSensor* GetKeyNotPresentSensor() const { return keyNotPresentSensor_.get(); }
    LockSensor* GetLockSensor() const { return lockSensor_.get(); }
    LightsSensor* GetLightsSensor() const { return lightsSensor_.get(); }
#ifdef CLARITY_DEBUG
    DebugErrorSensor* GetDebugErrorSensor() const { return debugErrorSensor_.get(); }
#endif
    
    // Status
    size_t GetInterruptCount() const;
    bool HasPendingEvaluations() const;
    
private:
    // Hybrid architecture methods
    void EvaluateAllInterrupts();
    void ExecuteHighestPriorityInterrupt();
    void ClearStateChanges();
    bool CanExecute(const Interrupt& interrupt) const;
    bool ShouldEvaluateInterrupt(const Interrupt& interrupt) const;
    void ExecuteInterrupt(Interrupt& interrupt);
    bool IsGroupExecuted(const char* group) const;
    
    // Simplified interrupt system methods
    void CheckSensorStateChanges();
    void TriggerInterruptByID(const char* interruptId);
    
    static constexpr size_t MAX_POLLED_INTERRUPTS = 16;
    static constexpr unsigned long DEFAULT_EVALUATION_INTERVAL_MS = 100;
    
    std::vector<Interrupt*> polledInterrupts_;
    std::vector<const char*> executedGroups_; // Track exclusion groups
    unsigned long defaultInterval_;
    
    // Handler-owned sensors for GPIO monitoring
    IGpioProvider* gpioProvider_;
    std::unique_ptr<KeyPresentSensor> keyPresentSensor_;
    std::unique_ptr<KeyNotPresentSensor> keyNotPresentSensor_;
    std::unique_ptr<LockSensor> lockSensor_;
    std::unique_ptr<LightsSensor> lightsSensor_;
#ifdef CLARITY_DEBUG
    std::unique_ptr<DebugErrorSensor> debugErrorSensor_;
#endif
};