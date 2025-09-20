#pragma once

#include "interfaces/i_gpio_provider.h"
#include <Arduino.h>
#include <map>

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
 * - Interrupt handling: Attach/detach interrupts with callback functions
 *
 * @pin_constraints:
 * - GPIO 36 & 39: ADC input only (no pull-up/pull-down)
 * - GPIO 25-27: Full-featured general purpose I/O
 * - GPIO 32 & 33: Digital input with pull-up capability
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
  private:
    // Use fixed array for ESP32 GPIO pins (0-39) - much faster than std::map
    static constexpr int MAX_GPIO_PIN = 40;

  public:
    bool DigitalRead(int pin) override;

    uint16_t AnalogRead(int pin) override;

    void PinMode(int pin, int mode) override;

    void AttachInterrupt(int pin, void (*callback)(), int mode) override;

    void DetachInterrupt(int pin) override;
};