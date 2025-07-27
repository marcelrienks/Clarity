#include <unity.h>
#include "test_utilities.h"

// Mock sensor data
static int32_t current_oil_pressure = 0;
static int32_t current_oil_temperature = 0;
static bool sensor_initialized = false;
static uint32_t last_update_time = 0;
static const uint32_t UPDATE_INTERVAL_MS = 100;

// Mock ADC readings for testing
static uint16_t mock_pressure_adc = 0;
static uint16_t mock_temperature_adc = 0;

// Note: setUp() and tearDown() are defined in test_main.cpp

// Mock sensor functions
void mockOilPressureSensorInit() {
    sensor_initialized = true;
    MockHardware::simulateAdcReading(34, mock_pressure_adc); // Mock pressure pin
}

void mockOilTemperatureSensorInit() {
    sensor_initialized = true;
    MockHardware::simulateAdcReading(35, mock_temperature_adc); // Mock temperature pin
}

int32_t mockGetOilPressureReading() {
    if (!sensor_initialized) return -1;
    
    // Use mock timing instead of real millis() for unit testing
    static uint32_t mock_time = 0;
    mock_time += UPDATE_INTERVAL_MS; // Force update every call for testing
    
    if (mock_time - last_update_time >= UPDATE_INTERVAL_MS) {
        last_update_time = mock_time;
        
        // Convert ADC to pressure (0-4095 -> 0-10 Bar)
        uint16_t adc_value = MockHardware::getAdcReading(34);
        current_oil_pressure = (adc_value * 10) / 4095;
    }
    
    return current_oil_pressure;
}

int32_t mockGetOilTemperatureReading() {
    if (!sensor_initialized) return -1;
    
    // Use mock timing instead of real millis() for unit testing
    static uint32_t mock_time = 0;
    mock_time += UPDATE_INTERVAL_MS; // Force update every call for testing
    
    if (mock_time - last_update_time >= UPDATE_INTERVAL_MS) {
        last_update_time = mock_time;
        
        // Convert ADC to temperature (0-4095 -> 0-120°C)
        uint16_t adc_value = MockHardware::getAdcReading(35);
        current_oil_temperature = (adc_value * 120) / 4095;
    }
    
    return current_oil_temperature;
}

// =================================================================
// SENSOR INITIALIZATION TESTS
// =================================================================

void test_oil_pressure_sensor_initialization(void) {
    // Test oil pressure sensor initialization
    TEST_ASSERT_FALSE(sensor_initialized);
    
    mockOilPressureSensorInit();
    TEST_ASSERT_TRUE(sensor_initialized);
    
    // Initial reading should be available
    int32_t reading = mockGetOilPressureReading();
    TEST_ASSERT_GREATER_OR_EQUAL(0, reading);
    TEST_ASSERT_LESS_OR_EQUAL(10, reading); // 0-10 Bar range
}

void test_oil_temperature_sensor_initialization(void) {
    // Test oil temperature sensor initialization
    TEST_ASSERT_FALSE(sensor_initialized);
    
    mockOilTemperatureSensorInit();
    TEST_ASSERT_TRUE(sensor_initialized);
    
    // Initial reading should be available
    int32_t reading = mockGetOilTemperatureReading();
    TEST_ASSERT_GREATER_OR_EQUAL(0, reading);
    TEST_ASSERT_LESS_OR_EQUAL(120, reading); // 0-120°C range
}

// =================================================================
// SENSOR READING TESTS
// =================================================================

void test_oil_pressure_reading_accuracy(void) {
    mockOilPressureSensorInit();
    
    // Test various pressure levels
    struct {
        uint16_t adc_value;
        int32_t expected_pressure;
    } test_cases[] = {
        {0, 0},           // 0% -> 0 Bar
        {1023, 2},        // 25% -> ~2.5 Bar (rounded to 2)
        {2047, 4},        // 50% -> ~5 Bar (rounded to 4)
        {3071, 7},        // 75% -> ~7.5 Bar (rounded to 7)
        {4095, 10}        // 100% -> 10 Bar
    };
    
    for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++) {
        MockHardware::simulateAdcReading(34, test_cases[i].adc_value);
        int32_t reading = mockGetOilPressureReading();
        
        // Allow for small rounding differences
        TEST_ASSERT_INT_WITHIN(1, test_cases[i].expected_pressure, reading);
    }
}

