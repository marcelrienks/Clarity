#include "providers/gpio_provider.h"

bool GpioProvider::DigitalRead(int pin)
{
    return ::digitalRead(pin);
}

uint16_t GpioProvider::AnalogRead(int pin)
{
    return ::analogRead(pin);
}

void GpioProvider::PinMode(int pin, int mode)
{
    ::pinMode(pin, mode);
}

void GpioProvider::AttachInterrupt(int pin, void (*callback)(), int mode)
{
    ::attachInterrupt(digitalPinToInterrupt(pin), callback, mode);
    attachedInterrupts[pin] = true;
}

void GpioProvider::DetachInterrupt(int pin)
{
    ::detachInterrupt(digitalPinToInterrupt(pin));
    attachedInterrupts[pin] = false;
}

bool GpioProvider::HasInterrupt(int pin)
{
    return attachedInterrupts.count(pin) && attachedInterrupts[pin];
}