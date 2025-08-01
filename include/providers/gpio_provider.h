#pragma once

#include "interfaces/i_gpio_provider.h"
#include <Arduino.h>

/// @brief Hardware implementation of GPIO provider
/// @details Provides concrete implementation for GPIO operations
/// using the Arduino framework
class GpioProvider : public IGpioProvider
{
public:
    /// @brief Read digital value from a GPIO pin
    /// @param pin GPIO pin number
    /// @return true if pin is HIGH, false if LOW
    bool digitalRead(int pin) override;

    /// @brief Read analog value from an ADC pin
    /// @param pin ADC pin number
    /// @return ADC reading (0-4095 for 12-bit ADC)
    uint16_t analogRead(int pin) override;

    /// @brief Configure pin mode
    /// @param pin GPIO pin number
    /// @param mode Pin mode (INPUT, OUTPUT, INPUT_PULLUP, etc.)
    void pinMode(int pin, int mode) override;
};