#include <unity.h>
#include "sensors/oil_pressure_sensor.h"
#include "mock_gpio_provider.h"
#include "hardware/gpio_pins.h"

// Arduino functions are already mocked in test/mocks/Arduino.h

MockGpioProvider* oilPressureMockGpio;
OilPressureSensor* oilPressureSensor;

void setUp_oil_pressure_sensor() {
    oilPressureMockGpio = new MockGpioProvider();
    oilPressureSensor = new OilPressureSensor(oilPressureMockGpio);
}

void tearDown_oil_pressure_sensor() {
    delete oilPressureSensor;
    delete oilPressureMockGpio;
}

void test_oil_pressure_sensor_init() {
    oilPressureSensor->init();
    
    // Verify that the oilPressureSensor initializes without throwing
    TEST_ASSERT_TRUE(true);
}

void test_oil_pressure_sensor_reading_conversion() {
    // Set a known ADC value
    uint16_t testAdcValue = 2048; // Mid-range 12-bit value
    oilPressureMockGpio->setAnalogValue(gpio_pins::OIL_PRESSURE, testAdcValue);
    
    oilPressureSensor->init();
    
    // Get the reading
    Reading pressureReading = oilPressureSensor->getReading();
    double pressure = std::get<double>(pressureReading);
    
    // Pressure should be positive and reasonable
    TEST_ASSERT_GREATER_THAN(0.0, pressure);
    TEST_ASSERT_LESS_THAN(200.0, pressure); // Reasonable max pressure
}

void test_oil_pressure_sensor_value_change_detection() {
    oilPressureSensor->init();
    
    // Set initial value
    oilPressureMockGpio->setAnalogValue(gpio_pins::OIL_PRESSURE, 1000);
    Reading reading1 = oilPressureSensor->getReading();
    double value1 = std::get<double>(reading1);
    
    // Same value should give same reading
    Reading reading2 = oilPressureSensor->getReading();
    double value2 = std::get<double>(reading2);
    TEST_ASSERT_EQUAL_DOUBLE(value1, value2);
    
    // Different value should give different reading
    oilPressureMockGpio->setAnalogValue(gpio_pins::OIL_PRESSURE, 2000);
    Reading reading3 = oilPressureSensor->getReading();
    double value3 = std::get<double>(reading3);
    TEST_ASSERT_NOT_EQUAL(value1, value3);
}

void test_oil_pressure_sensor_boundary_values() {
    oilPressureSensor->init();
    
    // Test minimum value (0 ADC)
    oilPressureMockGpio->setAnalogValue(gpio_pins::OIL_PRESSURE, 0);
    Reading minPressureReading = oilPressureSensor->getReading();
    double minPressure = std::get<double>(minPressureReading);
    TEST_ASSERT_GREATER_OR_EQUAL(0.0, minPressure);
    
    // Test maximum value (4095 ADC for 12-bit)
    oilPressureMockGpio->setAnalogValue(gpio_pins::OIL_PRESSURE, 4095);
    Reading maxPressureReading = oilPressureSensor->getReading();
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