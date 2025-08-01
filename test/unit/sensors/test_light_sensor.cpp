#include <unity.h>
#include "sensors/light_sensor.h"
#include "../mocks/mock_gpio_provider.h"
#include "hardware/gpio_pins.h"
#include "utilities/types.h"

MockGpioProvider* mockGpio;
LightSensor* sensor;

void setUp_light_sensor() {
    mockGpio = new MockGpioProvider();
    sensor = new LightSensor(mockGpio);
}

void tearDown_light_sensor() {
    delete sensor;
    delete mockGpio;
}

void test_light_sensor_init() {
    // Test initialization doesn't crash
    sensor->init();
    TEST_ASSERT_TRUE(true);
}

void test_light_sensor_reading_conversion() {
    sensor->init();
    
    // Set a known ADC value
    uint16_t testAdcValue = 2048; // Mid-range 12-bit value
    mockGpio->setAnalogValue(gpio_pins::LIGHTS, testAdcValue);
    
    // Get the reading
    double lightLevel = sensor->getReading();
    
    // Light level should be reasonable
    TEST_ASSERT_GREATER_THAN(0.0, lightLevel);
    TEST_ASSERT_LESS_THAN(5000.0, lightLevel); // Reasonable max for light sensor
}

void test_light_sensor_boundary_values() {
    sensor->init();
    
    // Test minimum value (0 ADC)
    mockGpio->setAnalogValue(gpio_pins::LIGHTS, 0);
    double minLight = sensor->getReading();
    TEST_ASSERT_GREATER_OR_EQUAL(0.0, minLight);
    
    // Test maximum value (4095 ADC for 12-bit)
    mockGpio->setAnalogValue(gpio_pins::LIGHTS, 4095);
    double maxLight = sensor->getReading();
    TEST_ASSERT_GREATER_THAN(minLight, maxLight);
}

void test_light_sensor_value_change_detection() {
    sensor->init();
    
    // Set initial value
    mockGpio->setAnalogValue(gpio_pins::LIGHTS, 1000);
    double reading1 = sensor->getReading();
    bool hasChanged1 = sensor->hasValueChanged();
    
    // First reading should indicate change (from initial state)
    TEST_ASSERT_TRUE(hasChanged1);
    
    // Same value should not indicate change
    double reading2 = sensor->getReading();
    bool hasChanged2 = sensor->hasValueChanged();
    TEST_ASSERT_FALSE(hasChanged2);
    
    // Different value should indicate change
    mockGpio->setAnalogValue(gpio_pins::LIGHTS, 2000);
    double reading3 = sensor->getReading();
    bool hasChanged3 = sensor->hasValueChanged();
    TEST_ASSERT_TRUE(hasChanged3);
    TEST_ASSERT_NOT_EQUAL(reading1, reading3);
}

void test_light_sensor_construction() {
    // Test that sensor can be created and destroyed
    TEST_ASSERT_NOT_NULL(sensor);
}

void test_light_sensor_reading_consistency() {
    sensor->init();
    
    // Set a known value
    mockGpio->setAnalogValue(gpio_pins::LIGHTS, 1500);
    
    // Multiple readings should be consistent
    double reading1 = sensor->getReading();
    double reading2 = sensor->getReading();
    
    TEST_ASSERT_EQUAL_DOUBLE(reading1, reading2);
}

void test_light_sensor_monotonic_response() {
    sensor->init();
    
    // Test that increasing ADC values produce increasing light values
    mockGpio->setAnalogValue(gpio_pins::LIGHTS, 1000);
    double light1 = sensor->getReading();
    
    mockGpio->setAnalogValue(gpio_pins::LIGHTS, 2000);
    double light2 = sensor->getReading();
    
    mockGpio->setAnalogValue(gpio_pins::LIGHTS, 3000);
    double light3 = sensor->getReading();
    
    // Light values should generally increase with ADC value
    TEST_ASSERT_GREATER_THAN(light1, light2);
    TEST_ASSERT_GREATER_THAN(light2, light3);
}

void runLightSensorTests() {
    setUp_light_sensor();
    RUN_TEST(test_light_sensor_construction);
    RUN_TEST(test_light_sensor_init);
    RUN_TEST(test_light_sensor_reading_conversion);
    RUN_TEST(test_light_sensor_boundary_values);
    RUN_TEST(test_light_sensor_value_change_detection);
    RUN_TEST(test_light_sensor_reading_consistency);
    RUN_TEST(test_light_sensor_monotonic_response);
    tearDown_light_sensor();
}