#include "providers/gpio_provider.h"

bool GpioProvider::digitalRead(int pin)
{
    return ::digitalRead(pin);
}

uint16_t GpioProvider::analogRead(int pin)
{
    return ::analogRead(pin);
}

void GpioProvider::pinMode(int pin, int mode)
{
    ::pinMode(pin, mode);
}

void GpioProvider::attachInterrupt(int pin, void (*callback)(), int mode)
{
    ::attachInterrupt(digitalPinToInterrupt(pin), callback, mode);
    attachedInterrupts[pin] = true;
}

void GpioProvider::detachInterrupt(int pin)
{
    ::detachInterrupt(digitalPinToInterrupt(pin));
    attachedInterrupts[pin] = false;
}

bool GpioProvider::hasInterrupt(int pin)
{
    return attachedInterrupts.count(pin) && attachedInterrupts[pin];
}