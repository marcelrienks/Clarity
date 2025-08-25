#include "sensors/key_not_present_sensor.h"
#include <Arduino.h>
#include <esp32-hal-log.h>

KeyNotPresentSensor::KeyNotPresentSensor(IGpioProvider* gpioProvider)
    : gpioProvider_(gpioProvider)
{
    log_d("KeyNotPresentSensor created for GPIO %d", gpio_pins::KEY_NOT_PRESENT);
}

KeyNotPresentSensor::~KeyNotPresentSensor()
{
    if (gpioProvider_ && initialized_) {
        // Clean up GPIO interrupt if attached
        gpioProvider_->DetachInterrupt(gpio_pins::KEY_NOT_PRESENT);
        log_d("KeyNotPresentSensor destructor: GPIO %d interrupt detached", gpio_pins::KEY_NOT_PRESENT);
    }
}

void KeyNotPresentSensor::Init()
{
    if (!gpioProvider_) {
        log_e("KeyNotPresentSensor: GPIO provider is null");
        return;
    }

    // Configure GPIO 26 as input with pull-down resistor
    gpioProvider_->PinMode(gpio_pins::KEY_NOT_PRESENT, INPUT_PULLDOWN);
    
    // Initialize the sensor state
    previousState_ = readKeyNotPresentState();
    initialized_ = true;
    
    log_d("KeyNotPresentSensor initialized on GPIO %d, initial state: %s", 
          gpio_pins::KEY_NOT_PRESENT, previousState_ ? "NOT_PRESENT" : "PRESENT");
}

Reading KeyNotPresentSensor::GetReading()
{
    bool currentState = readKeyNotPresentState();
    log_v("KeyNotPresentSensor reading: %s", currentState ? "NOT_PRESENT" : "PRESENT");
    return Reading(currentState);
}

bool KeyNotPresentSensor::GetKeyNotPresentState()
{
    return readKeyNotPresentState();
}

bool KeyNotPresentSensor::HasStateChanged()
{
    if (!initialized_) {
        log_w("KeyNotPresentSensor: HasStateChanged called before initialization");
        return false;
    }
    
    bool currentState = readKeyNotPresentState();
    bool changed = DetectChange(currentState, previousState_);
    
    if (changed) {
        log_i("KeyNotPresentSensor state changed: %s -> %s", 
              previousState_ ? "NOT_PRESENT" : "PRESENT",
              currentState ? "NOT_PRESENT" : "PRESENT");
    }
    
    return changed;
}

void KeyNotPresentSensor::OnInterruptTriggered()
{
    log_i("KeyNotPresentSensor interrupt triggered - key not present state changed");
    // Additional sensor-specific interrupt handling can be added here
}

bool KeyNotPresentSensor::readKeyNotPresentState()
{
    if (!gpioProvider_) {
        log_w("KeyNotPresentSensor: GPIO provider is null, returning false");
        return false;
    }
    
    // Read GPIO 26 state - HIGH means key is not present
    bool state = gpioProvider_->DigitalRead(gpio_pins::KEY_NOT_PRESENT);
    log_v("KeyNotPresentSensor GPIO %d read: %s", gpio_pins::KEY_NOT_PRESENT, state ? "HIGH" : "LOW");
    return state;
}