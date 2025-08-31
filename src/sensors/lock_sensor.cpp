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
        log_d("Initializing lock sensor on GPIO %d", gpio_pins::LOCK);
        initialized = true;
    }
    gpioProvider_->PinMode(gpio_pins::LOCK, INPUT_PULLDOWN);
    
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
        log_d("Lock sensor reading: %s (pin %d %s)", isLockEngaged ? "engaged" : "disengaged", gpio_pins::LOCK,
              isLockEngaged ? "HIGH" : "LOW");
        lastState = isLockEngaged;
        firstRead = false;
    }

    return Reading{isLockEngaged};
}

bool LockSensor::readLockState()
{
    log_v("readLockState() called");
    return gpioProvider_->DigitalRead(gpio_pins::LOCK);
}

bool LockSensor::HasStateChanged()
{
    log_v("HasStateChanged() called");
    
    bool currentState = readLockState();
    bool changed = DetectChange(currentState, previousLockState_);
    
    if (changed)
    {
        log_d("Lock state changed from %s to %s", 
              previousLockState_ ? "engaged" : "disengaged",
              currentState ? "engaged" : "disengaged");
              
        // In simplified system, determine which interrupt should be triggered
        if (currentState) 
        {
            // Lock is now engaged - trigger lock_engaged interrupt
            log_d("Lock engaged - should trigger 'lock_engaged' interrupt");
            triggerInterruptId_ = "lock_engaged";
        }
        else 
        {
            // Lock is now disengaged - trigger lock_disengaged interrupt  
            log_d("Lock disengaged - should trigger 'lock_disengaged' interrupt");
            triggerInterruptId_ = "lock_disengaged";
        }
    }
    else
    {
        triggerInterruptId_ = nullptr; // No interrupt to trigger
    }
    
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