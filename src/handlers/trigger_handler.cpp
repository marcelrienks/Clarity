#include "handlers/trigger_handler.h"
#include "managers/error_manager.h"
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

TriggerHandler::TriggerHandler(IGpioProvider* gpioProvider) 
    : gpioProvider_(gpioProvider),
      triggerCount_(0)
{
    log_v("TriggerHandler() constructor called");
    
    // Initialize priority tracking
    for (int i = 0; i < 3; i++) {
        priorityActive_[i] = false;
    }
    
    // Create and initialize GPIO sensors owned by this handler
    if (gpioProvider_) {
        log_d("Creating TriggerHandler-owned GPIO sensors");
        
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
        log_i("TriggerHandler created and initialized 5 GPIO sensors: KeyPresent, KeyNotPresent, Lock, Lights, DebugError (debug build)");
#else
        log_i("TriggerHandler created and initialized 4 GPIO sensors: KeyPresent, KeyNotPresent, Lock, Lights");
#endif
    }
    else {
        log_w("TriggerHandler created without GPIO provider - sensors not initialized");
    }
}

TriggerHandler::~TriggerHandler() {
    log_v("TriggerHandler destructor called");
}

void TriggerHandler::Process() {
    // Only evaluate triggers during UI idle - this is called from InterruptManager
    EvaluateTriggers();
}

bool TriggerHandler::RegisterTrigger(const Trigger& trigger) {
    if (triggerCount_ >= MAX_TRIGGERS) {
        log_e("Cannot register trigger '%s' - maximum triggers reached (%d)", 
              trigger.id, MAX_TRIGGERS);
        return false;
    }
    
    // Check for duplicate ID
    if (FindTrigger(trigger.id) != nullptr) {
        log_w("Trigger with ID '%s' already registered", trigger.id);
        return false;
    }
    
    // Copy trigger to storage
    triggers_[triggerCount_] = trigger;
    triggers_[triggerCount_].isActive = false;  // Always start inactive
    triggerCount_++;
    
    log_i("Registered trigger '%s' (type: %d, priority: %d)", 
          trigger.id, static_cast<int>(trigger.type), static_cast<int>(trigger.priority));
    return true;
}

void TriggerHandler::UnregisterTrigger(const char* id) {
    Trigger* trigger = FindTrigger(id);
    if (!trigger) {
        log_w("Cannot unregister trigger '%s' - not found", id);
        return;
    }
    
    // Find index and shift array
    size_t index = trigger - triggers_;
    for (size_t i = index; i < triggerCount_ - 1; i++) {
        triggers_[i] = triggers_[i + 1];
    }
    triggerCount_--;
    
    log_i("Unregistered trigger '%s'", id);
}

void TriggerHandler::EvaluateTriggers() {
    // Process each trigger for state changes
    for (size_t i = 0; i < triggerCount_; i++) {
        EvaluateIndividualTrigger(triggers_[i]);
    }
}

void TriggerHandler::EvaluateIndividualTrigger(Trigger& trigger) {
    if (!trigger.sensor) {
        return; // No sensor associated
    }
    
    // Check sensor state change
    bool sensorActive = IsSensorActive(trigger);
    bool wasActive = trigger.isActive;
    
    // Determine if trigger should activate or deactivate
    if (!wasActive && sensorActive && ShouldActivate(trigger)) {
        // Activation Flow with Priority Logic (as per interrupt-architecture.md)
        if (!HasHigherPriorityActive(trigger.priority)) {
            log_d("Executing activation for '%s'", trigger.id);
            trigger.ExecuteActivate();  // Execute and set active
        } else {
            log_d("Activation of '%s' suppressed by higher priority", trigger.id);
            trigger.isActive = true;    // Only mark active, don't execute
        }
        UpdatePriorityState(trigger.priority, true);
    }
    else if (wasActive && !sensorActive && ShouldDeactivate(trigger)) {
        // Deactivation Flow with Type-Based Restoration (as per interrupt-architecture.md)
        log_d("Executing deactivation for '%s'", trigger.id);
        trigger.ExecuteDeactivate();  // Execute and set inactive
        UpdatePriorityState(trigger.priority, false);
        
        // Find highest priority active trigger of same type
        Trigger* sameTypeTrigger = FindHighestPrioritySameType(trigger.type, trigger.priority);
        if (sameTypeTrigger) {
            log_d("Re-activating same-type trigger '%s'", sameTypeTrigger->id);
            sameTypeTrigger->ExecuteActivate();
        }
    }
}

bool TriggerHandler::ShouldActivate(const Trigger& trigger) const {
    // Block activation if higher priority trigger is active
    return !IsBlocked(trigger);
}

bool TriggerHandler::ShouldDeactivate(const Trigger& trigger) const {
    // Always allow deactivation
    return true;
}

bool TriggerHandler::IsBlocked(const Trigger& trigger) const {
    // Check if any higher priority trigger is active
    return HasHigherPriorityActive(trigger.priority);
}

