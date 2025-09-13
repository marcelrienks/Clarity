#pragma once

#include "interfaces/i_handler.h"
#include "interfaces/i_gpio_provider.h"
#include "utilities/types.h"
#include "utilities/constants.h"
#include <vector>
#include <memory>

// Forward declarations for sensors
class BaseSensor;
class GpioSensor;

#include "esp32-hal-log.h"

/**
 * @class TriggerHandler
 * @brief Handles state-based triggers with GPIO sensor ownership and dual function system
 * 
 * @details Processes triggers that monitor GPIO state changes and execute dual functions:
 * - activateFunc: Called when sensor transitions from inactive to active
 * - deactivateFunc: Called when sensor transitions from active to inactive
 * 
 * @architecture Owns all GPIO sensors for exclusive state monitoring
 * @priority_system Implements priority-based blocking with same-type restoration
 * @memory_optimization Fixed-size trigger array for ESP32 safety
 */
class TriggerHandler : public IHandler
{
public:
    TriggerHandler(IGpioProvider* gpioProvider);
    ~TriggerHandler();
    
    // IHandler interface - new interrupt system only
    void Process() override;
    
    // New Trigger system interface
    bool RegisterTrigger(const Trigger& trigger);
    void UnregisterTrigger(const char* id);
    void EvaluateTriggers();  // Called during UI idle only
    
    // Priority override and restoration logic
    void SetHigherPriorityActive(Priority priority);
    void ClearHigherPriorityActive(Priority priority);
    bool IsBlocked(const Trigger& trigger) const;
    void RestoreSameTypeTrigger(TriggerType type, Priority priority);
    
    // Sensor access for trigger context
    GpioSensor* GetKeyPresentSensor() const { return keyPresentSensor_.get(); }
    GpioSensor* GetKeyNotPresentSensor() const { return keyNotPresentSensor_.get(); }
    GpioSensor* GetLockSensor() const { return lockSensor_.get(); }
    GpioSensor* GetLightsSensor() const { return lightsSensor_.get(); }
#ifdef CLARITY_DEBUG
    GpioSensor* GetDebugErrorSensor() const { return debugErrorSensor_.get(); }
#endif
    
    // Status and diagnostics
    size_t GetTriggerCount() const;
    bool HasActiveTriggers() const;
    void PrintTriggerStatus() const;
    
    // Find highest priority trigger of given type (public for InterruptManager)
    Trigger* FindHighestPrioritySameType(TriggerType type);
    
private:
    // Core trigger processing
    void EvaluateIndividualTrigger(Trigger& trigger);
    void ExecuteTriggerFunction(const Trigger& trigger, bool isActivation);
    bool ShouldActivate(const Trigger& trigger) const;
    bool ShouldDeactivate(const Trigger& trigger) const;
    
    // Priority and blocking logic
    bool HasHigherPriorityActive(Priority priority) const;
    void UpdatePriorityState(Priority priority, bool active);
    
    // Helper methods
    Trigger* FindTrigger(const char* id);
    bool IsSensorActive(const Trigger& trigger) const;
    
    static constexpr size_t MAX_TRIGGERS = 16;
    
    // Trigger storage
    Trigger triggers_[MAX_TRIGGERS];
    size_t triggerCount_ = 0;
    
    // Priority tracking for blocking logic
    bool priorityActive_[3] = {false, false, false};  // CRITICAL, IMPORTANT, NORMAL
    
    // Handler-owned sensors for GPIO monitoring
    IGpioProvider* gpioProvider_;
    std::unique_ptr<GpioSensor> keyPresentSensor_;
    std::unique_ptr<GpioSensor> keyNotPresentSensor_;
    std::unique_ptr<GpioSensor> lockSensor_;
    std::unique_ptr<GpioSensor> lightsSensor_;
#ifdef CLARITY_DEBUG
    std::unique_ptr<GpioSensor> debugErrorSensor_;
#endif
};