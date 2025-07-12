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
    log_d("Initializing lock sensor on GPIO %d", GpioPins::LOCK);
    pinMode(GpioPins::LOCK, INPUT_PULLDOWN);
}

/// @brief Get the current lock status reading
/// @return Current lock status (true if engaged, false if disengaged)
Reading LockSensor::get_reading()
{
    bool is_lock_engaged = digitalRead(GpioPins::LOCK);
    log_d("Lock sensor reading: %s (pin %d %s)", 
          is_lock_engaged ? "engaged" : "disengaged",
          GpioPins::LOCK,
          is_lock_engaged ? "HIGH" : "LOW");
    
    return Reading{is_lock_engaged};
}