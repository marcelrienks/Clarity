#include <unity.h>
#include "sensors/light_sensor.h"
#include "mock_gpio_provider.h"
#include "hardware/gpio_pins.h"
#include "utilities/types.h"

MockGpioProvider* lightMockGpio;
LightSensor* lightSensor;

void setUp_light_sensor() {
    lightMockGpio = new MockGpioProvider();
    lightSensor = new LightSensor(lightMockGpio);
}

void tearDown_light_sensor() {
    delete lightSensor;
    delete lightMockGpio;
}

void test_light_sensor_init() {
    // Test initialization doesn't crash
    lightSensor->init();
    TEST_ASSERT_TRUE(true);
}

void test_light_sensor_reading_conversion() {
    lightSensor->init();
    
    // Set a known ADC value
    uint16_t testAdcValue = 2048; // Mid-range 12-bit value
    lightMockGpio->setAnalogValue(gpio_pins::LIGHTS, testAdcValue);
    
    // Get the reading
    Reading lightReading = lightSensor->getReading();
    double lightLevel = std::get<double>(lightReading);
    
    // Light level should be reasonable
    TEST_ASSERT_GREATER_THAN(0.0, lightLevel);
    TEST_ASSERT_LESS_THAN(5000.0, lightLevel); // Reasonable max for light lightSensor
}

void test_light_sensor_boundary_values() {
    lightSensor->init();
    
    // Test minimum value (0 ADC)
    lightMockGpio->setAnalogValue(gpio_pins::LIGHTS, 0);
    Reading minLightReading = lightSensor->getReading();
    double minLight = std::get<double>(minLightReading);
    TEST_ASSERT_GREATER_OR_EQUAL(0.0, minLight);
    
    // Test maximum value (4095 ADC for 12-bit)
    lightMockGpio->setAnalogValue(gpio_pins::LIGHTS, 4095);
    Reading maxLightReading = lightSensor->getReading();
    double maxLight = std::get<double>(maxLightReading);
    TEST_ASSERT_GREATER_THAN(minLight, maxLight);
}

void test_light_sensor_value_change_detection() {
    lightSensor->init();
    
    // Set initial value
    lightMockGpio->setAnalogValue(gpio_pins::LIGHTS, 1000);
    Reading lightReading1 = lightSensor->getReading();
    double reading1 = std::get<double>(lightReading1);
    
    // Same value should give same reading
    Reading lightReading2 = lightSensor->getReading();
    double reading2 = std::get<double>(lightReading2);
    TEST_ASSERT_EQUAL_DOUBLE(reading1, reading2);
    
    // Different value should give different reading
    lightMockGpio->setAnalogValue(gpio_pins::LIGHTS, 2000);
    Reading lightReading3 = lightSensor->getReading();
    double reading3 = std::get<double>(lightReading3);
    TEST_ASSERT_NOT_EQUAL(reading1, reading3);
}

void test_light_sensor_construction() {
    // Test that lightSensor can be created and destroyed
    TEST_ASSERT_NOT_NULL(lightSensor);
}

void test_light_sensor_reading_consistency() {
    lightSensor->init();
    
    // Set a known value
    lightMockGpio->setDigitalValue(gpio_pins::LIGHTS, true);
    
    // Multiple readings should be consistent
    Reading reading1 = lightSensor->getReading();
    Reading reading2 = lightSensor->getReading();
    
    TEST_ASSERT_EQUAL(std::get<bool>(reading1), std::get<bool>(reading2));
}

void test_light_sensor_monotonic_response() {
    lightSensor->init();
    
    // Test that increasing ADC values produce increasing light values
    lightMockGpio->setAnalogValue(gpio_pins::LIGHTS, 1000);
    Reading light1Reading = lightSensor->getReading();
    double light1 = std::get<double>(light1Reading);
    
    lightMockGpio->setAnalogValue(gpio_pins::LIGHTS, 2000);
    Reading light2Reading = lightSensor->getReading();
    double light2 = std::get<double>(light2Reading);
    
    lightMockGpio->setAnalogValue(gpio_pins::LIGHTS, 3000);
    Reading light3Reading = lightSensor->getReading();
    double light3 = std::get<double>(light3Reading);
    
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