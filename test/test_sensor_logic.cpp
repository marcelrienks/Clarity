#ifdef UNIT_TESTING

#include <unity.h>
#include <string>
#include <variant>

// Mock types for testing
using Reading = std::variant<std::monostate, int32_t, double, std::string, bool>;

// Mock sensor class for testing business logic
class MockSensor {
private:
    double rawValue;
    double minThreshold;
    double maxThreshold;
    bool errorState;

public:
    MockSensor(double min = 0.0, double max = 100.0) 
        : rawValue(0.0), minThreshold(min), maxThreshold(max), errorState(false) {}

    void setRawValue(double value) { rawValue = value; }
    void setErrorState(bool error) { errorState = error; }

    Reading getProcessedReading() {
        if (errorState) {
            return std::string("ERROR");
        }
        
        if (rawValue < minThreshold) {
            return std::string("LOW");
        }
        
        if (rawValue > maxThreshold) {
            return std::string("HIGH");
        }
        
        return rawValue;
    }

    bool isInWarningRange(double lowWarn, double highWarn) {
        return (rawValue >= lowWarn && rawValue < minThreshold) ||
               (rawValue > maxThreshold && rawValue <= highWarn);
    }

    bool isCritical(double criticalLow, double criticalHigh) {
        return rawValue < criticalLow || rawValue > criticalHigh;
    }
};

// Mock trigger class for testing trigger logic
class MockTrigger {
private:
    bool conditionMet;
    std::string targetPanel;
    int priority;
    bool shouldRestorePanel;

public:
    MockTrigger(std::string panel, int prio = 1, bool restore = false) 
        : conditionMet(false), targetPanel(panel), priority(prio), shouldRestorePanel(restore) {}

    void setCondition(bool met) { conditionMet = met; }
    
    bool evaluate() { return conditionMet; }
    std::string getTargetPanel() { return targetPanel; }
    int getPriority() { return priority; }
    bool shouldRestore() { return shouldRestorePanel; }
};


void test_sensor_normal_reading(void) {
    MockSensor sensor(10.0, 90.0);
    sensor.setRawValue(50.0);
    
    Reading result = sensor.getProcessedReading();
    TEST_ASSERT_TRUE(std::holds_alternative<double>(result));
    TEST_ASSERT_DOUBLE_WITHIN(0.1, 50.0, std::get<double>(result));
}

void test_sensor_low_threshold(void) {
    MockSensor sensor(10.0, 90.0);
    sensor.setRawValue(5.0);
    
    Reading result = sensor.getProcessedReading();
    TEST_ASSERT_TRUE(std::holds_alternative<std::string>(result));
    TEST_ASSERT_EQUAL_STRING("LOW", std::get<std::string>(result).c_str());
}

void test_sensor_high_threshold(void) {
    MockSensor sensor(10.0, 90.0);
    sensor.setRawValue(95.0);
    
    Reading result = sensor.getProcessedReading();
    TEST_ASSERT_TRUE(std::holds_alternative<std::string>(result));
    TEST_ASSERT_EQUAL_STRING("HIGH", std::get<std::string>(result).c_str());
}

void test_sensor_error_state(void) {
    MockSensor sensor;
    sensor.setErrorState(true);
    sensor.setRawValue(50.0);
    
    Reading result = sensor.getProcessedReading();
    TEST_ASSERT_TRUE(std::holds_alternative<std::string>(result));
    TEST_ASSERT_EQUAL_STRING("ERROR", std::get<std::string>(result).c_str());
}

void test_sensor_warning_range_low(void) {
    MockSensor sensor(20.0, 80.0);
    sensor.setRawValue(15.0);  // Between warning (10) and normal (20)
    
    TEST_ASSERT_TRUE(sensor.isInWarningRange(10.0, 90.0));
    TEST_ASSERT_FALSE(sensor.isCritical(5.0, 95.0));
}

void test_sensor_warning_range_high(void) {
    MockSensor sensor(20.0, 80.0);
    sensor.setRawValue(85.0);  // Between normal (80) and warning (90)
    
    TEST_ASSERT_TRUE(sensor.isInWarningRange(10.0, 90.0));
    TEST_ASSERT_FALSE(sensor.isCritical(5.0, 95.0));
}

void test_sensor_critical_low(void) {
    MockSensor sensor(20.0, 80.0);
    sensor.setRawValue(3.0);  // Below critical threshold
    
    TEST_ASSERT_TRUE(sensor.is_critical(5.0, 95.0));
    TEST_ASSERT_FALSE(sensor.isInWarningRange(10.0, 90.0));
}

void test_sensor_critical_high(void) {
    MockSensor sensor(20.0, 80.0);
    sensor.setRawValue(98.0);  // Above critical threshold
    
    TEST_ASSERT_TRUE(sensor.is_critical(5.0, 95.0));
    TEST_ASSERT_FALSE(sensor.isInWarningRange(10.0, 90.0));
}

void test_trigger_basic_evaluation(void) {
    MockTrigger trigger("TestPanel", 1, false);
    
    TEST_ASSERT_FALSE(trigger.evaluate());
    
    trigger.setCondition(true);
    TEST_ASSERT_TRUE(trigger.evaluate());
}

void test_trigger_properties(void) {
    MockTrigger trigger("KeyPanel", 5, true);
    
    TEST_ASSERT_EQUAL_STRING("KeyPanel", trigger.getTargetPanel().c_str());
    TEST_ASSERT_EQUAL(5, trigger.getPriority());
    TEST_ASSERT_TRUE(trigger.shouldRestore());
}

void test_trigger_priority_comparison(void) {
    MockTrigger lowPriority("Panel1", 1, false);
    MockTrigger highPriority("Panel2", 10, false);
    
    // Higher number = higher priority
    TEST_ASSERT_TRUE(highPriority.getPriority() > lowPriority.getPriority());
}

void test_oil_pressure_ranges(void) {
    // Typical oil pressure sensor ranges (0-100 PSI)
    MockSensor oilPressure(10.0, 80.0);  // Normal: 10-80 PSI
    
    // Test normal pressure
    oilPressure.setRawValue(45.0);
    Reading result = oilPressure.getProcessedReading();
    TEST_ASSERT_TRUE(std::holds_alternative<double>(result));
    
    // Test low pressure warning
    oilPressure.setRawValue(5.0);
    result = oilPressure.getProcessedReading();
    TEST_ASSERT_EQUAL_STRING("LOW", std::get<std::string>(result).c_str());
    
    // Test high pressure warning
    oilPressure.setRawValue(85.0);
    result = oilPressure.getProcessedReading();
    TEST_ASSERT_EQUAL_STRING("HIGH", std::get<std::string>(result).c_str());
}

void test_oil_temperature_ranges(void) {
    // Typical oil temperature sensor ranges (70-250°F)
    MockSensor oilTemp(180.0, 230.0);  // Normal: 180-230°F
    
    // Test normal temperature
    oilTemp.setRawValue(200.0);
    Reading result = oilTemp.getProcessedReading();
    TEST_ASSERT_TRUE(std::holds_alternative<double>(result));
    
    // Test low temperature
    oilTemp.setRawValue(150.0);
    result = oilTemp.getProcessedReading();
    TEST_ASSERT_EQUAL_STRING("LOW", std::get<std::string>(result).c_str());
    
    // Test overheating
    oilTemp.setRawValue(250.0);
    result = oilTemp.getProcessedReading();
    TEST_ASSERT_EQUAL_STRING("HIGH", std::get<std::string>(result).c_str());
}

void test_sensor_logic_main() {
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
}

#endif