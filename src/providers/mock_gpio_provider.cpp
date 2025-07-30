#include "providers/mock_gpio_provider.h"

bool MockGpioProvider::digitalRead(int pin)
{
    auto it = digitalPins_.find(pin);
    return it != digitalPins_.end() ? it->second : false;
}

void MockGpioProvider::digitalWrite(int pin, bool value)
{
    digitalPins_[pin] = value;
}

uint16_t MockGpioProvider::analogRead(int pin)
{
    auto it = analogPins_.find(pin);
    return it != analogPins_.end() ? it->second : 0;
}

void MockGpioProvider::pinMode(int pin, int mode)
{
    pinModes_[pin] = mode;
}

void MockGpioProvider::setDigitalPin(int pin, bool value)
{
    digitalPins_[pin] = value;
}

void MockGpioProvider::setAnalogPin(int pin, uint16_t value)
{
    analogPins_[pin] = value;
}

int MockGpioProvider::getPinMode(int pin) const
{
    auto it = pinModes_.find(pin);
    return it != pinModes_.end() ? it->second : -1;
}

void MockGpioProvider::reset()
{
    digitalPins_.clear();
    analogPins_.clear();
    pinModes_.clear();
}