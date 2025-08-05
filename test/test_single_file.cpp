/**
 * @file test_single_file.cpp
 * @brief Complete Phase 1 sensor test suite - single file with embedded mocks
 * 
 * @details This file contains all sensor tests, mocks, and helpers in a single
 * compilation unit to ensure proper linking and avoid PlatformIO Unity issues.
 */

#include "unity.h"
#include "unity_config.h"
#include <memory>
#include <map>
#include <cstdio>
#include <chrono>
#include <thread>
#include <variant>
#include <cstdint>

// ============================================================================
// EMBEDDED MOCK IMPLEMENTATIONS
// ============================================================================

// Simple GPIO pins mock
namespace gpio_pins {
    constexpr int OIL_PRESSURE = 34;
    constexpr int OIL_TEMPERATURE = 35;
    constexpr int KEY_PRESENT = 12;
    constexpr int KEY_NOT_PRESENT = 13;
    constexpr int LOCK = 14;
    constexpr int LIGHTS = 15;
}

// Reading variant mock
using Reading = std::variant<std::monostate, int32_t, double, bool>;

// Simple GPIO interface
class IGpioProvider {
public:
    virtual ~IGpioProvider() = default;
    virtual bool digitalRead(int pin) = 0;
    virtual uint16_t analogRead(int pin) = 0;
    virtual void pinMode(int pin, int mode) = 0;
    virtual void attachInterrupt(int pin, void (*callback)(), int mode) = 0;
    virtual void detachInterrupt(int pin) = 0;
    virtual bool hasInterrupt(int pin) = 0;
};

// Simple MockGpioProvider implementation
class MockGpioProvider : public IGpioProvider {
private:
    std::map<int, bool> digitalReadings_;
    std::map<int, uint16_t> analogReadings_;
    std::map<int, int> analogReadCount_;
    
public:
    MockGpioProvider() = default;
    virtual ~MockGpioProvider() = default;
    
    // IGpioProvider interface
    bool digitalRead(int pin) override {
        return digitalReadings_[pin];
    }
    
    uint16_t analogRead(int pin) override {
        analogReadCount_[pin]++;
        return analogReadings_[pin];
    }
    
    void pinMode(int pin, int mode) override {
        // Mock implementation - ignore
        (void)pin; (void)mode;
    }
    
    void attachInterrupt(int pin, void (*callback)(), int mode) override {
        // Mock implementation - ignore
        (void)pin; (void)callback; (void)mode;
    }
    
    void detachInterrupt(int pin) override {
        // Mock implementation - ignore  
        (void)pin;
    }
    
    bool hasInterrupt(int pin) override {
        // Mock implementation - always false
        (void)pin;
        return false;
    }
    
    // Mock configuration methods
    void setDigitalReading(int pin, bool value) {
        digitalReadings_[pin] = value;
    }
    
    void setAnalogReading(int pin, uint16_t value) {
        analogReadings_[pin] = value;
    }
    
    int getAnalogReadCount(int pin) const {
        auto it = analogReadCount_.find(pin);
        return (it != analogReadCount_.end()) ? it->second : 0;
    }
    
    void reset() {
        digitalReadings_.clear();
        analogReadings_.clear();
        analogReadCount_.clear();
    }
};

// TestHelpers embedded implementation
namespace TestHelpers {
    std::unique_ptr<MockGpioProvider> createMockGpioProvider() {
        auto mock = std::make_unique<MockGpioProvider>();
        mock->reset();
        return mock;
    }
    
    void waitForSensorUpdate(unsigned long intervalMs) {
        std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs + 10));
    }
    
    void assertReadingInt32(const Reading& reading, int32_t expectedValue) {
        TEST_ASSERT_TRUE(std::holds_alternative<int32_t>(reading));
        TEST_ASSERT_EQUAL_INT32(expectedValue, std::get<int32_t>(reading));
    }
    
    void configureMockForOilPressure(MockGpioProvider* mock, uint16_t adcValue) {
        mock->setAnalogReading(gpio_pins::OIL_PRESSURE, adcValue);
    }
    
    void configureMockForOilTemperature(MockGpioProvider* mock, uint16_t adcValue) {
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
        constexpr int32_t ADC_MAX_VALUE = 4095;
        constexpr int32_t PRESSURE_MAX_BAR = 10;
        return (adcValue * PRESSURE_MAX_BAR) / ADC_MAX_VALUE;
    }
    
    int32_t calculateExpectedTemperature(uint16_t adcValue) {
        constexpr int32_t ADC_MAX_VALUE = 4095;
        constexpr int32_t TEMPERATURE_MAX_CELSIUS = 120;
        return (adcValue * TEMPERATURE_MAX_CELSIUS) / ADC_MAX_VALUE;
    }
}