bool TriggerHandler::HasHigherPriorityActive(Priority priority) const {
    // With corrected priority values: CRITICAL=2, IMPORTANT=1, NORMAL=0
    // Higher numeric value = higher priority
    int candidatePriority = static_cast<int>(priority);
    
    // Check if any active trigger has higher numeric priority
    for (size_t i = 0; i < triggerCount_; i++) {
        const Trigger& trigger = triggers_[i];
        if (trigger.isActive && static_cast<int>(trigger.priority) > candidatePriority) {
            return true;
        }
    }
    return false;
}

void TriggerHandler::UpdatePriorityState(Priority priority, bool active) {
    int priorityIndex = static_cast<int>(priority);
    if (priorityIndex >= 0 && priorityIndex < 3) {
        priorityActive_[priorityIndex] = active;
    }
}

void TriggerHandler::RestoreSameTypeTrigger(TriggerType type, Priority excludePriority) {
    // Find the highest priority trigger of the same type that is still active
    Trigger* toRestore = FindHighestPrioritySameType(type, excludePriority);
    if (toRestore && toRestore->isActive && toRestore->activateFunc) {
        log_d("Restoring same-type trigger '%s' after '%s' deactivated", 
              toRestore->id, TriggerIds::KEY_PRESENT); // Placeholder for logging
        toRestore->activateFunc();
    }
}

Trigger* TriggerHandler::FindHighestPrioritySameType(TriggerType type, Priority excludePriority) {
    Trigger* highest = nullptr;
    int highestPriorityValue = -1;  // Track numeric priority value
    
    for (size_t i = 0; i < triggerCount_; i++) {
        Trigger& trigger = triggers_[i];
        if (trigger.type == type && 
            trigger.priority != excludePriority &&
            trigger.isActive) {
            
            int triggerPriorityValue = static_cast<int>(trigger.priority);
            if (triggerPriorityValue > highestPriorityValue) {
                highest = &trigger;
                highestPriorityValue = triggerPriorityValue;
            }
        }
    }
    
    return highest;
}

void TriggerHandler::ExecuteTriggerFunction(const Trigger& trigger, bool isActivation) {
    if (isActivation && trigger.activateFunc) {
        trigger.activateFunc();
    }
    else if (!isActivation && trigger.deactivateFunc) {
        trigger.deactivateFunc();
    }
}

bool TriggerHandler::IsSensorActive(const Trigger& trigger) const {
    if (!trigger.sensor || !trigger.sensor->HasStateChanged()) {
        return false;
    }
    
    // Check sensor reading (assuming bool sensors for GPIO)
    // This is a simplified implementation - real sensors might need different logic
    return trigger.sensor->HasStateChanged();
}

Trigger* TriggerHandler::FindTrigger(const char* id) {
    for (size_t i = 0; i < triggerCount_; i++) {
        if (strcmp(triggers_[i].id, id) == 0) {
            return &triggers_[i];
        }
    }
    return nullptr;
}

// Legacy compatibility methods
void TriggerHandler::RegisterInterrupt(struct Interrupt* interrupt) {
    log_w("TriggerHandler::RegisterInterrupt() called - legacy compatibility only");
    // Could convert Interrupt to Trigger format if needed
}

void TriggerHandler::UnregisterInterrupt(const char* id) {
    log_w("TriggerHandler::UnregisterInterrupt() called - legacy compatibility only");
    UnregisterTrigger(id);
}

// Status and diagnostics
size_t TriggerHandler::GetTriggerCount() const {
    return triggerCount_;
}

bool TriggerHandler::HasActiveTriggers() const {
    for (size_t i = 0; i < triggerCount_; i++) {
        if (triggers_[i].isActive) {
            return true;
        }
    }
    return false;
}

void TriggerHandler::PrintTriggerStatus() const {
    log_i("TriggerHandler Status:");
    log_i("  Total triggers: %d", triggerCount_);
    log_i("  Priority states: CRITICAL=%s, IMPORTANT=%s, NORMAL=%s",
          priorityActive_[0] ? "active" : "inactive",
          priorityActive_[1] ? "active" : "inactive", 
          priorityActive_[2] ? "active" : "inactive");
    
    for (size_t i = 0; i < triggerCount_; i++) {
        const Trigger& trigger = triggers_[i];
        log_i("  Trigger[%d]: id='%s', type=%d, priority=%d, active=%s",
              i, trigger.id, static_cast<int>(trigger.type), 
              static_cast<int>(trigger.priority), trigger.isActive ? "yes" : "no");
    }
}

// Priority control methods (for external coordination)
void TriggerHandler::SetHigherPriorityActive(Priority priority) {
    UpdatePriorityState(priority, true);
}

void TriggerHandler::ClearHigherPriorityActive(Priority priority) {
    UpdatePriorityState(priority, false);
}