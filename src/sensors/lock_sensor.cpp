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
    log_d("...");
    
    // Simulate lock status with a different pattern than key sensor
    // Lock is engaged (true) during seconds 4, 5, 6, 7 of every 10-second cycle
    uint32_t cycle_position = (millis() / 1000) % 10;
    bool is_lock_engaged = (cycle_position >= 4 && cycle_position <= 7);
    
    log_v("Lock sensor reading: %s", is_lock_engaged ? "engaged" : "disengaged");
    return Reading{is_lock_engaged};
}