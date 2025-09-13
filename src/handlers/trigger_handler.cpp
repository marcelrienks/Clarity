#include "handlers/trigger_handler.h"
#include "managers/error_manager.h"
#include "sensors/gpio_sensor.h"
#include "hardware/gpio_pins.h"
#include "utilities/logging.h"
#include <Arduino.h>
#include <algorithm>
#include <cstring>

#include "esp32-hal-log.h"

TriggerHandler::TriggerHandler(IGpioProvider* gpioProvider) 
    : gpioProvider_(gpioProvider),
      triggerCount_(0)
{
    log_d("TriggerHandler() constructor called");
    
    // Initialize priority tracking
    for (int i = 0; i < 3; i++) {
        priorityActive_[i] = false;
    }
    
    // Create and initialize GPIO sensors owned by this handler
    if (gpioProvider_) {

        // Create GPIO sensors using generic GpioSensor with predefined configurations
        keyPresentSensor_ = std::make_unique<GpioSensor>(sensor_configs::KEY_PRESENT, gpioProvider_);
        keyNotPresentSensor_ = std::make_unique<GpioSensor>(sensor_configs::KEY_NOT_PRESENT, gpioProvider_);
        lockSensor_ = std::make_unique<GpioSensor>(sensor_configs::LOCK, gpioProvider_);
        lightsSensor_ = std::make_unique<GpioSensor>(sensor_configs::LIGHTS, gpioProvider_);

#ifdef CLARITY_DEBUG
        // Create debug error sensor (debug builds only)
        debugErrorSensor_ = std::make_unique<GpioSensor>(sensor_configs::DEBUG_ERROR, gpioProvider_);
#endif

        // Initialize all sensors
        keyPresentSensor_->Init();
        keyNotPresentSensor_->Init();
        lockSensor_->Init();
        lightsSensor_->Init();

#ifdef CLARITY_DEBUG
        debugErrorSensor_->Init();
        log_i("TriggerHandler created and initialized 5 GPIO sensors using generic GpioSensor: KeyPresent, KeyNotPresent, Lock, Lights, DebugError (debug build)");
#else
        log_i("TriggerHandler created and initialized 4 GPIO sensors using generic GpioSensor: KeyPresent, KeyNotPresent, Lock, Lights");
#endif
    }
    else {
        log_w("TriggerHandler created without GPIO provider - sensors not initialized");
    }
}

TriggerHandler::~TriggerHandler() {
    log_d("TriggerHandler destructor called");
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
    
    // Extra debugging for lock trigger
    
    // Check if sensor state has changed
    bool hasChanged = trigger.sensor->HasStateChanged();
    
    if (!hasChanged) {
        return; // No change, nothing to do
    }
    
    // Get current sensor state
    bool sensorActive = IsSensorActive(trigger);
    bool wasActive = trigger.isActive;
    
    // Debug logging for KeyPresent trigger specifically
    if (strcmp(trigger.id, "key_present") == 0) {
        log_t("KeyPresent trigger evaluation: hasChanged=%s, wasActive=%s, sensorActive=%s", 
              hasChanged ? "true" : "false",
              wasActive ? "true" : "false", 
              sensorActive ? "true" : "false");
    }
    
    
    // Determine if trigger should activate or deactivate
    if (!wasActive && sensorActive) {
        // CRITICAL ARCHITECTURE: Priority-based execution with state preservation
        // 1. isActive flag ALWAYS updated when sensor changes (regardless of priority)
        // 2. Function execution BLOCKED by higher priority but state preserved
        // 3. This preserves trigger state for restoration when higher priority clears
        // 4. Error panel active check prevents UI corruption during error display

        // Activation Flow with Priority Logic (as per interrupt-architecture.md)
        // ALWAYS set isActive when sensor becomes active, regardless of priority
        trigger.isActive = true;
        UpdatePriorityState(trigger.priority, true);

        // But only execute the activate function if not blocked by higher priority or error panel
        if (!HasHigherPriorityActive(trigger.priority)) {
            // Check if error panel is active - if so, suppress trigger execution but keep state
            if (ErrorManager::Instance().IsErrorPanelActive()) {
            } else {
                if (trigger.activateFunc) {
                    trigger.activateFunc();
                }
            }
        } else {
        }
    }
    else if (wasActive && !sensorActive && ShouldDeactivate(trigger)) {
        // Deactivation Flow with Type-Based Restoration (as per interrupt-architecture.md)
        
        // First, mark this trigger as inactive and update priority state
        trigger.isActive = false;
        UpdatePriorityState(trigger.priority, false);
        
        // Find highest priority active trigger of same type (don't exclude any priority)
        Trigger* sameTypeTrigger = FindHighestPrioritySameType(trigger.type);
        if (sameTypeTrigger) {
            // Same-type trigger activation
            sameTypeTrigger->ExecuteActivate();
        } else {
            // Only call deactivate function (which does restoration) if no same-type triggers found
            if (trigger.deactivateFunc) {
                trigger.deactivateFunc();
            }
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
    // Priority comparison: Higher numeric values = higher priority
    // CRITICAL=2 > IMPORTANT=1 > NORMAL=0 (reverse of typical enum ordering)
    // This allows simple numeric comparison for priority blocking logic

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
    // Note: excludePriority is kept for API compatibility but not used in new logic
    Trigger* toRestore = FindHighestPrioritySameType(type);
    if (toRestore && toRestore->isActive && toRestore->activateFunc) {
        toRestore->activateFunc();
    }
}

Trigger* TriggerHandler::FindHighestPrioritySameType(TriggerType type) {
    Trigger* highest = nullptr;
    int highestPriorityValue = -1;  // Track numeric priority value
    
    for (size_t i = 0; i < triggerCount_; i++) {
        Trigger& trigger = triggers_[i];
        // Find active triggers of the same type
        if (trigger.type == type && trigger.isActive) {
            int triggerPriorityValue = static_cast<int>(trigger.priority);
            if (triggerPriorityValue > highestPriorityValue) {
                highest = &trigger;
                highestPriorityValue = triggerPriorityValue;
            }
        }
    }
    
    if (highest) {
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
    if (!trigger.sensor) {
        return false;
    }
    
    // Get sensor reading to determine if it's active
    Reading reading = trigger.sensor->GetReading();
    
    // Check if reading is a boolean (GPIO sensors)
    if (std::holds_alternative<bool>(reading)) {
        return std::get<bool>(reading);
    }
    
    // For non-boolean sensors, consider them inactive
    return false;
}

Trigger* TriggerHandler::FindTrigger(const char* id) {
    for (size_t i = 0; i < triggerCount_; i++) {
        if (strcmp(triggers_[i].id, id) == 0) {
            return &triggers_[i];
        }
    }
    return nullptr;
}

// Handler interface implementation complete

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