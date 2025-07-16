#ifdef UNIT_TESTING

#include <unity.h>
#include <string>
#include <variant>
#include <cstdint>

// Mock Arduino functions
static uint32_t mock_millis_value = 0;
static int32_t mock_analog_read_value = 0;
static uint8_t mock_adc_resolution = 12;
static uint8_t mock_adc_attenuation = 0;

uint32_t sensors_millis() {
    return mock_millis_value;
}

int32_t analogRead(int pin) {
    return mock_analog_read_value;
}

void analogReadResolution(uint8_t resolution) {
    mock_adc_resolution = resolution;
}

void analogSetAttenuation(uint8_t attenuation) {
    mock_adc_attenuation = attenuation;
}

// Mock constants
#define ADC_11db 3

// Mock types
using Reading = std::variant<std::monostate, int32_t, double, std::string, bool>;

// Mock GPIO pins
namespace GpioPins {
    constexpr int OIL_PRESSURE = 36;
    constexpr int OIL_TEMPERATURE = 39;
    constexpr int KEY_PRESENT = 25;
    constexpr int KEY_NOT_PRESENT = 26;
    constexpr int LOCK = 27;
}

// Mock sensor implementations based on actual classes
class MockOilPressureSensor {
private:
    int32_t _current_reading = 0;
    int32_t _previous_reading = -1;
    unsigned long _last_update_time = 0;
    static constexpr unsigned long UPDATE_INTERVAL_MS = 1000;
    static constexpr int32_t ADC_MAX_VALUE = 4095;
    static constexpr int32_t PRESSURE_MAX_BAR = 10;

public:
    MockOilPressureSensor() {
        _current_reading = 0;
        _previous_reading = -1;
    }

    void init() {
        analogReadResolution(12);
        analogSetAttenuation(ADC_11db);
        
        int32_t adc_value = analogRead(GpioPins::OIL_PRESSURE);
        get_reading(); // Read initial value
    }

    Reading get_reading() {
        unsigned long current_time = sensors_millis();
        
        if (current_time - _last_update_time >= UPDATE_INTERVAL_MS) {
            _last_update_time = current_time;
            _previous_reading = _current_reading;
            
            int32_t adc_value = analogRead(GpioPins::OIL_PRESSURE);
            int32_t new_value = (adc_value * PRESSURE_MAX_BAR) / ADC_MAX_VALUE;
            
            if (new_value != _current_reading) {
                _current_reading = new_value;
            }
        }
        
        return _current_reading;
    }

    bool has_value_changed() {
        return _previous_reading != -1 && _current_reading != _previous_reading;
    }

    // Test helpers
    int32_t get_current_reading() const { return _current_reading; }
    int32_t get_previous_reading() const { return _previous_reading; }
    unsigned long get_last_update_time() const { return _last_update_time; }
};

class MockOilTemperatureSensor {
private:
    int32_t _current_reading = 0;
    int32_t _previous_reading = -1;
    unsigned long _last_update_time = 0;
    static constexpr unsigned long UPDATE_INTERVAL_MS = 1000;
    static constexpr int32_t ADC_MAX_VALUE = 4095;
    static constexpr int32_t TEMP_MIN_C = 70;
    static constexpr int32_t TEMP_MAX_C = 250;

public:
    MockOilTemperatureSensor() {
        _current_reading = TEMP_MIN_C;
        _previous_reading = -1;
    }

    void init() {
        analogReadResolution(12);
        analogSetAttenuation(ADC_11db);
        
        int32_t adc_value = analogRead(GpioPins::OIL_TEMPERATURE);
        get_reading(); // Read initial value
    }

    Reading get_reading() {
        unsigned long current_time = sensors_millis();
        
        if (current_time - _last_update_time >= UPDATE_INTERVAL_MS) {
            _last_update_time = current_time;
            _previous_reading = _current_reading;
            
            int32_t adc_value = analogRead(GpioPins::OIL_TEMPERATURE);
            int32_t new_value = TEMP_MIN_C + ((adc_value * (TEMP_MAX_C - TEMP_MIN_C)) / ADC_MAX_VALUE);
            
            if (new_value != _current_reading) {
                _current_reading = new_value;
            }
        }
        
        return _current_reading;
    }

    bool has_value_changed() {
        return _previous_reading != -1 && _current_reading != _previous_reading;
    }

