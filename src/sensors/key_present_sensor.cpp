#include "sensors/key_present_sensor.h"
#include "utilities/logging.h"
#include <Arduino.h>
#include <esp32-hal-log.h>

KeyPresentSensor::KeyPresentSensor(IGpioProvider* gpioProvider)
    : gpioProvider_(gpioProvider)
{
}

KeyPresentSensor::~KeyPresentSensor()
{
    if (gpioProvider_ && initialized_) {
        // Clean up GPIO interrupt if attached
        gpioProvider_->DetachInterrupt(gpio_pins::KEY_PRESENT);
    }
}

void KeyPresentSensor::Init()
{
    if (!gpioProvider_) {
        log_e("KeyPresentSensor: GPIO provider is null");
        return;
    }

    // Configure GPIO 25 as input with pull-down resistor
    gpioProvider_->PinMode(gpio_pins::KEY_PRESENT, INPUT_PULLDOWN);
    
    // Initialize the sensor state
    previousState_ = readKeyPresentState();
    initialized_ = true;
    
    log_i("KeyPresentSensor initialization completed on GPIO %d, initial state: %s", 
          gpio_pins::KEY_PRESENT, previousState_ ? "PRESENT" : "NOT_PRESENT");
}

Reading KeyPresentSensor::GetReading()
{
    bool currentState = readKeyPresentState();
    log_v("KeyPresentSensor reading: %s", currentState ? "PRESENT" : "NOT_PRESENT");
    return Reading(currentState);
}

bool KeyPresentSensor::GetKeyPresentState()
{
    return readKeyPresentState();
}

bool KeyPresentSensor::HasStateChanged()
{
    log_v("HasStateChanged() called");
    if (!initialized_) {
        log_w("KeyPresentSensor: HasStateChanged called before initialization");
        return false;
    }
    
    bool currentState = readKeyPresentState();
    bool changed = DetectChange(currentState, previousState_);
    
    if (changed) {
        log_t("KeyPresentSensor state changed: %s -> %s", 
              previousState_ ? "PRESENT" : "NOT_PRESENT",
              currentState ? "PRESENT" : "NOT_PRESENT");
    }
    
    return changed;
}

void KeyPresentSensor::OnInterruptTriggered()
{
    log_t("KeyPresentSensor interrupt triggered - key present state changed");
    // Additional sensor-specific interrupt handling can be added here
}

bool KeyPresentSensor::readKeyPresentState()
{
    if (!gpioProvider_) {
        log_w("KeyPresentSensor: GPIO provider is null, returning false");
        return false;
    }
    
    // Read GPIO 25 state - HIGH means key is present
    bool state = gpioProvider_->DigitalRead(gpio_pins::KEY_PRESENT);
    return state;
}