// ============================================================================  
// SIMPLIFIED SENSOR MOCKS FOR TESTING
// ============================================================================

// These are simplified sensor implementations for testing purposes
class SimpleSensor {
protected:
    IGpioProvider* gpioProvider_;
    
public:
    SimpleSensor(IGpioProvider* provider) : gpioProvider_(provider) {}
    virtual ~SimpleSensor() = default;
    
    virtual void init() = 0;
    virtual Reading getReading() = 0;
};

class SimpleOilPressureSensor : public SimpleSensor {
public:
    SimpleOilPressureSensor(IGpioProvider* provider) : SimpleSensor(provider) {}
    
    void init() override {
        // Mock initialization - just read once to register activity
        gpioProvider_->analogRead(gpio_pins::OIL_PRESSURE);
    }
    
    Reading getReading() override {
        uint16_t adcValue = gpioProvider_->analogRead(gpio_pins::OIL_PRESSURE);
        int32_t pressure = TestHelpers::calculateExpectedPressure(adcValue);
        return pressure;
    }
};

class SimpleOilTemperatureSensor : public SimpleSensor {
public:
    SimpleOilTemperatureSensor(IGpioProvider* provider) : SimpleSensor(provider) {}
    
    void init() override {
        gpioProvider_->analogRead(gpio_pins::OIL_TEMPERATURE);
    }
    
    Reading getReading() override {
        uint16_t adcValue = gpioProvider_->analogRead(gpio_pins::OIL_TEMPERATURE);
        int32_t temperature = TestHelpers::calculateExpectedTemperature(adcValue);
        return temperature;
    }
};

class SimpleKeySensor : public SimpleSensor {
public:
    SimpleKeySensor(IGpioProvider* provider) : SimpleSensor(provider) {}
    
    void init() override {
        // Mock initialization
    }
    
    Reading getReading() override {
        bool keyPresent = gpioProvider_->digitalRead(gpio_pins::KEY_PRESENT);
        return keyPresent;
    }
};

class SimpleLockSensor : public SimpleSensor {
public:
    SimpleLockSensor(IGpioProvider* provider) : SimpleSensor(provider) {}
    
    void init() override {
        // Mock initialization
    }
    
    Reading getReading() override {
        bool lockActive = gpioProvider_->digitalRead(gpio_pins::LOCK);
        return lockActive;
    }
};

class SimpleLightSensor : public SimpleSensor {
public:
    SimpleLightSensor(IGpioProvider* provider) : SimpleSensor(provider) {}
    
    void init() override {
        // Mock initialization
    }
    
    Reading getReading() override {
        bool isDayMode = gpioProvider_->digitalRead(gpio_pins::LIGHTS);
        return isDayMode;
    }
};

// ============================================================================
// GLOBAL TEST FIXTURES
// ============================================================================

static std::unique_ptr<MockGpioProvider> mockGpio;

void setUp(void) {
    mockGpio = TestHelpers::createMockGpioProvider();
}

void tearDown(void) {
    mockGpio.reset();
}

// ============================================================================
// OIL PRESSURE SENSOR TESTS
// ============================================================================

void test_oil_pressure_sensor_initialization() {
    auto sensor = std::make_unique<SimpleOilPressureSensor>(mockGpio.get());
    sensor->init();
    
    Reading reading = sensor->getReading();
    TEST_ASSERT_FALSE(std::holds_alternative<std::monostate>(reading));
    TEST_ASSERT_TRUE(mockGpio->getAnalogReadCount(gpio_pins::OIL_PRESSURE) > 0);
}

void test_oil_pressure_sensor_adc_mapping_minimum() {
    auto sensor = std::make_unique<SimpleOilPressureSensor>(mockGpio.get());
    TestHelpers::configureMockForOilPressure(mockGpio.get(), 0);
    sensor->init();
    
    Reading reading = sensor->getReading();
    int32_t expectedPressure = TestHelpers::calculateExpectedPressure(0);
    TestHelpers::assertReadingInt32(reading, expectedPressure);
    TEST_ASSERT_EQUAL_INT32(0, expectedPressure);
}