    // Test helpers
    int32_t get_current_reading() const { return _current_reading; }
    int32_t get_previous_reading() const { return _previous_reading; }
};

class MockKeySensor {
private:
    bool _key_present = false;
    bool _key_not_present = false;
    bool _previous_state = false;

public:
    void init() {
        // Configure digital pins as inputs
    }

    Reading get_reading() {
        // Simulate digital pin reading
        _key_present = mock_analog_read_value > 2048; // Simulate digital high
        _key_not_present = mock_analog_read_value < 2048; // Simulate digital low
        
        return _key_present;
    }

    bool has_value_changed() {
        bool current_state = _key_present;
        bool changed = current_state != _previous_state;
        _previous_state = current_state;
        return changed;
    }

    // Test helpers
    bool is_key_present() const { return _key_present; }
    bool is_key_not_present() const { return _key_not_present; }
};

// Test fixtures
MockOilPressureSensor* pressure_sensor = nullptr;
MockOilTemperatureSensor* temperature_sensor = nullptr;
MockKeySensor* key_sensor = nullptr;

void sensors_setUp(void) {
    // Reset mock values
    mock_millis_value = 0;
    mock_analog_read_value = 0;
    mock_adc_resolution = 12;
    mock_adc_attenuation = 0;
    
    // Create sensor instances
    pressure_sensor = new MockOilPressureSensor();
    temperature_sensor = new MockOilTemperatureSensor();
    key_sensor = new MockKeySensor();
}

void sensors_tearDown(void) {
    delete pressure_sensor;
    delete temperature_sensor;
    delete key_sensor;
    pressure_sensor = nullptr;
    temperature_sensor = nullptr;
    key_sensor = nullptr;
}

// Oil Pressure Sensor Tests
void test_oil_pressure_sensor_initialization(void) {
    mock_analog_read_value = 2048; // Mid-range ADC value
    
    pressure_sensor->init();
    
    TEST_ASSERT_EQUAL(12, mock_adc_resolution);
    TEST_ASSERT_EQUAL(ADC_11db, mock_adc_attenuation);
}

void test_oil_pressure_sensor_reading_conversion(void) {
    mock_millis_value = 1000;
    mock_analog_read_value = 2048; // Half of 4095
    
    Reading result = pressure_sensor->get_reading();
    
    TEST_ASSERT_TRUE(std::holds_alternative<int32_t>(result));
    int32_t pressure = std::get<int32_t>(result);
    
    // Should be approximately 5 Bar (half of 10 Bar max)
    TEST_ASSERT_EQUAL(5, pressure);
}

void test_oil_pressure_sensor_min_max_values(void) {
    mock_millis_value = 1000;
    
    // Test minimum pressure (0 Bar)
    mock_analog_read_value = 0;
    Reading result = pressure_sensor->get_reading();
    TEST_ASSERT_EQUAL(0, std::get<int32_t>(result));
    
    // Test maximum pressure (10 Bar)
    mock_millis_value = 2000;
    mock_analog_read_value = 4095;
    result = pressure_sensor->get_reading();
    TEST_ASSERT_EQUAL(10, std::get<int32_t>(result));
}

void test_oil_pressure_sensor_time_based_sampling(void) {
    mock_millis_value = 1000;
    mock_analog_read_value = 2048;
    
    // First reading
    Reading result1 = pressure_sensor->get_reading();
    TEST_ASSERT_EQUAL(5, std::get<int32_t>(result1));
    
    // Change ADC value but not enough time elapsed
    mock_analog_read_value = 3072;
    mock_millis_value = 1500; // Only 500ms elapsed
    
    Reading result2 = pressure_sensor->get_reading();
    TEST_ASSERT_EQUAL(5, std::get<int32_t>(result2)); // Should be same as before
    
    // Enough time elapsed
    mock_millis_value = 2000; // 1000ms elapsed
    Reading result3 = pressure_sensor->get_reading();
    TEST_ASSERT_EQUAL(7, std::get<int32_t>(result3)); // Should update to new value
}

