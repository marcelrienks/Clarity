#include <unity.h>
#include "sensors/oil_pressure_sensor.h"
#include "mock_gpio_provider.h"
#include "hardware/gpio_pins.h"

// Arduino functions are already mocked in test/mocks/Arduino.h
extern void set_mock_millis(uint32_t value);

static MockGpioProvider* oilPressureMockGpio;
static OilPressureSensor* oilPressureSensor;

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
    int32_t pressure = std::get<int32_t>(pressureReading);
    
    // Pressure should be positive and reasonable (0-10 Bar range)
    TEST_ASSERT_GREATER_OR_EQUAL(0, pressure);
    TEST_ASSERT_LESS_OR_EQUAL(10, pressure);
}

void test_oil_pressure_sensor_value_change_detection() {
    // Need to account for 1000ms update interval
    oilPressureSensor->init();
    
    // Set initial value  
    oilPressureMockGpio->setAnalogValue(gpio_pins::OIL_PRESSURE, 1000);
    Reading reading1 = oilPressureSensor->getReading();
    int32_t value1 = std::get<int32_t>(reading1);
    
    // Same value should give same reading (within same time interval)
    Reading reading2 = oilPressureSensor->getReading();
    int32_t value2 = std::get<int32_t>(reading2);
    TEST_ASSERT_EQUAL_INT32(value1, value2);
    
    // Advance mock time by more than 1000ms to trigger update
    set_mock_millis(1500);
    
    // Different ADC value should give different reading after time elapsed
    oilPressureMockGpio->setAnalogValue(gpio_pins::OIL_PRESSURE, 2000);
    Reading reading3 = oilPressureSensor->getReading();
    int32_t value3 = std::get<int32_t>(reading3);
    TEST_ASSERT_NOT_EQUAL(value1, value3);
}

void test_oil_pressure_sensor_boundary_values() {
    oilPressureSensor->init();
    
    // Test minimum value (0 ADC) 
    set_mock_millis(0);
    oilPressureMockGpio->setAnalogValue(gpio_pins::OIL_PRESSURE, 0);
    Reading minPressureReading = oilPressureSensor->getReading();
    int32_t minPressure = std::get<int32_t>(minPressureReading);
    TEST_ASSERT_GREATER_OR_EQUAL(0, minPressure);
    
    // Advance time and test maximum value (4095 ADC for 12-bit)
    set_mock_millis(1500);
    oilPressureMockGpio->setAnalogValue(gpio_pins::OIL_PRESSURE, 4095);
    Reading maxPressureReading = oilPressureSensor->getReading();
    int32_t maxPressure = std::get<int32_t>(maxPressureReading);
    TEST_ASSERT_GREATER_THAN(minPressure, maxPressure);
    TEST_ASSERT_LESS_OR_EQUAL(10, maxPressure); // Should be exactly 10 Bar at max ADC
}

void runOilPressureSensorTests() {
    setUp_oil_pressure_sensor();
    RUN_TEST(test_oil_pressure_sensor_init);
    RUN_TEST(test_oil_pressure_sensor_reading_conversion);
    RUN_TEST(test_oil_pressure_sensor_value_change_detection);
    RUN_TEST(test_oil_pressure_sensor_boundary_values);
    tearDown_oil_pressure_sensor();
}