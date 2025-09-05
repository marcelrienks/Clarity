#include "providers/gpio_provider.h"
#include <esp32-hal-log.h>

bool GpioProvider::DigitalRead(int pin)
{
    log_v("DigitalRead() called for pin %d", pin);
    bool value = ::digitalRead(pin);
    log_v("DigitalRead() pin %d returned: %d", pin, value);
    return value;
}

uint16_t GpioProvider::AnalogRead(int pin)
{
    log_v("AnalogRead() called for pin %d", pin);
    uint16_t value = ::analogRead(pin);
    log_v("AnalogRead() pin %d returned: %d", pin, value);
    return value;
}

void GpioProvider::PinMode(int pin, int mode)
{
    log_v("PinMode() called for pin %d, mode %d", pin, mode);
    ::pinMode(pin, mode);
}

void GpioProvider::AttachInterrupt(int pin, void (*callback)(), int mode)
{
    log_v("AttachInterrupt() called");
    ::attachInterrupt(digitalPinToInterrupt(pin), callback, mode);
    attachedInterrupts[pin] = true;
}

void GpioProvider::DetachInterrupt(int pin)
{
    log_v("DetachInterrupt() called");
    ::detachInterrupt(digitalPinToInterrupt(pin));
    attachedInterrupts[pin] = false;
}

bool GpioProvider::HasInterrupt(int pin)
{
    log_v("HasInterrupt() called");
    return attachedInterrupts.count(pin) && attachedInterrupts[pin];
}