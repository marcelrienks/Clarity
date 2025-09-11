#include "sensors/lock_sensor.h"
#include <Arduino.h>
#include <esp32-hal-log.h>

// Constructors and Destructors

/// @brief Constructor for LockSensor
LockSensor::LockSensor(IGpioProvider *gpioProvider) : gpioProvider_(gpioProvider)
{
    log_v("LockSensor() constructor called");
}

// Core Functionality Methods

/// @brief Initialize the lock sensor hardware
void LockSensor::Init()
{
    log_v("Init() called");
    static bool initialized = false;
    if (!initialized)
    {
        initialized = true;
    }
    gpioProvider_->PinMode(gpio_pins::LOCK, INPUT_PULLDOWN);
    
    // Initialize previous state to current GPIO state
    previousLockState_ = readLockState();
    
    // Interrupt registration is now handled centrally in ManagerFactory
    
    log_i("LockSensor initialization completed on GPIO %d with coordinated interrupts", gpio_pins::LOCK);
}

/// @brief Get the current lock status reading
/// @return Current lock status (true if engaged, false if disengaged)
Reading LockSensor::GetReading()
{
    log_v("GetReading() called");
    bool isLockEngaged = gpioProvider_->DigitalRead(gpio_pins::LOCK);

    // Only log state changes to reduce log spam during polling
    static bool lastState = false;
    static bool firstRead = true;

    if (firstRead || isLockEngaged != lastState)
    {
        log_v("Lock sensor GPIO %d state: %s", gpio_pins::LOCK,
              isLockEngaged ? "HIGH" : "LOW");
        lastState = isLockEngaged;
        firstRead = false;
    }

    return Reading{isLockEngaged};
}

bool LockSensor::readLockState()
{
    return gpioProvider_->DigitalRead(gpio_pins::LOCK);
}

bool LockSensor::HasStateChanged()
{
    log_v("HasStateChanged() called");
    
    bool currentState = readLockState();
    
    // Store previous state for logging before DetectChange updates it
    bool oldPreviousState = previousLockState_;
    
    // Detailed timing and state debugging
    static unsigned long lastLogTime = 0;
    unsigned long currentTime = millis();
    
    // Log every 2 seconds or when state actually differs
    if (currentTime - lastLogTime > 2000 || currentState != previousLockState_) {
        log_v("LOCK SENSOR DEBUG [%lu ms]: GPIO=%s, current=%s, previous=%s, initialized=%s",
              currentTime,
              gpioProvider_->DigitalRead(gpio_pins::LOCK) ? "HIGH" : "LOW",
              currentState ? "HIGH" : "LOW", 
              previousLockState_ ? "HIGH" : "LOW",
              initialized_ ? "true" : "false");
        lastLogTime = currentTime;
    }
    
    bool changed = DetectChange(currentState, previousLockState_);
    
    if (changed)
    {
        log_i("LOCK SENSOR STATE CHANGE [%lu ms]: %s -> %s", 
              currentTime,
              oldPreviousState ? "HIGH/engaged" : "LOW/disengaged",
              currentState ? "HIGH/engaged" : "LOW/disengaged");
              
        // In simplified system, determine which interrupt should be triggered
        if (currentState) 
        {
            // Lock is now engaged - trigger lock_engaged interrupt
            triggerInterruptId_ = "lock_engaged";
        }
        else 
        {
            // Lock is now disengaged - trigger lock_disengaged interrupt  
            triggerInterruptId_ = "lock_disengaged";
        }
    }
    else
    {
        triggerInterruptId_ = nullptr; // No interrupt to trigger
    }
    
    log_v("LOCK SENSOR DEBUG: DetectChange returned %s, previousLockState_ now=%s",
          changed ? "true" : "false",
          previousLockState_ ? "HIGH" : "LOW");
    
    return changed;
}

const char* LockSensor::GetTriggerInterruptId() const
{
    return triggerInterruptId_;
}

void LockSensor::OnInterruptTriggered()
{
    log_v("OnInterruptTriggered() called");
    
    bool currentState = readLockState();
    log_i("Lock sensor interrupt triggered - current state: %s", 
          currentState ? "engaged" : "disengaged");
    
    // Example custom behavior based on lock state
    if (currentState)
    {
        log_i("Lock engaged - system could activate security panels");
        // Could trigger specific panel loading, theme changes, etc.
    }
    else
    {
        log_i("Lock disengaged - system could enter normal mode");
        // Could trigger different panels, unlock features, etc.
    }
}