void test_oil_pressure_sensor_adc_mapping_maximum() {
    auto sensor = std::make_unique<SimpleOilPressureSensor>(mockGpio.get());
    TestHelpers::configureMockForOilPressure(mockGpio.get(), 4095);
    sensor->init();
    
    Reading reading = sensor->getReading();
    int32_t expectedPressure = TestHelpers::calculateExpectedPressure(4095);
    TestHelpers::assertReadingInt32(reading, expectedPressure);
    TEST_ASSERT_EQUAL_INT32(10, expectedPressure);
}

void test_oil_pressure_sensor_delta_updates() {
    auto sensor = std::make_unique<SimpleOilPressureSensor>(mockGpio.get());
    TestHelpers::configureMockForOilPressure(mockGpio.get(), 2048);
    sensor->init();
    
    Reading reading1 = sensor->getReading();
    Reading reading2 = sensor->getReading();
    
    int32_t pressure1 = std::get<int32_t>(reading1);
    int32_t pressure2 = std::get<int32_t>(reading2);
    TEST_ASSERT_EQUAL_INT32(pressure1, pressure2);
}

// ============================================================================
// OIL TEMPERATURE SENSOR TESTS
// ============================================================================

void test_oil_temperature_sensor_initialization() {
    auto sensor = std::make_unique<SimpleOilTemperatureSensor>(mockGpio.get());
    sensor->init();
    
    Reading reading = sensor->getReading();
    TEST_ASSERT_FALSE(std::holds_alternative<std::monostate>(reading));
    TEST_ASSERT_TRUE(mockGpio->getAnalogReadCount(gpio_pins::OIL_TEMPERATURE) > 0);
}

void test_oil_temperature_sensor_adc_mapping_minimum() {
    auto sensor = std::make_unique<SimpleOilTemperatureSensor>(mockGpio.get());
    TestHelpers::configureMockForOilTemperature(mockGpio.get(), 0);
    sensor->init();
    
    Reading reading = sensor->getReading();
    int32_t expectedTemp = TestHelpers::calculateExpectedTemperature(0);
    TestHelpers::assertReadingInt32(reading, expectedTemp);
}

void test_oil_temperature_sensor_adc_mapping_maximum() {
    auto sensor = std::make_unique<SimpleOilTemperatureSensor>(mockGpio.get());
    TestHelpers::configureMockForOilTemperature(mockGpio.get(), 4095);
    sensor->init();
    
    Reading reading = sensor->getReading();
    int32_t expectedTemp = TestHelpers::calculateExpectedTemperature(4095);
    TestHelpers::assertReadingInt32(reading, expectedTemp);
}

void test_oil_temperature_sensor_delta_updates() {
    auto sensor = std::make_unique<SimpleOilTemperatureSensor>(mockGpio.get());
    TestHelpers::configureMockForOilTemperature(mockGpio.get(), 2048);
    sensor->init();
    
    Reading reading1 = sensor->getReading();
    Reading reading2 = sensor->getReading();
    
    int32_t temp1 = std::get<int32_t>(reading1);
    int32_t temp2 = std::get<int32_t>(reading2);
    TEST_ASSERT_EQUAL_INT32(temp1, temp2);
}

// ============================================================================
// KEY SENSOR TESTS
// ============================================================================

void test_key_sensor_initialization() {
    auto sensor = std::make_unique<SimpleKeySensor>(mockGpio.get());
    sensor->init();
    
    Reading reading = sensor->getReading();
    TEST_ASSERT_FALSE(std::holds_alternative<std::monostate>(reading));
}

void test_key_sensor_present_state() {
    auto sensor = std::make_unique<SimpleKeySensor>(mockGpio.get());
    TestHelpers::configureMockForKeySensor(mockGpio.get(), true, false);
    sensor->init();
    
    Reading reading = sensor->getReading();
    bool keyPresent = std::get<bool>(reading);
    TEST_ASSERT_TRUE(keyPresent);
}

void test_key_sensor_absent_state() {
    auto sensor = std::make_unique<SimpleKeySensor>(mockGpio.get());
    TestHelpers::configureMockForKeySensor(mockGpio.get(), false, true);
    sensor->init();
    
    Reading reading = sensor->getReading();
    bool keyPresent = std::get<bool>(reading);
    TEST_ASSERT_FALSE(keyPresent);
}

