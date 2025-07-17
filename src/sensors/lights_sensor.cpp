#include "sensors/lights_sensor.h"
#include <Arduino.h>

// Constructors and Destructors

/// @brief Constructor for LightsSensor
LightsSensor::LightsSensor()
{
}

// Core Functionality Methods

/// @brief Initialize the lights sensor hardware
void LightsSensor::init()
{
    // Configure GPIO 32 for digital input (safe to call multiple times)
    log_d("Initializing lights sensor on GPIO %d", GpioPins::LIGHTS);
    
    pinMode(GpioPins::LIGHTS, INPUT_PULLDOWN);
}

/// @brief Get the current lights switch reading
/// @return bool indicating lights state (true = lights on/night, false = lights off/day)
Reading LightsSensor::get_reading()
{
    bool pin_high = digitalRead(GpioPins::LIGHTS);
    
    if (pin_high)
    {
        log_d("Lights switch: Lights on - Night theme requested (GPIO %d HIGH)", GpioPins::LIGHTS);
        return true; // Lights on - Night theme
    }
    else
    {
        log_d("Lights switch: Lights off - Day theme requested (GPIO %d LOW)", GpioPins::LIGHTS);
        return false; // Lights off - Day theme
    }
}