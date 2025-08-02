#pragma once

#include "interfaces/i_gpio_provider.h"
#include <map>

class MockGpioProvider : public IGpioProvider {
private:
    std::map<int, bool> digitalValues;
    std::map<int, uint16_t> analogValues;
    std::map<int, int> pinModes;
    std::map<int, bool> interrupts;

public:
    // IGpioProvider interface - matches actual interface
    void pinMode(int pin, int mode) override {
        pinModes[pin] = mode;
    }
    
    bool digitalRead(int pin) override {
        return digitalValues.count(pin) ? digitalValues[pin] : false;
    }
    
    uint16_t analogRead(int pin) override {
        return analogValues.count(pin) ? analogValues[pin] : 0;
    }

    // Mock-specific methods for test setup
    void setDigitalValue(int pin, bool value) {
        digitalValues[pin] = value;
    }
    
    void setAnalogValue(int pin, uint16_t value) {
        analogValues[pin] = value;
    }
    
    int getPinMode(int pin) {
        return pinModes.count(pin) ? pinModes[pin] : -1;
    }
    
    void setup_pin(int pin, int mode) {
        pinMode(pin, mode);
    }
    
    // Missing methods used by tests
    void setPinValue(uint8_t pin, int value) {
        if (pinModes[pin] == INPUT || pinModes[pin] == INPUT_PULLUP || pinModes[pin] == INPUT_PULLDOWN) {
            // For digital pins, convert value to boolean
            digitalValues[pin] = (value != 0);
        } else {
            // For analog pins, store as analog value
            analogValues[pin] = (uint16_t)value;
        }
    }
    
    void triggerInterrupt(uint8_t pin) {
        interrupts[pin] = true;
    }
    
    bool hasInterrupt(uint8_t pin) {
        return interrupts.count(pin) && interrupts[pin];
    }
    
    void reset() {
        digitalValues.clear();
        analogValues.clear();
        pinModes.clear();
        interrupts.clear();
    }
};