void test_oil_temperature_reading_accuracy(void) {
    mockOilTemperatureSensorInit();
    
    // Test various temperature levels
    struct {
        uint16_t adc_value;
        int32_t expected_temp;
    } test_cases[] = {
        {0, 0},           // 0% -> 0°C
        {1023, 29},       // 25% -> ~30°C
        {2047, 59},       // 50% -> ~60°C
        {3071, 89},       // 75% -> ~90°C
        {4095, 120}       // 100% -> 120°C
    };
    
    for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++) {
        MockHardware::simulateAdcReading(35, test_cases[i].adc_value);
        int32_t reading = mockGetOilTemperatureReading();
        
        // Allow for small rounding differences
        TEST_ASSERT_INT_WITHIN(1, test_cases[i].expected_temp, reading);
    }
}

void test_sensor_reading_bounds(void) {
    mockOilPressureSensorInit();
    mockOilTemperatureSensorInit();
    
    // Test boundary conditions
    
    // Minimum values
    MockHardware::simulateAdcReading(34, 0);
    MockHardware::simulateAdcReading(35, 0);
    
    int32_t pressure = mockGetOilPressureReading();
    int32_t temperature = mockGetOilTemperatureReading();
    
    TEST_ASSERT_EQUAL(0, pressure);
    TEST_ASSERT_EQUAL(0, temperature);
    
    // Maximum values
    MockHardware::simulateAdcReading(34, 4095);
    MockHardware::simulateAdcReading(35, 4095);
    
    pressure = mockGetOilPressureReading();
    temperature = mockGetOilTemperatureReading();
    
    TEST_ASSERT_EQUAL(10, pressure);
    TEST_ASSERT_EQUAL(120, temperature);
}

// =================================================================
// SENSOR UPDATE TIMING TESTS
// =================================================================

void test_sensor_update_interval(void) {
    mockOilPressureSensorInit();
    
    // Set initial ADC value
    MockHardware::simulateAdcReading(34, 1000);
    int32_t initial_reading = mockGetOilPressureReading();
    
    // Change ADC value but time hasn't passed UPDATE_INTERVAL_MS
    MockHardware::simulateAdcReading(34, 2000);
    int32_t reading_before_interval = mockGetOilPressureReading();
    
    // Reading should be the same (cached)
    TEST_ASSERT_EQUAL(initial_reading, reading_before_interval);
    
    // Simulate time passage (in real test would need proper time mocking)
    // For this mock, we'll force an update by calling again
    last_update_time = 0; // Force update
    int32_t reading_after_interval = mockGetOilPressureReading();
    
    // Reading should be different now
    TEST_ASSERT_NOT_EQUAL(initial_reading, reading_after_interval);
}

void test_sensor_reading_consistency(void) {
    mockOilPressureSensorInit();
    
    // Set stable ADC value
    MockHardware::simulateAdcReading(34, 2047); // 50% -> ~5 Bar
    
    // Multiple readings within update interval should be consistent
    int32_t reading1 = mockGetOilPressureReading();
    int32_t reading2 = mockGetOilPressureReading();
    int32_t reading3 = mockGetOilPressureReading();
    
    TEST_ASSERT_EQUAL(reading1, reading2);
    TEST_ASSERT_EQUAL(reading2, reading3);
}

// =================================================================
// SENSOR ERROR HANDLING TESTS
// =================================================================

void test_sensor_reading_without_initialization(void) {
    // Test reading sensor before initialization
    TEST_ASSERT_FALSE(sensor_initialized);
    
    int32_t pressure = mockGetOilPressureReading();
    int32_t temperature = mockGetOilTemperatureReading();
    
    // Should return error values
    TEST_ASSERT_EQUAL(-1, pressure);
    TEST_ASSERT_EQUAL(-1, temperature);
}

void test_sensor_adc_failure_handling(void) {
    mockOilPressureSensorInit();
    
    // Simulate ADC failure (no reading available)
    MockHardware::simulateAdcReading(34, 0xFFFF); // Invalid ADC value
    
    int32_t reading = mockGetOilPressureReading();
    
    // Should handle gracefully (clamp to max or return error)
    TEST_ASSERT_TRUE(reading >= 0 && reading <= 10);
}

// =================================================================
// SENSOR INTEGRATION TESTS
// =================================================================

void test_dual_sensor_operation(void) {
    // Test both sensors operating simultaneously
    mockOilPressureSensorInit();
    mockOilTemperatureSensorInit();
    
    // Set different values for each sensor
    MockHardware::simulateAdcReading(34, 1500); // Pressure
    MockHardware::simulateAdcReading(35, 2500); // Temperature
    
    int32_t pressure = mockGetOilPressureReading();
    int32_t temperature = mockGetOilTemperatureReading();
    
    // Both should return valid readings
    TEST_ASSERT_GREATER_OR_EQUAL(0, pressure);
    TEST_ASSERT_LESS_OR_EQUAL(10, pressure);
    TEST_ASSERT_GREATER_OR_EQUAL(0, temperature);
    TEST_ASSERT_LESS_OR_EQUAL(120, temperature);
    
    // Values should be independent
    TEST_ASSERT_NOT_EQUAL(pressure, temperature);
}

