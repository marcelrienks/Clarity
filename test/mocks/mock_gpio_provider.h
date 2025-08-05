#pragma once

#include "interfaces/i_gpio_provider.h"
#ifdef UNIT_TESTING
#include "arduino_mock.h"
#endif
#include <map>
#include <vector>
#include <functional>

/**
 * @class MockGpioProvider
 * @brief Mock implementation of IGpioProvider for unit testing
 * 
 * @details This mock provides configurable GPIO behavior for testing sensors
 * without hardware dependencies. It supports:
 * - Configurable digital/analog readings per pin
 * - Pin mode validation and tracking
 * - Interrupt simulation and validation
 * - Method call verification
 * 
 * @usage_patterns:
 * - Configure expected readings before test execution
 * - Verify pin configurations and method calls after test
 * - Simulate hardware state changes during test execution
 */
class MockGpioProvider : public IGpioProvider
{
public:
    // Constructor
    MockGpioProvider();
    virtual ~MockGpioProvider() = default;

    // IGpioProvider interface implementation
    bool digitalRead(int pin) override;
    uint16_t analogRead(int pin) override;
    void pinMode(int pin, int mode) override;
    void attachInterrupt(int pin, void (*callback)(), int mode) override;
    void detachInterrupt(int pin) override;
    bool hasInterrupt(int pin) override;

    // Test configuration methods
    
    /// @brief Set digital reading value for a specific pin
    /// @param pin GPIO pin number
    /// @param value Digital value (true=HIGH, false=LOW)
    void setDigitalReading(int pin, bool value);
    
    /// @brief Set analog reading value for a specific pin
    /// @param pin ADC pin number  
    /// @param value Analog value (0-4095 for 12-bit ADC)
    void setAnalogReading(int pin, uint16_t value);
    
    /// @brief Set sequence of analog readings for time-based testing
    /// @param pin ADC pin number
    /// @param values Vector of analog values to return in sequence
    void setAnalogReadingSequence(int pin, const std::vector<uint16_t>& values);
    
    /// @brief Reset all pin configurations and readings to defaults
    void reset();

    // Test verification methods
    
    /// @brief Get the current pin mode for a specific pin
    /// @param pin GPIO pin number
    /// @return Pin mode (INPUT, OUTPUT, INPUT_PULLUP, etc.)
    int getPinMode(int pin) const;
    
    /// @brief Check if pinMode was called for a specific pin
    /// @param pin GPIO pin number
    /// @return true if pinMode was called, false otherwise
    bool wasPinModeSet(int pin) const;
    
    /// @brief Get count of digitalRead calls for a specific pin
    /// @param pin GPIO pin number
    /// @return Number of digitalRead calls made
    int getDigitalReadCount(int pin) const;
    
    /// @brief Get count of analogRead calls for a specific pin
    /// @param pin ADC pin number
    /// @return Number of analogRead calls made
    int getAnalogReadCount(int pin) const;
    
    /// @brief Check if interrupt was attached to a specific pin
    /// @param pin GPIO pin number
    /// @return true if interrupt attached, false otherwise
    bool wasInterruptAttached(int pin) const;
    
    /// @brief Get interrupt mode for a specific pin
    /// @param pin GPIO pin number
    /// @return Interrupt mode (RISING, FALLING, CHANGE, etc.)
    int getInterruptMode(int pin) const;

    // Hardware simulation methods
    
    /// @brief Simulate hardware state change (triggers interrupt if attached)
    /// @param pin GPIO pin number
    /// @param newValue New digital value for the pin
    void simulateDigitalChange(int pin, bool newValue);
    
    /// @brief Simulate ADC noise (random variation in analog readings)
    /// @param pin ADC pin number
    /// @param noiseLevel Maximum noise amplitude (±noiseLevel)
    void simulateAdcNoise(int pin, uint16_t noiseLevel);

private:
    // State tracking
    std::map<int, bool> digitalReadings_;           // Pin -> digital value
    std::map<int, uint16_t> analogReadings_;        // Pin -> analog value
    std::map<int, std::vector<uint16_t>> analogSequences_; // Pin -> sequence of values
    std::map<int, size_t> analogSequenceIndex_;     // Pin -> current sequence index
    std::map<int, int> pinModes_;                   // Pin -> mode
    std::map<int, bool> pinModeSet_;                // Pin -> pinMode called flag
    std::map<int, void(*)()> interruptCallbacks_;   // Pin -> callback function
    std::map<int, int> interruptModes_;             // Pin -> interrupt mode
    std::map<int, bool> interruptsAttached_;        // Pin -> interrupt attached flag
    
    // Call counters for verification
    std::map<int, int> digitalReadCounts_;          // Pin -> read count
    std::map<int, int> analogReadCounts_;           // Pin -> read count
    
    // Noise simulation
    std::map<int, uint16_t> adcNoiseLevels_;        // Pin -> noise level
    
    /// @brief Get next analog reading from sequence or single value
    /// @param pin ADC pin number
    /// @return Next analog reading value
    uint16_t getNextAnalogReading(int pin);
    
    /// @brief Apply noise to analog reading if configured
    /// @param pin ADC pin number
    /// @param baseValue Base analog reading
    /// @return Noisy analog reading
    uint16_t applyNoise(int pin, uint16_t baseValue);
};