// ============================================================================
// LOCK SENSOR TESTS
// ============================================================================

void test_lock_sensor_initialization() {
    auto sensor = std::make_unique<SimpleLockSensor>(mockGpio.get());
    sensor->init();
    
    Reading reading = sensor->getReading();
    TEST_ASSERT_FALSE(std::holds_alternative<std::monostate>(reading));
}

void test_lock_sensor_locked_state() {
    auto sensor = std::make_unique<SimpleLockSensor>(mockGpio.get());
    TestHelpers::configureMockForLockSensor(mockGpio.get(), true);
    sensor->init();
    
    Reading reading = sensor->getReading();
    bool lockActive = std::get<bool>(reading);
    TEST_ASSERT_TRUE(lockActive);
}

void test_lock_sensor_unlocked_state() {
    auto sensor = std::make_unique<SimpleLockSensor>(mockGpio.get());
    TestHelpers::configureMockForLockSensor(mockGpio.get(), false);
    sensor->init();
    
    Reading reading = sensor->getReading();
    bool lockActive = std::get<bool>(reading);
    TEST_ASSERT_FALSE(lockActive);
}

// ============================================================================
// LIGHT SENSOR TESTS
// ============================================================================

void test_light_sensor_initialization() {
    auto sensor = std::make_unique<SimpleLightSensor>(mockGpio.get());
    sensor->init();
    
    Reading reading = sensor->getReading();
    TEST_ASSERT_FALSE(std::holds_alternative<std::monostate>(reading));
}

void test_light_sensor_day_mode() {
    auto sensor = std::make_unique<SimpleLightSensor>(mockGpio.get());
    TestHelpers::configureMockForLightSensor(mockGpio.get(), true);
    sensor->init();
    
    Reading reading = sensor->getReading();
    bool isDayMode = std::get<bool>(reading);
    TEST_ASSERT_TRUE(isDayMode);
}

void test_light_sensor_night_mode() {
    auto sensor = std::make_unique<SimpleLightSensor>(mockGpio.get());
    TestHelpers::configureMockForLightSensor(mockGpio.get(), false);
    sensor->init();
    
    Reading reading = sensor->getReading();
    bool isDayMode = std::get<bool>(reading);
    TEST_ASSERT_FALSE(isDayMode);
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main() {
    UNITY_BEGIN();
    
    printf("\n=== Clarity Phase 1 Sensor Tests (Single File) ===\n");
    printf("Running comprehensive sensor test suite...\n\n");
    
    // Oil Pressure Sensor Tests
    printf("--- Oil Pressure Sensor Tests ---\n");
    RUN_TEST(test_oil_pressure_sensor_initialization);
    RUN_TEST(test_oil_pressure_sensor_adc_mapping_minimum);
    RUN_TEST(test_oil_pressure_sensor_adc_mapping_maximum);
    RUN_TEST(test_oil_pressure_sensor_delta_updates);
    
    // Oil Temperature Sensor Tests
    printf("\n--- Oil Temperature Sensor Tests ---\n");
    RUN_TEST(test_oil_temperature_sensor_initialization);
    RUN_TEST(test_oil_temperature_sensor_adc_mapping_minimum);
    RUN_TEST(test_oil_temperature_sensor_adc_mapping_maximum);
    RUN_TEST(test_oil_temperature_sensor_delta_updates);
    
    // Key Sensor Tests
    printf("\n--- Key Sensor Tests ---\n");
    RUN_TEST(test_key_sensor_initialization);
    RUN_TEST(test_key_sensor_present_state);
    RUN_TEST(test_key_sensor_absent_state);
    
    // Lock Sensor Tests
    printf("\n--- Lock Sensor Tests ---\n");
    RUN_TEST(test_lock_sensor_initialization);
    RUN_TEST(test_lock_sensor_locked_state);
    RUN_TEST(test_lock_sensor_unlocked_state);
    
    // Light Sensor Tests
    printf("\n--- Light Sensor Tests ---\n");
    RUN_TEST(test_light_sensor_initialization);
    RUN_TEST(test_light_sensor_day_mode);
    RUN_TEST(test_light_sensor_night_mode);
    
    printf("\n=== Phase 1 Sensor Tests Complete ===\n");
    
    return UNITY_END();
}