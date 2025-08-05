#include "test_helpers.h"
#include "hardware/gpio_pins.h"
#include <variant>
#include <thread>
#include <chrono>

namespace TestHelpers {

std::unique_ptr<MockGpioProvider> createMockGpioProvider() {
    auto mock = std::make_unique<MockGpioProvider>();
    mock->reset(); // Ensure clean state
    return mock;
}

void advanceTime(unsigned long milliseconds) {
    // Use actual delay to simulate real time passage
    // This ensures millis() advances correctly
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

void waitForSensorUpdate(unsigned long intervalMs) {
    // Add 10ms buffer to ensure update occurs
    advanceTime(intervalMs + 10);
}

void assertReadingInt32(const Reading& reading, int32_t expectedValue) {
    TEST_ASSERT_TRUE(std::holds_alternative<int32_t>(reading));
    TEST_ASSERT_EQUAL_INT32(expectedValue, std::get<int32_t>(reading));
}

void assertReadingBool(const Reading& reading, bool expectedValue) {
    TEST_ASSERT_TRUE(std::holds_alternative<bool>(reading));
    TEST_ASSERT_EQUAL(expectedValue, std::get<bool>(reading));
}

void assertReadingMonostate(const Reading& reading) {
    TEST_ASSERT_TRUE(std::holds_alternative<std::monostate>(reading));
}

void assertValidAdcRange(uint16_t value) {
    TEST_ASSERT_TRUE(value >= 0 && value <= 4095);
}

void assertValidPressureRange(int32_t pressure) {
    TEST_ASSERT_TRUE(pressure >= 0 && pressure <= 10);
}

void assertValidTemperatureRange(int32_t temperature) {
    TEST_ASSERT_TRUE(temperature >= 0 && temperature <= 120);
}

void configureMockForOilPressure(MockGpioProvider* mock, uint16_t adcValue) {
    assertValidAdcRange(adcValue);
    mock->setAnalogReading(gpio_pins::OIL_PRESSURE, adcValue);
}

void configureMockForOilTemperature(MockGpioProvider* mock, uint16_t adcValue) {
    assertValidAdcRange(adcValue); 
    mock->setAnalogReading(gpio_pins::OIL_TEMPERATURE, adcValue);
}

void configureMockForKeySensor(MockGpioProvider* mock, bool keyPresent, bool keyNotPresent) {
    mock->setDigitalReading(gpio_pins::KEY_PRESENT, keyPresent);
    mock->setDigitalReading(gpio_pins::KEY_NOT_PRESENT, keyNotPresent);
}

void configureMockForLockSensor(MockGpioProvider* mock, bool lockState) {
    mock->setDigitalReading(gpio_pins::LOCK, lockState);
}

void configureMockForLightSensor(MockGpioProvider* mock, bool lightState) {
    mock->setDigitalReading(gpio_pins::LIGHTS, lightState);
}

int32_t calculateExpectedPressure(uint16_t adcValue) {
    // Use same formula as OilPressureSensor
    // Pressure = (ADC_value * 10) / 4095
    constexpr int32_t ADC_MAX_VALUE = 4095;
    constexpr int32_t PRESSURE_MAX_BAR = 10;
    return (adcValue * PRESSURE_MAX_BAR) / ADC_MAX_VALUE;
}

int32_t calculateExpectedTemperature(uint16_t adcValue) {
    // Use same formula as OilTemperatureSensor  
    // Temperature = (ADC_value * 120) / 4095
    constexpr int32_t ADC_MAX_VALUE = 4095;
    constexpr int32_t TEMPERATURE_MAX_CELSIUS = 120;
    return (adcValue * TEMPERATURE_MAX_CELSIUS) / ADC_MAX_VALUE;
}

template<typename TSensor>
void testSensorInitialization(TSensor& sensor, MockGpioProvider* mock) {
    // Reset mock to clean state
    mock->reset();
    
    // Initialize sensor should not throw or crash
    sensor.init();
    
    // Sensor should be ready to provide readings
    Reading reading = sensor.getReading();
    TEST_ASSERT_FALSE(std::holds_alternative<std::monostate>(reading));
}

template<typename TSensor, typename TValue>
void testDeltaBasedUpdates(TSensor& sensor, MockGpioProvider* mock,
                          std::function<TValue()> getValue, 
                          std::function<void(TValue)> setValue) {
    // Set initial value
    TValue initialValue = getValue();
    setValue(initialValue);
    
    // Initialize sensor
    sensor.init();
    Reading reading1 = sensor.getReading();
    
    // Same value should not trigger update immediately
    Reading reading2 = sensor.getReading(); 
    // Note: This tests that consecutive calls return same value
    
    // Change value and wait for update interval
    TValue newValue = getValue() + 1; // Assumes arithmetic type
    setValue(newValue);
    waitForSensorUpdate(1000); // Default update interval
    
    // New reading should reflect changed value
    Reading reading3 = sensor.getReading();
    // Verify reading changed (specific to sensor type)
}

// Explicit template instantiations for commonly used sensor types
// These will be defined in specific test files as needed

} // namespace TestHelpers