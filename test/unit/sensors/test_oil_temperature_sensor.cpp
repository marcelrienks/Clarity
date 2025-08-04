#include <unity.h>
#include "sensors/oil_temperature_sensor.h"
#include "mocks/mock_gpio_provider.h"
#include "hardware/gpio_pins.h"

// Arduino functions are already mocked in test/mocks/Arduino.h
extern void set_mock_millis(uint32_t value);

static MockGpioProvider* oilTempMockGpio;
static OilTemperatureSensor* oilTempSensor;

void setUp_oil_temperature_sensor() {
    oilTempMockGpio = new MockGpioProvider();
    oilTempSensor = new OilTemperatureSensor(oilTempMockGpio);
}

void tearDown_oil_temperature_sensor() {
    delete oilTempSensor;
    delete oilTempMockGpio;
}

void test_oil_temperature_sensor_init() {
    oilTempSensor->init();
    
    // Verify that the sensor initializes without throwing
    TEST_ASSERT_TRUE(true);
}

void test_oil_temperature_sensor_reading_conversion() {
    // Set a known ADC value
    uint16_t testAdcValue = 2048; // Mid-range 12-bit value
    oilTempMockGpio->setAnalogValue(gpio_pins::OIL_TEMPERATURE, testAdcValue);
    
    oilTempSensor->init();
    
    // Get the reading
    Reading tempReading = oilTempSensor->getReading();
    int32_t temperature = std::get<int32_t>(tempReading);
    
    // Temperature should be reasonable (0-120°C range)
    TEST_ASSERT_GREATER_OR_EQUAL(0, temperature);
    TEST_ASSERT_LESS_OR_EQUAL(120, temperature);
}

void test_oil_temperature_sensor_value_change_detection() {
    oilTempSensor->init();
    
    // Set initial value
    oilTempMockGpio->setAnalogValue(gpio_pins::OIL_TEMPERATURE, 1000);
    Reading tempReading1 = oilTempSensor->getReading();
    int32_t reading1 = std::get<int32_t>(tempReading1);
    
    // Same value should give same reading (within update interval)
    Reading tempReading2 = oilTempSensor->getReading();
    int32_t reading2 = std::get<int32_t>(tempReading2);
    TEST_ASSERT_EQUAL_INT32(reading1, reading2);
    
    // Advance time and set different value to trigger update
    set_mock_millis(1500);
    oilTempMockGpio->setAnalogValue(gpio_pins::OIL_TEMPERATURE, 2000);
    Reading tempReading3 = oilTempSensor->getReading();
    int32_t reading3 = std::get<int32_t>(tempReading3);
    TEST_ASSERT_NOT_EQUAL(reading1, reading3);
}

void test_oil_temperature_sensor_boundary_values() {
    oilTempSensor->init();
    
    // Test minimum value (0 ADC)
    set_mock_millis(0);
    oilTempMockGpio->setAnalogValue(gpio_pins::OIL_TEMPERATURE, 0);
    Reading minTempReading = oilTempSensor->getReading();
    int32_t minTemp = std::get<int32_t>(minTempReading);
    
    // Advance time and test maximum value (4095 ADC for 12-bit)
    set_mock_millis(1500);
    oilTempMockGpio->setAnalogValue(gpio_pins::OIL_TEMPERATURE, 4095);
    Reading maxTempReading = oilTempSensor->getReading();
    int32_t maxTemp = std::get<int32_t>(maxTempReading);
    
    // Max temperature should be higher than min temperature
    TEST_ASSERT_GREATER_THAN(minTemp, maxTemp);
    
    // Temperatures should be in range (0-120°C)
    TEST_ASSERT_GREATER_OR_EQUAL(0, minTemp);
    TEST_ASSERT_LESS_OR_EQUAL(120, maxTemp);
}

void test_oil_temperature_sensor_construction() {
    // Test that sensor can be created and destroyed
    TEST_ASSERT_NOT_NULL(oilTempSensor);
}

void runOilTemperatureSensorTests() {
    setUp_oil_temperature_sensor();
    RUN_TEST(test_oil_temperature_sensor_construction);
    RUN_TEST(test_oil_temperature_sensor_init);
    RUN_TEST(test_oil_temperature_sensor_reading_conversion);
    RUN_TEST(test_oil_temperature_sensor_value_change_detection);
    RUN_TEST(test_oil_temperature_sensor_boundary_values);
    tearDown_oil_temperature_sensor();
}