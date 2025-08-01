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