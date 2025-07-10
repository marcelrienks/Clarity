#include "sensors/key_sensor.h"

// Constructors and Destructors

KeySensor::KeySensor()
{

}

// Core Functionality Methods

/// @brief Get the current key reading
/// @return The current key reading (true if key is present, false otherwise)
Reading KeySensor::get_reading()
{    
    // Get the last digit (0-9) to create a 10-second cycle
    uint32_t last_digit = (millis() / 1000) % 10;
    
    // Return true only if last digit equals 1, 2, or 3
    bool key_present = (last_digit == 1) || (last_digit == 2) || (last_digit == 3);
    log_d("key_present is: %d", key_present);
    return key_present;
}

void KeySensor::init()
{

}