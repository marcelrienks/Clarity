#pragma once

#include "interfaces/i_gpio_provider.h"
#include <Arduino.h>

/**
 * @class GpioProvider
 * @brief Hardware implementation of GPIO provider using Arduino framework
 * 
 * @details This class provides the concrete implementation of IGpioProvider
 * interface, handling all GPIO operations through the Arduino framework.
 * It manages digital I/O, analog-to-digital conversion, and pin configuration
 * for the ESP32 hardware.
 * 
 * @hardware_abstraction Bridges IGpioProvider interface to Arduino framework
 * @target_platform ESP32-WROOM-32 with NodeMCU-32S development board
 * @adc_resolution 12-bit ADC providing 0-4095 range for analog readings
 * 
 * @gpio_capabilities:
 * - Digital I/O: Read/write digital pin states
 * - Analog input: 12-bit ADC readings from analog pins
 * - Pin configuration: Mode setting (INPUT, OUTPUT, INPUT_PULLUP)
 * 
 * @pin_constraints:
 * - GPIO 36 & 39: ADC input only (no pull-up/pull-down)
 * - GPIO 25-27: Full-featured general purpose I/O
 * - GPIO 33: Digital input with pull-up capability
 * 
 * @dependency_injection:
 * - Implements IGpioProvider for testability
 * - Injectable into sensors and other hardware-dependent classes
 * - Supports mocking for unit tests
 * 
 * @context This is the main hardware interface for GPIO operations.
 * All sensors use this provider to read from physical pins on the ESP32.
 * It abstracts Arduino framework calls behind a clean interface.
 */
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