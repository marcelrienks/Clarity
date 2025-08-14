#include "sensors/lights_sensor.h"
#include <Arduino.h>
#include <esp32-hal-log.h>

// Constructors and Destructors

/// @brief Constructor for LightsSensor
LightsSensor::LightsSensor(IGpioProvider *gpioProvider) : gpioProvider_(gpioProvider)
{
}

// Core Functionality Methods

/// @brief Initialize the lights sensor hardware
void LightsSensor::Init()
{
    // Configure GPIO pin for digital input (safe to call multiple times)
    log_d("Initializing lights sensor on GPIO %d", gpio_pins::LIGHTS);

    gpioProvider_->PinMode(gpio_pins::LIGHTS, INPUT_PULLDOWN);
}

/// @brief Get the current lights reading
/// @return Boolean indicating lights state (true=on, false=off)
Reading LightsSensor::GetReading()
{
    bool lightsOn = GetLightsState();
    return Reading{lightsOn};
}

/// @brief Get current lights state directly (for triggers)
/// @return true if lights are on, false if lights are off
bool LightsSensor::GetLightsState()
{
    bool lightsOn = gpioProvider_->DigitalRead(gpio_pins::LIGHTS);

    // Only log state changes to reduce log spam during polling
    static bool lastState = false;
    static bool firstRead = true;

    if (firstRead || lightsOn != lastState)
    {
        log_d("Lights sensor reading: %s (pin %d %s)", lightsOn ? "ON" : "OFF", gpio_pins::LIGHTS,
              lightsOn ? "HIGH" : "LOW");
        lastState = lightsOn;
        firstRead = false;
    }

    return lightsOn;
}