#include "sensors/lock_sensor.h"

// Constructors and Destructors

/// @brief Constructor for LockSensor
LockSensor::LockSensor()
{
}

// Core Functionality Methods

/// @brief Initialize the lock sensor hardware
void LockSensor::init()
{
    log_d("Initializing lock sensor on GPIO %d", gpio_pins::LOCK);
    pinMode(gpio_pins::LOCK, INPUT_PULLDOWN);
}

/// @brief Get the current lock status reading
/// @return Current lock status (true if engaged, false if disengaged)
Reading LockSensor::GetReading()
{
    bool isLockEngaged = digitalRead(gpio_pins::LOCK);
    log_d("Lock sensor reading: %s (pin %d %s)", 
          isLockEngaged ? "engaged" : "disengaged",
          gpio_pins::LOCK,
          isLockEngaged ? "HIGH" : "LOW");
    
    return Reading{isLockEngaged};
}