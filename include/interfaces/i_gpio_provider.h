#pragma once

#include <cstdint>

/**
 * @interface IGpioProvider
 * @brief Interface for GPIO hardware abstraction layer
 * 
 * @details This interface provides hardware abstraction for GPIO operations,
 * enabling dependency injection and testability. It abstracts digital I/O,
 * analog-to-digital conversion, and pin configuration functionality.
 * 
 * @hardware_abstraction Enables testing with mock implementations
 * @dependency_injection Injectable into sensors and hardware-dependent classes
 * @gpio_operations Digital read/write, analog read, pin mode configuration
 * 
 * @core_capabilities:
 * - Digital I/O: Read digital pin states (HIGH/LOW)
 * - Analog input: Read ADC values from analog pins
 * - Pin configuration: Set pin modes (INPUT, OUTPUT, INPUT_PULLUP)
 * 
 * @implementation_notes:
 * - Real hardware: GpioProvider using Arduino framework
 * - Testing: MockGpioProvider with simulated readings
 * - ADC range: Typically 0-4095 for 12-bit ESP32 ADC
 * 
 * @thread_safety Implementation-dependent (Arduino framework not thread-safe)
 * @error_handling Interface does not define error handling - implementation dependent
 * 
 * @context This interface allows sensors to access hardware without
 * direct dependencies, enabling unit testing and hardware abstraction.
 * All GPIO operations in the system go through this interface.
 */
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