#include "test_utilities.h"
#include <cstring>

// Define global mock state
bool mock_gpio_states[40] = {false};

// Initialize static members
uint16_t MockHardware::mock_adc_readings[40] = {0};
bool MockHardware::mock_adc_failures[40] = {false};

void MockHardware::reset() {
    for (int i = 0; i < 40; i++) {
        mock_gpio_states[i] = false;
        mock_adc_readings[i] = 0;
        mock_adc_failures[i] = false;
    }
}

void MockHardware::setGpioState(uint8_t pin, bool state) {
    if (pin < 40) {
        mock_gpio_states[pin] = state;
    }
}

bool MockHardware::getGpioState(uint8_t pin) {
    if (pin < 40) {
        return mock_gpio_states[pin];
    }
    return false;
}

void MockHardware::simulateAdcReading(uint8_t pin, uint16_t value) {
    if (pin < 40) {
        mock_adc_readings[pin] = value;
    }
}

void MockHardware::simulateAdcFailure(uint8_t pin, bool failed) {
    if (pin < 40) {
        mock_adc_failures[pin] = failed;
    }
}

uint16_t MockHardware::getAdcReading(uint8_t pin) {
    if (pin < 40) {
        return mock_adc_readings[pin];
    }
    return 0;
}
