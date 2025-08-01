#include "providers/esp32_gpio_provider.h"

bool Esp32GpioProvider::digitalRead(int pin)
{
    return ::digitalRead(pin);
}

void Esp32GpioProvider::digitalWrite(int pin, bool value)
{
    ::digitalWrite(pin, value ? HIGH : LOW);
}

uint16_t Esp32GpioProvider::analogRead(int pin)
{
    return ::analogRead(pin);
}

void Esp32GpioProvider::pinMode(int pin, int mode)
{
    ::pinMode(pin, mode);
}