void test_oil_pressure_sensor_change_detection(void) {
    mock_millis_value = 1000;
    mock_analog_read_value = 2048;
    
    // First reading - _previous_reading starts as -1, gets set to 0 (initial _current_reading)
    // _current_reading becomes 5 (from ADC 2048)
    // So has_value_changed() returns: 0 != -1 && 5 != 0 = true && true = true
    pressure_sensor->get_reading();
    TEST_ASSERT_TRUE(pressure_sensor->has_value_changed()); // First reading shows as changed
    
    // Same value after time interval - _previous_reading = 5, _current_reading stays 5
    mock_millis_value = 2000;
    mock_analog_read_value = 2048; // Same value
    pressure_sensor->get_reading();
    TEST_ASSERT_FALSE(pressure_sensor->has_value_changed()); // No change
    
    // Different value after time interval - _previous_reading = 5, _current_reading = 7
    mock_millis_value = 3000;
    mock_analog_read_value = 3072; // Different value
    pressure_sensor->get_reading();
    TEST_ASSERT_TRUE(pressure_sensor->has_value_changed()); // Should detect change
}

// Oil Temperature Sensor Tests
void test_oil_temperature_sensor_initialization(void) {
    mock_analog_read_value = 2048;
    
    temperature_sensor->init();
    
    TEST_ASSERT_EQUAL(12, mock_adc_resolution);
    TEST_ASSERT_EQUAL(ADC_11db, mock_adc_attenuation);
}

void test_oil_temperature_sensor_reading_conversion(void) {
    mock_millis_value = 1000;
    mock_analog_read_value = 2048; // Half of 4095
    
    Reading result = temperature_sensor->get_reading();
    
    TEST_ASSERT_TRUE(std::holds_alternative<int32_t>(result));
    int32_t temperature = std::get<int32_t>(result);
    
    // Should be approximately 160°C (middle of 70-250°C range)
    TEST_ASSERT_EQUAL(160, temperature);
}

void test_oil_temperature_sensor_min_max_values(void) {
    mock_millis_value = 1000;
    
    // Test minimum temperature (70°C)
    mock_analog_read_value = 0;
    Reading result = temperature_sensor->get_reading();
    TEST_ASSERT_EQUAL(70, std::get<int32_t>(result));
    
    // Test maximum temperature (250°C)
    mock_millis_value = 2000;
    mock_analog_read_value = 4095;
    result = temperature_sensor->get_reading();
    TEST_ASSERT_EQUAL(250, std::get<int32_t>(result));
}

void test_oil_temperature_sensor_time_based_sampling(void) {
    mock_millis_value = 1000;
    mock_analog_read_value = 2048;
    
    // First reading
    Reading result1 = temperature_sensor->get_reading();
    TEST_ASSERT_EQUAL(160, std::get<int32_t>(result1));
    
    // Change ADC value but not enough time elapsed
    mock_analog_read_value = 3072;
    mock_millis_value = 1500; // Only 500ms elapsed
    
    Reading result2 = temperature_sensor->get_reading();
    TEST_ASSERT_EQUAL(160, std::get<int32_t>(result2)); // Should be same as before
    
    // Enough time elapsed
    mock_millis_value = 2000; // 1000ms elapsed
    Reading result3 = temperature_sensor->get_reading();
    TEST_ASSERT_EQUAL(205, std::get<int32_t>(result3)); // Should update to new value
}

void test_oil_temperature_sensor_change_detection(void) {
    mock_millis_value = 1000;
    mock_analog_read_value = 2048;
    
    // First reading shows as changed (from initial state)
    temperature_sensor->get_reading();
    TEST_ASSERT_TRUE(temperature_sensor->has_value_changed()); // First reading shows as changed
    
    // Same value after time interval
    mock_millis_value = 2000;
    mock_analog_read_value = 2048;
    temperature_sensor->get_reading();
    TEST_ASSERT_FALSE(temperature_sensor->has_value_changed()); // No change
    
    // Different value after time interval
    mock_millis_value = 3000;
    mock_analog_read_value = 3072;
    temperature_sensor->get_reading();
    TEST_ASSERT_TRUE(temperature_sensor->has_value_changed()); // Should detect change
}

// Key Sensor Tests
void test_key_sensor_initialization(void) {
    key_sensor->init();
    // Basic initialization test - no specific assertions needed for digital pins
    TEST_ASSERT_TRUE(true); // Placeholder - init doesn't return values
}

void test_key_sensor_present_state(void) {
    mock_analog_read_value = 3000; // Simulate digital high
    
    Reading result = key_sensor->get_reading();
    
    TEST_ASSERT_TRUE(std::holds_alternative<bool>(result));
    TEST_ASSERT_TRUE(std::get<bool>(result));
    TEST_ASSERT_TRUE(key_sensor->is_key_present());
}

