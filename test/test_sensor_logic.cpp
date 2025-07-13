#ifdef UNIT_TESTING

#include <unity.h>
#include "utilities/types.h"

// Mock sensor class for testing business logic
class MockSensor {
private:
    double raw_value;
    double min_threshold;
    double max_threshold;
    bool error_state;

public:
    MockSensor(double min = 0.0, double max = 100.0) 
        : raw_value(0.0), min_threshold(min), max_threshold(max), error_state(false) {}

    void set_raw_value(double value) { raw_value = value; }
    void set_error_state(bool error) { error_state = error; }

    Reading get_processed_reading() {
        if (error_state) {
            return std::string("ERROR");
        }
        
        if (raw_value < min_threshold) {
            return std::string("LOW");
        }
        
        if (raw_value > max_threshold) {
            return std::string("HIGH");
        }
        
        return raw_value;
    }

    bool is_in_warning_range(double low_warn, double high_warn) {
        return (raw_value >= low_warn && raw_value < min_threshold) ||
               (raw_value > max_threshold && raw_value <= high_warn);
    }

    bool is_critical(double critical_low, double critical_high) {
        return raw_value < critical_low || raw_value > critical_high;
    }
};

// Mock trigger class for testing trigger logic
class MockTrigger {
private:
    bool condition_met;
    std::string target_panel;
    int priority;
    bool should_restore_panel;

public:
    MockTrigger(std::string panel, int prio = 1, bool restore = false) 
        : condition_met(false), target_panel(panel), priority(prio), should_restore_panel(restore) {}

    void set_condition(bool met) { condition_met = met; }
    
    bool evaluate() { return condition_met; }
    std::string get_target_panel() { return target_panel; }
    int get_priority() { return priority; }
    bool should_restore() { return should_restore_panel; }
};

void setUp(void) {
    // Setup before each test
}

void tearDown(void) {
    // Cleanup after each test
}

void test_sensor_normal_reading(void) {
    MockSensor sensor(10.0, 90.0);
    sensor.set_raw_value(50.0);
    
    Reading result = sensor.get_processed_reading();
    TEST_ASSERT_TRUE(std::holds_alternative<double>(result));
    TEST_ASSERT_DOUBLE_WITHIN(0.1, 50.0, std::get<double>(result));
}

void test_sensor_low_threshold(void) {
    MockSensor sensor(10.0, 90.0);
    sensor.set_raw_value(5.0);
    
    Reading result = sensor.get_processed_reading();
    TEST_ASSERT_TRUE(std::holds_alternative<std::string>(result));
    TEST_ASSERT_EQUAL_STRING("LOW", std::get<std::string>(result).c_str());
}

void test_sensor_high_threshold(void) {
    MockSensor sensor(10.0, 90.0);
    sensor.set_raw_value(95.0);
    
    Reading result = sensor.get_processed_reading();
    TEST_ASSERT_TRUE(std::holds_alternative<std::string>(result));
    TEST_ASSERT_EQUAL_STRING("HIGH", std::get<std::string>(result).c_str());
}

void test_sensor_error_state(void) {
    MockSensor sensor;
    sensor.set_error_state(true);
    sensor.set_raw_value(50.0);
    
    Reading result = sensor.get_processed_reading();
    TEST_ASSERT_TRUE(std::holds_alternative<std::string>(result));
    TEST_ASSERT_EQUAL_STRING("ERROR", std::get<std::string>(result).c_str());
}

void test_sensor_warning_range_low(void) {
    MockSensor sensor(20.0, 80.0);
    sensor.set_raw_value(15.0);  // Between warning (10) and normal (20)
    
    TEST_ASSERT_TRUE(sensor.is_in_warning_range(10.0, 90.0));
    TEST_ASSERT_FALSE(sensor.is_critical(5.0, 95.0));
}

void test_sensor_warning_range_high(void) {
    MockSensor sensor(20.0, 80.0);
    sensor.set_raw_value(85.0);  // Between normal (80) and warning (90)
    
    TEST_ASSERT_TRUE(sensor.is_in_warning_range(10.0, 90.0));
    TEST_ASSERT_FALSE(sensor.is_critical(5.0, 95.0));
}

