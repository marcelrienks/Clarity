#include <unity.h>
#include "sensors/oil_pressure_sensor.h"
#include "mock_gpio_provider.h"
#include "hardware/gpio_pins.h"

#ifdef UNIT_TESTING
// Mock Arduino functions
extern "C" {
    void analogReadResolution(uint8_t bits) {}
    void analogSetAttenuation(adc_attenuation_t attenuation) {}
    void log_d(const char* format, ...) {}
    void log_w(const char* format, ...) {}
}
#endif

MockGpioProvider* mockGpio;
OilPressureSensor* sensor;

void setUp_oil_pressure_sensor() {
    mockGpio = new MockGpioProvider();
    sensor = new OilPressureSensor(mockGpio);
}

void tearDown_oil_pressure_sensor() {
    delete sensor;
    delete mockGpio;
}

void test_oil_pressure_sensor_init() {
    sensor->init();
    
    // Verify that the sensor initializes without throwing
    TEST_ASSERT_TRUE(true);
}

void test_oil_pressure_sensor_reading_conversion() {
    // Set a known ADC value
    uint16_t testAdcValue = 2048; // Mid-range 12-bit value
    mockGpio->setAnalogValue(gpio_pins::OIL_PRESSURE, testAdcValue);
    
    sensor->init();
    
    // Get the reading
    Reading pressureReading = sensor->getReading();
    double pressure = std::get<double>(pressureReading);
    
    // Pressure should be positive and reasonable
    TEST_ASSERT_GREATER_THAN(0.0, pressure);
    TEST_ASSERT_LESS_THAN(200.0, pressure); // Reasonable max pressure
}

void test_oil_pressure_sensor_value_change_detection() {
    sensor->init();
    
    // Set initial value
    mockGpio->setAnalogValue(gpio_pins::OIL_PRESSURE, 1000);
    Reading pressureReading1 = sensor->getReading();
    double reading1 = std::get<double>(pressureReading1);
    bool hasChanged1 = sensor->hasValueChanged();
    
    // First reading should indicate change (from initial state)
    TEST_ASSERT_TRUE(hasChanged1);
    
    // Same value should not indicate change
    Reading pressureReading2 = sensor->getReading();
    double reading2 = std::get<double>(pressureReading2);
    bool hasChanged2 = sensor->hasValueChanged();
    TEST_ASSERT_FALSE(hasChanged2);
    
    // Different value should indicate change
    mockGpio->setAnalogValue(gpio_pins::OIL_PRESSURE, 2000);
    Reading pressureReading3 = sensor->getReading();
    double reading3 = std::get<double>(pressureReading3);
    bool hasChanged3 = sensor->hasValueChanged();
    TEST_ASSERT_TRUE(hasChanged3);
    TEST_ASSERT_NOT_EQUAL(reading1, reading3);
}

void test_oil_pressure_sensor_boundary_values() {
    sensor->init();
    
    // Test minimum value (0 ADC)
    mockGpio->setAnalogValue(gpio_pins::OIL_PRESSURE, 0);
    Reading minPressureReading = sensor->getReading();
    double minPressure = std::get<double>(minPressureReading);
    TEST_ASSERT_GREATER_OR_EQUAL(0.0, minPressure);
    
    // Test maximum value (4095 ADC for 12-bit)
    mockGpio->setAnalogValue(gpio_pins::OIL_PRESSURE, 4095);
    Reading maxPressureReading = sensor->getReading();
    double maxPressure = std::get<double>(maxPressureReading);
    TEST_ASSERT_GREATER_THAN(minPressure, maxPressure);
    TEST_ASSERT_LESS_THAN(200.0, maxPressure); // Sanity check
}

void runOilPressureSensorTests() {
    setUp_oil_pressure_sensor();
    RUN_TEST(test_oil_pressure_sensor_init);
    RUN_TEST(test_oil_pressure_sensor_reading_conversion);
    RUN_TEST(test_oil_pressure_sensor_value_change_detection);
    RUN_TEST(test_oil_pressure_sensor_boundary_values);
    tearDown_oil_pressure_sensor();
}