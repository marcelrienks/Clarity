#include "sensors/lock_sensor.h"
#include <Arduino.h>
#include <esp32-hal-log.h>

// Constructors and Destructors

/// @brief Constructor for LockSensor
LockSensor::LockSensor(IGpioProvider *gpioProvider) : gpioProvider_(gpioProvider)
{
    log_v("LockSensor() constructor called");
}

// Core Functionality Methods

/// @brief Initialize the lock sensor hardware
void LockSensor::Init()
{
    log_v("Init() called");
    static bool initialized = false;
    if (!initialized)
    {
        log_d("Initializing lock sensor on GPIO %d", gpio_pins::LOCK);
        initialized = true;
    }
    gpioProvider_->PinMode(gpio_pins::LOCK, INPUT_PULLDOWN);
    
    log_i("LockSensor initialization completed on GPIO %d", gpio_pins::LOCK);
}

/// @brief Get the current lock status reading
/// @return Current lock status (true if engaged, false if disengaged)
Reading LockSensor::GetReading()
{
    log_v("GetReading() called");
    bool isLockEngaged = gpioProvider_->DigitalRead(gpio_pins::LOCK);

    // Only log state changes to reduce log spam during polling
    static bool lastState = false;
    static bool firstRead = true;

    if (firstRead || isLockEngaged != lastState)
    {
        log_d("Lock sensor reading: %s (pin %d %s)", isLockEngaged ? "engaged" : "disengaged", gpio_pins::LOCK,
              isLockEngaged ? "HIGH" : "LOW");
        lastState = isLockEngaged;
        firstRead = false;
    }

    return Reading{isLockEngaged};
}