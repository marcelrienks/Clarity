#include "sensors/lights_sensor.h"
#include "utilities/logging.h"
#include <Arduino.h>
#include <esp32-hal-log.h>

// Constructors and Destructors

/// @brief Constructor for LightsSensor
LightsSensor::LightsSensor(IGpioProvider *gpioProvider) : gpioProvider_(gpioProvider)
{
    log_v("LightsSensor() constructor called");
}

// Core Functionality Methods

/// @brief Initialize the lights sensor hardware
void LightsSensor::Init()
{
    log_v("Init() called");
    // Configure GPIO pin for digital input (safe to call multiple times)

    gpioProvider_->PinMode(gpio_pins::LIGHTS, INPUT_PULLDOWN);
    
    // Initialize the sensor state to avoid false change detection
    previousLightsState_ = readLightsState();
    
    // Interrupt registration is now handled centrally in ManagerFactory
    
    log_i("LightsSensor initialization completed on GPIO %d with coordinated interrupts", gpio_pins::LIGHTS);
}

/// @brief Get the current lights reading
/// @return Boolean indicating lights state (true=on, false=off)
Reading LightsSensor::GetReading()
{
    log_v("GetReading() called");
    bool lightsOn = GetLightsState();
    return Reading{lightsOn};
}

/// @brief Get current lights state directly (for triggers)
/// @return true if lights are on, false if lights are off
bool LightsSensor::GetLightsState()
{
    log_v("GetLightsState() called");
    bool lightsOn = gpioProvider_->DigitalRead(gpio_pins::LIGHTS);

    // Only log state changes to reduce log spam during polling
    static bool lastState = false;
    static bool firstRead = true;

    if (firstRead || lightsOn != lastState)
    {
        log_v("Lights sensor GPIO %d state: %s", gpio_pins::LIGHTS,
              lightsOn ? "HIGH" : "LOW");
        lastState = lightsOn;
        firstRead = false;
    }

    return lightsOn;
}

bool LightsSensor::readLightsState()
{
    return gpioProvider_->DigitalRead(gpio_pins::LIGHTS);
}

bool LightsSensor::HasStateChanged()
{
    log_v("HasStateChanged() called");
    
    bool currentState = readLightsState();
    bool previousState = previousLightsState_; // Save before DetectChange modifies it
    bool changed = DetectChange(currentState, previousLightsState_);
    
    if (changed)
    {
        log_t("Lights sensor state changed: %s -> %s", 
              previousState ? "ON" : "OFF",
              currentState ? "ON" : "OFF");
              
        // In simplified system, determine which interrupt should be triggered
        if (currentState) 
        {
            // Lights are now on - trigger lights_on interrupt
            triggerInterruptId_ = "lights_on";
        }
        else 
        {
            // Lights are now off - trigger lights_off interrupt  
            triggerInterruptId_ = "lights_off";
        }
    }
    else
    {
        triggerInterruptId_ = nullptr; // No interrupt to trigger
    }
    
    return changed;
}

const char* LightsSensor::GetTriggerInterruptId() const
{
    return triggerInterruptId_;
}

void LightsSensor::OnInterruptTriggered()
{
    log_v("OnInterruptTriggered() called");
    
    bool currentState = readLightsState();
    log_t("Lights sensor interrupt triggered - current state: %s", 
          currentState ? "ON" : "OFF");
    
    // Example custom behavior based on lights state
    if (currentState)
    {
        log_t("Lights turned on - system could switch to night theme");
        // Could trigger night theme, adjust brightness, etc.
    }
    else
    {
        log_t("Lights turned off - system could switch to day theme");
        // Could trigger day theme, increase brightness, etc.
    }
}