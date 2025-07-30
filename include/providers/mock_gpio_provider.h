#pragma once

#include "interfaces/i_gpio_provider.h"
#include <unordered_map>
#include <cstdint>

/// @brief Mock implementation of GPIO provider for testing
/// @details Provides controllable GPIO behavior for unit and integration tests
class MockGpioProvider : public IGpioProvider
{
private:
    std::unordered_map<int, bool> digitalPins_;
    std::unordered_map<int, uint16_t> analogPins_;
    std::unordered_map<int, int> pinModes_;

public:
    /// @brief Read digital value from a GPIO pin
    /// @param pin GPIO pin number
    /// @return true if pin is HIGH, false if LOW
    bool digitalRead(int pin) override;

    /// @brief Write digital value to a GPIO pin
    /// @param pin GPIO pin number  
    /// @param value true for HIGH, false for LOW
    void digitalWrite(int pin, bool value) override;

    /// @brief Read analog value from an ADC pin
    /// @param pin ADC pin number
    /// @return ADC reading (0-4095 for 12-bit ADC)
    uint16_t analogRead(int pin) override;

    /// @brief Configure pin mode
    /// @param pin GPIO pin number
    /// @param mode Pin mode (INPUT, OUTPUT, INPUT_PULLUP, etc.)
    void pinMode(int pin, int mode) override;

    // Test utility methods
    
    /// @brief Set digital pin state for testing
    /// @param pin GPIO pin number
    /// @param value Pin state (true = HIGH, false = LOW)
    void setDigitalPin(int pin, bool value);

    /// @brief Set analog pin value for testing
    /// @param pin ADC pin number
    /// @param value ADC value (0-4095)
    void setAnalogPin(int pin, uint16_t value);

    /// @brief Get pin mode for testing
    /// @param pin GPIO pin number
    /// @return Pin mode
    int getPinMode(int pin) const;

    /// @brief Reset all pin states
    void reset();
};