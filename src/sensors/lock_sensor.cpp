#include "sensors/lock_sensor.h"
#include <Arduino.h>
#include <esp32-hal-log.h>

// Constructors and Destructors

/// @brief Constructor for LockSensor
LockSensor::LockSensor(IGpioProvider* gpioProvider) : gpioProvider_(gpioProvider)
{
}

// Core Functionality Methods

/// @brief Initialize the lock sensor hardware
void LockSensor::init()
{
    log_d("Initializing lock sensor on GPIO %d", gpio_pins::LOCK);
    gpioProvider_->pinMode(gpio_pins::LOCK, INPUT_PULLDOWN);
}

/// @brief Get the current lock status reading
/// @return Current lock status (true if engaged, false if disengaged)
Reading LockSensor::getReading()
{
    bool isLockEngaged = gpioProvider_->digitalRead(gpio_pins::LOCK);
    log_d("Lock sensor reading: %s (pin %d %s)", 
          isLockEngaged ? "engaged" : "disengaged",
          gpio_pins::LOCK,
          isLockEngaged ? "HIGH" : "LOW");
    
    return Reading{isLockEngaged};
}