#include <unity.h>
#include "mock_utilities.h"
#include "mock_managers.h"
#include "mock_types.h"

// External declarations for test state
extern int32_t current_oil_temperature;
extern int32_t current_oil_pressure;
extern uint32_t last_update_time;
extern bool sensor_initialized;

void test_sensor_update_interval(void) {
    // Test that sensor readings are updated at appropriate intervals
    uint32_t start_time = 0;
    last_update_time = start_time;
    
    // Simulate passage of time less than update interval
    uint32_t current_time = start_time + 100; // 100ms
    UpdateSensorReadings(current_time);
    TEST_ASSERT_EQUAL(start_time, last_update_time);
    
    // Simulate passage of time greater than update interval
    current_time = start_time + 1100; // 1.1s
    UpdateSensorReadings(current_time);
    TEST_ASSERT_EQUAL(current_time, last_update_time);
}

void test_sensor_reading_bounds(void) {
    // Test that sensor readings stay within expected bounds
    TEST_ASSERT_LESS_OR_EQUAL(MAX_OIL_TEMPERATURE, current_oil_temperature);
    TEST_ASSERT_GREATER_OR_EQUAL(MIN_OIL_TEMPERATURE, current_oil_temperature);
    
    TEST_ASSERT_LESS_OR_EQUAL(MAX_OIL_PRESSURE, current_oil_pressure);
    TEST_ASSERT_GREATER_OR_EQUAL(MIN_OIL_PRESSURE, current_oil_pressure);
}

void test_oil_temperature_reading_accuracy(void) {
    // Test oil temperature sensor accuracy
    int32_t expected_temp = 90;
    SetOilTemperature(expected_temp);
    TEST_ASSERT_EQUAL(expected_temp, GetOilTemperature());
}

void test_oil_pressure_reading_accuracy(void) {
    // Test oil pressure sensor accuracy
    int32_t expected_pressure = 45;
    SetOilPressure(expected_pressure);
    TEST_ASSERT_EQUAL(expected_pressure, GetOilPressure());
}

void test_oil_temperature_sensor_initialization(void) {
    // Test temperature sensor initialization
    sensor_initialized = false;
    InitializeOilTemperatureSensor();
    TEST_ASSERT_TRUE(sensor_initialized);
    TEST_ASSERT_EQUAL(DEFAULT_OIL_TEMPERATURE, current_oil_temperature);
}

void test_oil_pressure_sensor_initialization(void) {
    // Test pressure sensor initialization
    sensor_initialized = false;
    InitializeOilPressureSensor();
    TEST_ASSERT_TRUE(sensor_initialized);
    TEST_ASSERT_EQUAL(DEFAULT_OIL_PRESSURE, current_oil_pressure);
}
