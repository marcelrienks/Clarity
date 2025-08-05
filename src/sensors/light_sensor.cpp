#include "sensors/light_sensor.h"
#include <Arduino.h>
#include <esp32-hal-log.h>

// Constructors and Destructors

/// @brief Constructor for LightSensor
LightSensor::LightSensor(IGpioProvider* gpioProvider) : gpioProvider_(gpioProvider)
{
}

// Core Functionality Methods

/// @brief Initialize the lights sensor hardware
void LightSensor::init()
{
    // Configure GPIO pin for digital input (safe to call multiple times)
    log_d("Initializing lights sensor on GPIO %d", gpio_pins::LIGHTS);

    gpioProvider_->pinMode(gpio_pins::LIGHTS, INPUT_PULLDOWN);
}

/// @brief Get the current lights reading
/// @return Boolean indicating lights state (true=on, false=off)
Reading LightSensor::getReading()
{
    bool lightsOn = getLightsState();
    return Reading{lightsOn};
}

/// @brief Get current lights state directly (for triggers)
/// @return true if lights are on, false if lights are off
bool LightSensor::getLightsState()
{
    bool lightsOn = gpioProvider_->digitalRead(gpio_pins::LIGHTS);
    
    // Only log state changes to reduce log spam during polling
    static bool lastState = false;
    static bool firstRead = true;
    
    if (firstRead || lightsOn != lastState) {
        log_d("Lights sensor reading: %s (pin %d %s)", 
              lightsOn ? "ON" : "OFF",
              gpio_pins::LIGHTS,
              lightsOn ? "HIGH" : "LOW");
        lastState = lightsOn;
        firstRead = false;
    }
    
    return lightsOn;
}