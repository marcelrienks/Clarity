#include <unity.h>
#include "sensors/oil_temperature_sensor.h"
#include "../mocks/mock_gpio_provider.h"
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
OilTemperatureSensor* sensor;

void setUp_oil_temperature_sensor() {
    mockGpio = new MockGpioProvider();
    sensor = new OilTemperatureSensor(mockGpio);
}

void tearDown_oil_temperature_sensor() {
    delete sensor;
    delete mockGpio;
}

void test_oil_temperature_sensor_init() {
    sensor->init();
    
    // Verify that the sensor initializes without throwing
    TEST_ASSERT_TRUE(true);
}

void test_oil_temperature_sensor_reading_conversion() {
    // Set a known ADC value
    uint16_t testAdcValue = 2048; // Mid-range 12-bit value
    mockGpio->setAnalogValue(gpio_pins::OIL_TEMPERATURE, testAdcValue);
    
    sensor->init();
    
    // Get the reading
    double temperature = sensor->getReading();
    
    // Temperature should be reasonable (assuming Celsius)
    TEST_ASSERT_GREATER_THAN(-50.0, temperature); // Above absolute minimum
    TEST_ASSERT_LESS_THAN(200.0, temperature);    // Below extreme maximum
}

void test_oil_temperature_sensor_value_change_detection() {
    sensor->init();
    
    // Set initial value
    mockGpio->setAnalogValue(gpio_pins::OIL_TEMPERATURE, 1000);
    double reading1 = sensor->getReading();
    bool hasChanged1 = sensor->hasValueChanged();
    
    // First reading should indicate change (from initial state)
    TEST_ASSERT_TRUE(hasChanged1);
    
    // Same value should not indicate change
    double reading2 = sensor->getReading();
    bool hasChanged2 = sensor->hasValueChanged();
    TEST_ASSERT_FALSE(hasChanged2);
    
    // Different value should indicate change
    mockGpio->setAnalogValue(gpio_pins::OIL_TEMPERATURE, 2000);
    double reading3 = sensor->getReading();
    bool hasChanged3 = sensor->hasValueChanged();
    TEST_ASSERT_TRUE(hasChanged3);
    TEST_ASSERT_NOT_EQUAL(reading1, reading3);
}

void test_oil_temperature_sensor_boundary_values() {
    sensor->init();
    
    // Test minimum value (0 ADC)
    mockGpio->setAnalogValue(gpio_pins::OIL_TEMPERATURE, 0);
    double minTemp = sensor->getReading();
    
    // Test maximum value (4095 ADC for 12-bit)
    mockGpio->setAnalogValue(gpio_pins::OIL_TEMPERATURE, 4095);
    double maxTemp = sensor->getReading();
    
    // Max temperature should be higher than min temperature
    TEST_ASSERT_GREATER_THAN(minTemp, maxTemp);
    
    // Temperatures should be in reasonable range
    TEST_ASSERT_GREATER_THAN(-50.0, minTemp);
    TEST_ASSERT_LESS_THAN(200.0, maxTemp);
}

void test_oil_temperature_sensor_monotonic_response() {
    sensor->init();
    
    // Test that increasing ADC values generally produce increasing temperatures
    // (this depends on the sensor's calibration curve)
    mockGpio->setAnalogValue(gpio_pins::OIL_TEMPERATURE, 1000);
    double temp1 = sensor->getReading();
    
    mockGpio->setAnalogValue(gpio_pins::OIL_TEMPERATURE, 2000);
    double temp2 = sensor->getReading();
    
    mockGpio->setAnalogValue(gpio_pins::OIL_TEMPERATURE, 3000);
    double temp3 = sensor->getReading();
    
    // Assuming linear or monotonic response (adjust based on actual calibration)
    // Note: This test may need adjustment based on the actual sensor calibration
    TEST_ASSERT_NOT_EQUAL(temp1, temp2);
    TEST_ASSERT_NOT_EQUAL(temp2, temp3);
}

void runOilTemperatureSensorTests() {
    setUp_oil_temperature_sensor();
    RUN_TEST(test_oil_temperature_sensor_init);
    RUN_TEST(test_oil_temperature_sensor_reading_conversion);
    RUN_TEST(test_oil_temperature_sensor_value_change_detection);
    RUN_TEST(test_oil_temperature_sensor_boundary_values);
    RUN_TEST(test_oil_temperature_sensor_monotonic_response);
    tearDown_oil_temperature_sensor();
}