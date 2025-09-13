#include "providers/gpio_provider.h"
#include <esp32-hal-log.h>

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
    if (pin >= 0 && pin < MAX_GPIO_PIN) {
        log_i("GPIO %d interrupt attached", pin);
        ::attachInterrupt(digitalPinToInterrupt(pin), callback, mode);
        attachedInterrupts_[pin] = true;
    }
}

void GpioProvider::DetachInterrupt(int pin)
{
    if (pin >= 0 && pin < MAX_GPIO_PIN) {
        log_i("GPIO %d interrupt detached", pin);
        ::detachInterrupt(digitalPinToInterrupt(pin));
        attachedInterrupts_[pin] = false;
    }
}

bool GpioProvider::HasInterrupt(int pin)
{
    return (pin >= 0 && pin < MAX_GPIO_PIN) ? attachedInterrupts_[pin] : false;
}