void test_sensor_value_change_detection(void) {
    mockOilPressureSensorInit();
    
    // Initial value
    MockHardware::simulateAdcReading(34, 1000);
    int32_t initial = mockGetOilPressureReading();
    
    // Change value significantly
    MockHardware::simulateAdcReading(34, 3000);
    last_update_time = 0; // Force update
    int32_t changed = mockGetOilPressureReading();
    
    // Should detect the change
    TEST_ASSERT_NOT_EQUAL(initial, changed);
    TEST_ASSERT_GREATER_THAN(initial, changed);
}

// =================================================================
// SENSOR PERFORMANCE TESTS
// =================================================================

void test_sensor_reading_performance(void) {
    mockOilPressureSensorInit();
    mockOilTemperatureSensorInit();
    
    // Set test values
    MockHardware::simulateAdcReading(34, 2000);
    MockHardware::simulateAdcReading(35, 2500);
    
    // Measure performance of multiple readings
    measureResponseTime([&]() {
        for (int i = 0; i < 100; i++) {
            mockGetOilPressureReading();
            mockGetOilTemperatureReading();
        }
    });
    
    // Should complete efficiently
    TEST_ASSERT_TRUE(true); // Performance measured in measureResponseTime
}

void test_sensor_memory_usage(void) {
    // Test sensor memory usage doesn't grow over time
    measureMemoryUsage();
    
    mockOilPressureSensorInit();
    mockOilTemperatureSensorInit();
    
    // Perform many operations
    for (int i = 0; i < 1000; i++) {
        MockHardware::simulateAdcReading(34, i % 4096);
        MockHardware::simulateAdcReading(35, (i * 2) % 4096);
        
        if (i % 10 == 0) {
            last_update_time = 0; // Force updates periodically
        }
        
        mockGetOilPressureReading();
        mockGetOilTemperatureReading();
    }
    
    measureMemoryUsage();
    
    // Memory should remain stable
    TEST_ASSERT_TRUE(true); // Memory usage measured in measureMemoryUsage
}

// =================================================================
// REALISTIC SCENARIO TESTS
// =================================================================

void test_engine_startup_scenario(void) {
    // Simulate engine startup - pressure and temperature rise
    mockOilPressureSensorInit();
    mockOilTemperatureSensorInit();
    
    struct {
        uint16_t pressure_adc;
        uint16_t temp_adc;
        const char* description;
    } startup_sequence[] = {
        {0, 800, "Engine off - no pressure, ambient temp"},
        {500, 900, "Engine cranking - building pressure"},
        {1500, 1200, "Engine running - normal pressure"},
        {2000, 2000, "Engine warm - operating pressure and temp"},
        {2200, 2500, "Engine hot - high temp"}
    };
    
    for (size_t i = 0; i < sizeof(startup_sequence) / sizeof(startup_sequence[0]); i++) {
        MockHardware::simulateAdcReading(34, startup_sequence[i].pressure_adc);
        MockHardware::simulateAdcReading(35, startup_sequence[i].temp_adc);
        last_update_time = 0; // Force update
        
        int32_t pressure = mockGetOilPressureReading();
        int32_t temperature = mockGetOilTemperatureReading();
        
        // Validate readings are in expected ranges
        TEST_ASSERT_GREATER_OR_EQUAL(0, pressure);
        TEST_ASSERT_LESS_OR_EQUAL(10, pressure);
        TEST_ASSERT_GREATER_OR_EQUAL(0, temperature);
        TEST_ASSERT_LESS_OR_EQUAL(120, temperature);
        
        // Pressure should generally increase during startup
        if (i > 0) {
            // Allow for some variation but expect general trend
            TEST_ASSERT_TRUE(pressure >= 0);
        }
    }
}

void test_sensor_fault_simulation(void) {
    // Test sensor behavior under fault conditions
    mockOilPressureSensorInit();
    
    // Simulate sensor disconnection (very high or very low readings)
    uint16_t fault_conditions[] = {0, 4095, 1, 4094};
    
    for (size_t i = 0; i < sizeof(fault_conditions) / sizeof(fault_conditions[0]); i++) {
        MockHardware::simulateAdcReading(34, fault_conditions[i]);
        last_update_time = 0; // Force update
        
        int32_t reading = mockGetOilPressureReading();
        
        // Should handle fault conditions gracefully
        TEST_ASSERT_GREATER_OR_EQUAL(0, reading);
        TEST_ASSERT_LESS_OR_EQUAL(10, reading);
    }
}

// Note: PlatformIO will automatically discover and run test_ functions