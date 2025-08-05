#include "mock_gpio_provider.h"
#include <random>
#include <algorithm>

// Arduino constants for pin modes (defined here for testing)
#ifndef INPUT
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define INPUT_PULLDOWN 3
#endif

#ifndef HIGH
#define HIGH 1
#define LOW 0
#endif

#ifndef RISING
#define RISING 1
#define FALLING 2
#define CHANGE 3
#endif

MockGpioProvider::MockGpioProvider()
{
    reset();
}

bool MockGpioProvider::digitalRead(int pin)
{
    digitalReadCounts_[pin]++;
    
    // Return configured value or default to LOW
    auto it = digitalReadings_.find(pin);
    if (it != digitalReadings_.end()) {
        return it->second;
    }
    return false; // Default to LOW
}

uint16_t MockGpioProvider::analogRead(int pin)
{
    analogReadCounts_[pin]++;
    
    uint16_t baseValue = getNextAnalogReading(pin);
    return applyNoise(pin, baseValue);
}

void MockGpioProvider::pinMode(int pin, int mode)
{
    pinModes_[pin] = mode;
    pinModeSet_[pin] = true;
}

void MockGpioProvider::attachInterrupt(int pin, void (*callback)(), int mode)
{
    interruptCallbacks_[pin] = callback;
    interruptModes_[pin] = mode;
    interruptsAttached_[pin] = true;
}

void MockGpioProvider::detachInterrupt(int pin)
{
    interruptCallbacks_.erase(pin);
    interruptModes_.erase(pin);
    interruptsAttached_[pin] = false;
}

bool MockGpioProvider::hasInterrupt(int pin)
{
    auto it = interruptsAttached_.find(pin);
    return (it != interruptsAttached_.end()) && it->second;
}

// Test configuration methods

void MockGpioProvider::setDigitalReading(int pin, bool value)
{
    digitalReadings_[pin] = value;
}

void MockGpioProvider::setAnalogReading(int pin, uint16_t value)
{
    analogReadings_[pin] = value;
    // Clear any existing sequence for this pin
    analogSequences_.erase(pin);
    analogSequenceIndex_.erase(pin);
}

void MockGpioProvider::setAnalogReadingSequence(int pin, const std::vector<uint16_t>& values)
{
    analogSequences_[pin] = values;
    analogSequenceIndex_[pin] = 0;
    // Clear single value for this pin
    analogReadings_.erase(pin);
}

void MockGpioProvider::reset()
{
    digitalReadings_.clear();
    analogReadings_.clear();
    analogSequences_.clear();
    analogSequenceIndex_.clear();
    pinModes_.clear();
    pinModeSet_.clear();
    interruptCallbacks_.clear();
    interruptModes_.clear();
    interruptsAttached_.clear();
    digitalReadCounts_.clear();
    analogReadCounts_.clear();
    adcNoiseLevels_.clear();
}

// Test verification methods

int MockGpioProvider::getPinMode(int pin) const
{
    auto it = pinModes_.find(pin);
    if (it != pinModes_.end()) {
        return it->second;
    }
    return -1; // Invalid mode indicates pinMode was never called
}

bool MockGpioProvider::wasPinModeSet(int pin) const
{
    auto it = pinModeSet_.find(pin);
    return (it != pinModeSet_.end()) && it->second;
}

int MockGpioProvider::getDigitalReadCount(int pin) const
{
    auto it = digitalReadCounts_.find(pin);
    return (it != digitalReadCounts_.end()) ? it->second : 0;
}

int MockGpioProvider::getAnalogReadCount(int pin) const
{
    auto it = analogReadCounts_.find(pin);
    return (it != analogReadCounts_.end()) ? it->second : 0;
}

bool MockGpioProvider::wasInterruptAttached(int pin) const
{
    auto it = interruptsAttached_.find(pin);
    return (it != interruptsAttached_.end()) && it->second;
}

int MockGpioProvider::getInterruptMode(int pin) const
{
    auto it = interruptModes_.find(pin);
    return (it != interruptModes_.end()) ? it->second : -1;
}

// Hardware simulation methods

void MockGpioProvider::simulateDigitalChange(int pin, bool newValue)
{
    bool oldValue = digitalReadings_[pin];
    digitalReadings_[pin] = newValue;
    
    // Trigger interrupt if attached and condition matches
    if (hasInterrupt(pin)) {
        int mode = getInterruptMode(pin);
        bool shouldTrigger = false;
        
        switch (mode) {
            case RISING:
                shouldTrigger = !oldValue && newValue;
                break;
            case FALLING:
                shouldTrigger = oldValue && !newValue;
                break;
            case CHANGE:
                shouldTrigger = (oldValue != newValue);
                break;
        }
        
        if (shouldTrigger && interruptCallbacks_[pin] != nullptr) {
            interruptCallbacks_[pin]();
        }
    }
}

void MockGpioProvider::simulateAdcNoise(int pin, uint16_t noiseLevel)
{
    adcNoiseLevels_[pin] = noiseLevel;
}

// Private helper methods

uint16_t MockGpioProvider::getNextAnalogReading(int pin)
{
    // Check if pin has a sequence configured
    auto seqIt = analogSequences_.find(pin);
    if (seqIt != analogSequences_.end() && !seqIt->second.empty()) {
        auto indexIt = analogSequenceIndex_.find(pin);
        if (indexIt != analogSequenceIndex_.end()) {
            size_t index = indexIt->second;
            uint16_t value = seqIt->second[index];
            
            // Advance to next value in sequence (wrap around)
            analogSequenceIndex_[pin] = (index + 1) % seqIt->second.size();
            return value;
        }
    }
    
    // Check if pin has a single value configured
    auto valueIt = analogReadings_.find(pin);
    if (valueIt != analogReadings_.end()) {
        return valueIt->second;
    }
    
    // Default to 0 if no configuration
    return 0;
}

uint16_t MockGpioProvider::applyNoise(int pin, uint16_t baseValue)
{
    auto noiseIt = adcNoiseLevels_.find(pin);
    if (noiseIt == adcNoiseLevels_.end() || noiseIt->second == 0) {
        return baseValue; // No noise configured
    }
    
    // Generate random noise using static generator for consistency
    static std::random_device rd;
    static std::mt19937 gen(rd());
    
    uint16_t noiseLevel = noiseIt->second;
    std::uniform_int_distribution<int> dist(-static_cast<int>(noiseLevel), static_cast<int>(noiseLevel));
    
    int noisyValue = static_cast<int>(baseValue) + dist(gen);
    
    // Clamp to valid ADC range (0-4095)
    return static_cast<uint16_t>(std::max(0, std::min(4095, noisyValue)));
}