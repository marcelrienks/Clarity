#include "sensors/lock_sensor.h"

// Constructors and Destructors
LockSensor::LockSensor()
{
}

// Core Functionality Methods
void LockSensor::init()
{
    log_d("...");
}

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