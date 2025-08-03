#include <unity.h>
#include <cstdint>

#ifdef UNIT_TESTING

// Test sensor value change detection logic
class SensorLogicTestSensor {
private:
    int32_t currentReading_ = 0;
    int32_t previousReading_ = -1;

public:
    void setReading(int32_t value) {
        previousReading_ = currentReading_;
        currentReading_ = value;
    }
    
    int32_t getReading() const {
        return currentReading_;
    }
    
    bool hasValueChanged() {
        return currentReading_ != previousReading_;
    }
};

// Test ADC to pressure conversion logic
double convertAdcToPressure(uint16_t adcValue) {
    // Simple linear conversion for testing
    // Real implementation would use proper calibration
    double voltage = (adcValue / 4095.0) * 3.3;
    double pressure = voltage * 30.0; // Example scaling
    return pressure;
}

// Test key state logic
enum class KeyState { INACTIVE = 0, PRESENT = 1, NOT_PRESENT = 2 };

KeyState determineKeyState(bool keyPresent, bool keyNotPresent) {
    if (keyPresent && !keyNotPresent) {
        return KeyState::PRESENT;
    } else if (!keyPresent && keyNotPresent) {
        return KeyState::NOT_PRESENT;
    } else {
        return KeyState::INACTIVE;
    }
}

#endif

void test_sensor_logic_value_change_detection() {
    SensorLogicTestSensor sensor;
    
    // Set initial value
    sensor.setReading(100);
    bool hasChanged1 = sensor.hasValueChanged();
    TEST_ASSERT_TRUE(hasChanged1); // First reading should show change
    
    // Same value should not show change
    sensor.setReading(100);
    bool hasChanged2 = sensor.hasValueChanged();
    TEST_ASSERT_FALSE(hasChanged2);
    
    // Different value should show change
    sensor.setReading(200);
    bool hasChanged3 = sensor.hasValueChanged();
    TEST_ASSERT_TRUE(hasChanged3);
}

void test_sensor_logic_adc_to_pressure_conversion() {
    // Test known conversion points
    double pressure1 = convertAdcToPressure(0);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, pressure1);
    
    double pressure2 = convertAdcToPressure(2048); // Mid-range
    TEST_ASSERT_GREATER_THAN(0.0, pressure2);
    TEST_ASSERT_LESS_THAN(100.0, pressure2);
    
    double pressure3 = convertAdcToPressure(4095); // Max
    TEST_ASSERT_GREATER_THAN(pressure2, pressure3);
}

void test_sensor_logic_key_state_logic() {
    // Test key present
    KeyState state1 = determineKeyState(true, false);
    TEST_ASSERT_EQUAL(KeyState::PRESENT, state1);
    
    // Test key not present
    KeyState state2 = determineKeyState(false, true);
    TEST_ASSERT_EQUAL(KeyState::NOT_PRESENT, state2);
    
    // Test inactive
    KeyState state3 = determineKeyState(false, false);
    TEST_ASSERT_EQUAL(KeyState::INACTIVE, state3);
    
    // Test invalid (both true - should default to inactive)
    KeyState state4 = determineKeyState(true, true);
    TEST_ASSERT_EQUAL(KeyState::INACTIVE, state4);
}

void runSensorLogicTests() {
    RUN_TEST(test_sensor_logic_value_change_detection);
    RUN_TEST(test_sensor_logic_adc_to_pressure_conversion);
    RUN_TEST(test_sensor_logic_key_state_logic);
}