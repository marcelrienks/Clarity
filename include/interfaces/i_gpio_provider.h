#pragma once

#include <cstdint>

/// @brief Interface for GPIO hardware abstraction
/// @details Provides abstraction for digital and analog I/O operations
/// to enable testing and hardware independence
class IGpioProvider
{
public:
    virtual ~IGpioProvider() = default;

    /// @brief Read digital value from a GPIO pin
    /// @param pin GPIO pin number
    /// @return true if pin is HIGH, false if LOW
    virtual bool digitalRead(int pin) = 0;

    /// @brief Read analog value from an ADC pin
    /// @param pin ADC pin number
    /// @return ADC reading (0-4095 for 12-bit ADC)
    virtual uint16_t analogRead(int pin) = 0;

    /// @brief Configure pin mode
    /// @param pin GPIO pin number
    /// @param mode Pin mode (INPUT, OUTPUT, INPUT_PULLUP, etc.)
    virtual void pinMode(int pin, int mode) = 0;
};