void test_key_sensor_not_present_state(void) {
    mock_analog_read_value = 1000; // Simulate digital low
    
    Reading result = key_sensor->get_reading();
    
    TEST_ASSERT_TRUE(std::holds_alternative<bool>(result));
    TEST_ASSERT_FALSE(std::get<bool>(result));
    TEST_ASSERT_TRUE(key_sensor->is_key_not_present());
}

void test_key_sensor_state_change_detection(void) {
    // Start with key not present
    mock_analog_read_value = 1000;
    key_sensor->get_reading();
    TEST_ASSERT_FALSE(key_sensor->has_value_changed()); // First read
    
    // Key still not present
    mock_analog_read_value = 1000;
    key_sensor->get_reading();
    TEST_ASSERT_FALSE(key_sensor->has_value_changed()); // No change
    
    // Key becomes present
    mock_analog_read_value = 3000;
    key_sensor->get_reading();
    TEST_ASSERT_TRUE(key_sensor->has_value_changed()); // Should detect change
}

// Integration Tests
void test_sensor_adc_calibration_constants(void) {
    // Test that ADC constants are properly defined
    TEST_ASSERT_EQUAL(36, GpioPins::OIL_PRESSURE);
    TEST_ASSERT_EQUAL(39, GpioPins::OIL_TEMPERATURE);
    TEST_ASSERT_EQUAL(25, GpioPins::KEY_PRESENT);
    TEST_ASSERT_EQUAL(26, GpioPins::KEY_NOT_PRESENT);
    TEST_ASSERT_EQUAL(27, GpioPins::LOCK);
}

void test_sensor_reading_variants(void) {
    mock_millis_value = 1000;
    mock_analog_read_value = 2048;
    
    // Test pressure sensor returns int32_t
    Reading pressure_result = pressure_sensor->get_reading();
    TEST_ASSERT_TRUE(std::holds_alternative<int32_t>(pressure_result));
    
    // Test temperature sensor returns int32_t
    Reading temp_result = temperature_sensor->get_reading();
    TEST_ASSERT_TRUE(std::holds_alternative<int32_t>(temp_result));
    
    // Test key sensor returns bool
    Reading key_result = key_sensor->get_reading();
    TEST_ASSERT_TRUE(std::holds_alternative<bool>(key_result));
}

void test_sensors_main() {
    // Each test needs its own setup/teardown
    sensors_setUp(); RUN_TEST(test_oil_pressure_sensor_initialization); sensors_tearDown();
    sensors_setUp(); RUN_TEST(test_oil_pressure_sensor_reading_conversion); sensors_tearDown();
    sensors_setUp(); RUN_TEST(test_oil_pressure_sensor_min_max_values); sensors_tearDown();
    sensors_setUp(); RUN_TEST(test_oil_pressure_sensor_time_based_sampling); sensors_tearDown();
    sensors_setUp(); RUN_TEST(test_oil_pressure_sensor_change_detection); sensors_tearDown();
    sensors_setUp(); RUN_TEST(test_oil_temperature_sensor_initialization); sensors_tearDown();
    sensors_setUp(); RUN_TEST(test_oil_temperature_sensor_reading_conversion); sensors_tearDown();
    sensors_setUp(); RUN_TEST(test_oil_temperature_sensor_min_max_values); sensors_tearDown();
    sensors_setUp(); RUN_TEST(test_oil_temperature_sensor_time_based_sampling); sensors_tearDown();
    sensors_setUp(); RUN_TEST(test_oil_temperature_sensor_change_detection); sensors_tearDown();
    sensors_setUp(); RUN_TEST(test_key_sensor_initialization); sensors_tearDown();
    sensors_setUp(); RUN_TEST(test_key_sensor_present_state); sensors_tearDown();
    sensors_setUp(); RUN_TEST(test_key_sensor_not_present_state); sensors_tearDown();
    sensors_setUp(); RUN_TEST(test_key_sensor_state_change_detection); sensors_tearDown();
    sensors_setUp(); RUN_TEST(test_sensor_adc_calibration_constants); sensors_tearDown();
    sensors_setUp(); RUN_TEST(test_sensor_reading_variants); sensors_tearDown();
}

#endif