void test_sensor_critical_low(void) {
    MockSensor sensor(20.0, 80.0);
    sensor.set_raw_value(3.0);  // Below critical threshold
    
    TEST_ASSERT_TRUE(sensor.is_critical(5.0, 95.0));
    TEST_ASSERT_FALSE(sensor.is_in_warning_range(10.0, 90.0));
}

void test_sensor_critical_high(void) {
    MockSensor sensor(20.0, 80.0);
    sensor.set_raw_value(98.0);  // Above critical threshold
    
    TEST_ASSERT_TRUE(sensor.is_critical(5.0, 95.0));
    TEST_ASSERT_FALSE(sensor.is_in_warning_range(10.0, 90.0));
}

void test_trigger_basic_evaluation(void) {
    MockTrigger trigger("TestPanel", 1, false);
    
    TEST_ASSERT_FALSE(trigger.evaluate());
    
    trigger.set_condition(true);
    TEST_ASSERT_TRUE(trigger.evaluate());
}

void test_trigger_properties(void) {
    MockTrigger trigger("KeyPanel", 5, true);
    
    TEST_ASSERT_EQUAL_STRING("KeyPanel", trigger.get_target_panel().c_str());
    TEST_ASSERT_EQUAL(5, trigger.get_priority());
    TEST_ASSERT_TRUE(trigger.should_restore());
}

void test_trigger_priority_comparison(void) {
    MockTrigger low_priority("Panel1", 1, false);
    MockTrigger high_priority("Panel2", 10, false);
    
    // Higher number = higher priority
    TEST_ASSERT_TRUE(high_priority.get_priority() > low_priority.get_priority());
}

void test_oil_pressure_ranges(void) {
    // Typical oil pressure sensor ranges (0-100 PSI)
    MockSensor oil_pressure(10.0, 80.0);  // Normal: 10-80 PSI
    
    // Test normal pressure
    oil_pressure.set_raw_value(45.0);
    Reading result = oil_pressure.get_processed_reading();
    TEST_ASSERT_TRUE(std::holds_alternative<double>(result));
    
    // Test low pressure warning
    oil_pressure.set_raw_value(5.0);
    result = oil_pressure.get_processed_reading();
    TEST_ASSERT_EQUAL_STRING("LOW", std::get<std::string>(result).c_str());
    
    // Test high pressure warning
    oil_pressure.set_raw_value(85.0);
    result = oil_pressure.get_processed_reading();
    TEST_ASSERT_EQUAL_STRING("HIGH", std::get<std::string>(result).c_str());
}

void test_oil_temperature_ranges(void) {
    // Typical oil temperature sensor ranges (70-250°F)
    MockSensor oil_temp(180.0, 230.0);  // Normal: 180-230°F
    
    // Test normal temperature
    oil_temp.set_raw_value(200.0);
    Reading result = oil_temp.get_processed_reading();
    TEST_ASSERT_TRUE(std::holds_alternative<double>(result));
    
    // Test low temperature
    oil_temp.set_raw_value(150.0);
    result = oil_temp.get_processed_reading();
    TEST_ASSERT_EQUAL_STRING("LOW", std::get<std::string>(result).c_str());
    
    // Test overheating
    oil_temp.set_raw_value(250.0);
    result = oil_temp.get_processed_reading();
    TEST_ASSERT_EQUAL_STRING("HIGH", std::get<std::string>(result).c_str());
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_sensor_normal_reading);
    RUN_TEST(test_sensor_low_threshold);
    RUN_TEST(test_sensor_high_threshold);
    RUN_TEST(test_sensor_error_state);
    RUN_TEST(test_sensor_warning_range_low);
    RUN_TEST(test_sensor_warning_range_high);
    RUN_TEST(test_sensor_critical_low);
    RUN_TEST(test_sensor_critical_high);
    RUN_TEST(test_trigger_basic_evaluation);
    RUN_TEST(test_trigger_properties);
    RUN_TEST(test_trigger_priority_comparison);
    RUN_TEST(test_oil_pressure_ranges);
    RUN_TEST(test_oil_temperature_ranges);
    
    return UNITY_END();
}

#endif