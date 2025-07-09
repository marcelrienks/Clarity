#include "sensors/key_sensor.h"

KeySensor::KeySensor()
{

}

void KeySensor::init()
{

}

/// @brief Get the current key reading
/// @return The current key reading (true if key is present, false otherwise)
Reading KeySensor::get_reading()
{    
    // Get the last digit (0-9) to create a 10-second cycle
    uint32_t last_digit = (millis() / 1000) % 10;
    
    // Return true only if last digit equals 1, 2, or 3
    bool key_present = (last_digit == 1) || (last_digit == 2) || (last_digit == 3);
    
    return key_present;
}