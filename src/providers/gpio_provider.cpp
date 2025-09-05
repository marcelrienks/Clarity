#include "providers/gpio_provider.h"
#include <esp32-hal-log.h>

bool GpioProvider::DigitalRead(int pin)
{
    bool value = ::digitalRead(pin);
    return value;
}

uint16_t GpioProvider::AnalogRead(int pin)
{
    uint16_t value = ::analogRead(pin);
    return value;
}

void GpioProvider::PinMode(int pin, int mode)
{
    log_v("PinMode() called");
    ::pinMode(pin, mode);
}

void GpioProvider::AttachInterrupt(int pin, void (*callback)(), int mode)
{
    log_v("AttachInterrupt() called");
    log_i("GPIO %d interrupt attached", pin);
    ::attachInterrupt(digitalPinToInterrupt(pin), callback, mode);
    attachedInterrupts[pin] = true;
}

void GpioProvider::DetachInterrupt(int pin)
{
    log_v("DetachInterrupt() called");
    log_i("GPIO %d interrupt detached", pin);
    ::detachInterrupt(digitalPinToInterrupt(pin));
    attachedInterrupts[pin] = false;
}

bool GpioProvider::HasInterrupt(int pin)
{
    log_v("HasInterrupt() called");
    return attachedInterrupts.count(pin) && attachedInterrupts[pin];
}