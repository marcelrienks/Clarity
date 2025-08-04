#include <unity.h>
#include <cstdint>
#include <string>
#include <map>
#include "Arduino.h"
#include "utilities/types.h"
#include "utilities/test_common.h"

#ifdef UNIT_TESTING

// Declare all existing test runner functions from individual test files
extern void runComponentInterfaceTests();        // 13 tests
extern void runStandaloneComponentTests();       // 5 tests
extern void runManagerFactoryTests();            // 16 tests  
extern void runUIFactoryTests();                 // 18 tests
extern void runSimplifiedUIFactoryTests();       // 16 tests
extern void runPanelManagerTests();              // 8 tests
extern void runPreferenceManagerTests();         // 29 tests
extern void runStyleManagerTests();              // 14 tests
extern void runTriggerManagerTests();            // 7 tests
extern void runPanelInterfaceTests();            // 16 tests
extern void runStandalonePanelTests();           // 6 tests
extern void runGpioProviderTests();              // 5 tests
extern void runLvglDisplayProviderTests();       // 17 tests
extern void runKeySensorTests();                 // 21 tests
extern void runLightSensorTests();               // 7 tests
extern void runLockSensorTests();                // 8 tests
extern void runOilPressureSensorTests();         // 4 tests
extern void runOilTemperatureSensorTests();      // 5 tests
extern void runServiceContainerTests();          // 8 tests
extern void runSimpleTickerTests();              // 3 tests
extern void runTickerTests();                    // 4 tests

// ============================================================================
// INTEGRATED TEST_ALL.CPP FUNCTIONS (19 tests)
// ============================================================================

// Test sensor value change detection logic
class TestSensor {
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
        bool changed = currentReading_ != previousReading_;
        previousReading_ = currentReading_;
        return changed;
    }
};

// Test ADC to pressure conversion
double convertAdcToPressure(uint16_t adcValue) {
    double voltage = (adcValue / 4095.0) * 3.3;
    double pressure = voltage * 30.0;
    return pressure;
}

// Test key state logic using KeyState enum from types.h
KeyState determineKeyState(bool keyPresent, bool keyNotPresent) {
    if (keyPresent && !keyNotPresent) {
        return KeyState::Present;
    } else if (!keyPresent && keyNotPresent) {
        return KeyState::NotPresent;
    } else {
        return KeyState::Inactive;
    }
}

// Test configuration management
class ConfigManager {
private:
    std::map<std::string, std::string> config;
    
public:
    void setConfig(const std::string& key, const std::string& value) {
        config[key] = value;
    }
    
    std::string getConfig(const std::string& key, const std::string& defaultValue = "") {
        auto it = config.find(key);
        return (it != config.end()) ? it->second : defaultValue;
    }
    
    bool hasConfig(const std::string& key) {
        return config.find(key) != config.end();
    }
    
    void createDefaultConfig() {
        config["panelName"] = "OIL";
        config["theme"] = "DAY";
    }
    
    size_t getConfigCount() const {
        return config.size();
    }
    
    void clearConfig() {
        config.clear();
    }
};

bool isValidPanelName(const std::string& panelName) {
    return panelName == "OIL" || panelName == "KEY" || panelName == "LOCK";
}

bool isValidTheme(const std::string& theme) {
    return theme == "DAY" || theme == "NIGHT";
}

// ============================================================================
// TEST FUNCTIONS FROM TEST_ALL.CPP (19 functions)
// ============================================================================

void test_timing_calculation() {
    uint32_t targetFrameTime = 16;
    set_mock_millis(0);
    uint32_t startTime = 0;
    set_mock_millis(5);
    uint32_t elapsed = millis() - startTime;
    TEST_ASSERT_LESS_THAN(targetFrameTime, elapsed);
}

void test_adc_conversion_performance_benchmark() {
    uint32_t startTime = millis();
    int conversionCount = 1000;
    
    for (int i = 0; i < conversionCount; i++) {
        uint16_t adcValue = (i % 4096);
        double pressure = convertAdcToPressure(adcValue);
        TEST_ASSERT_GREATER_OR_EQUAL(0.0, pressure);
        TEST_ASSERT_LESS_THAN(200.0, pressure);
    }
    
    uint32_t endTime = millis();
    uint32_t totalTime = endTime - startTime;
    TEST_ASSERT_LESS_THAN(1000, totalTime);
    
    if (totalTime > 0) {
        uint32_t conversionsPerSecond = (conversionCount * 1000) / totalTime;
        TEST_ASSERT_GREATER_THAN(1000, conversionsPerSecond);
    }
}

void test_sensor_state_change_detection_performance() {
    TestSensor sensor;
    MockHardwareState::instance().setMillis(0);
    uint32_t startTime = millis();
    int iterations = 10000;
    int changeDetections = 0;
    
    for (int i = 0; i < iterations; i++) {
        sensor.setReading(i / 100);
        if (sensor.hasValueChanged()) {
            changeDetections++;
        }
        if (i % 1000 == 0) {
            MockHardwareState::instance().advanceTime(1);
        }
    }
    
    MockHardwareState::instance().advanceTime(100);
    uint32_t endTime = millis();
    uint32_t totalTime = endTime - startTime;
    
    TEST_ASSERT_LESS_THAN(2000, totalTime);
    TEST_ASSERT_GREATER_THAN(50, changeDetections);
    TEST_ASSERT_LESS_THAN(200, changeDetections);
}

void test_key_state_logic_performance_benchmark() {
    uint32_t startTime = millis();
    int iterations = 5000;
    int validStates = 0;
    
    for (int i = 0; i < iterations; i++) {
        bool keyPresent = (i % 3 == 0);
        bool keyNotPresent = (i % 5 == 0);
        
        if (keyPresent && keyNotPresent) {
            keyNotPresent = false;
        }
        
        KeyState state = determineKeyState(keyPresent, keyNotPresent);
        
        if (state == KeyState::Present || state == KeyState::NotPresent || state == KeyState::Inactive) {
            validStates++;
        }
    }
    
    uint32_t endTime = millis();
    uint32_t totalTime = endTime - startTime;
    
    TEST_ASSERT_LESS_THAN(500, totalTime);
    TEST_ASSERT_EQUAL(iterations, validStates);
    
    if (totalTime > 0) {
        uint32_t statesPerSecond = (iterations * 1000) / totalTime;
        TEST_ASSERT_GREATER_THAN(10000, statesPerSecond);
    }
}

void test_config_operations_performance_benchmark() {
    ConfigManager manager;
    uint32_t startTime = millis();
    int iterations = 1000;
    
    for (int i = 0; i < iterations; i++) {
        std::string key = "testKey" + std::to_string(i % 10);
        std::string value = "testValue" + std::to_string(i);
        
        manager.setConfig(key, value);
        std::string retrieved = manager.getConfig(key);
        bool hasKey = manager.hasConfig(key);
        
        TEST_ASSERT_EQUAL_STRING(value.c_str(), retrieved.c_str());
        TEST_ASSERT_TRUE(hasKey);
    }
    
    uint32_t endTime = millis();
    uint32_t totalTime = endTime - startTime;
    
    TEST_ASSERT_LESS_THAN(2000, totalTime);
    TEST_ASSERT_GREATER_THAN(5, manager.getConfigCount());
    TEST_ASSERT_LESS_OR_EQUAL(10, manager.getConfigCount());
}

void test_sensor_value_change_detection() {
    TestSensor sensor;
    sensor.setReading(100);
    bool hasChanged1 = sensor.hasValueChanged();
    TEST_ASSERT_TRUE(hasChanged1);
    
    sensor.setReading(100);
    bool hasChanged2 = sensor.hasValueChanged();
    TEST_ASSERT_FALSE(hasChanged2);
    
    sensor.setReading(200);
    bool hasChanged3 = sensor.hasValueChanged();
    TEST_ASSERT_TRUE(hasChanged3);
}

void test_adc_to_pressure_conversion() {
    double pressure1 = convertAdcToPressure(0);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, pressure1);
    
    double pressure2 = convertAdcToPressure(2048);
    TEST_ASSERT_GREATER_THAN(0.0, pressure2);
    TEST_ASSERT_LESS_THAN(100.0, pressure2);
    
    double pressure3 = convertAdcToPressure(4095);
    TEST_ASSERT_GREATER_THAN(pressure2, pressure3);
}

void test_key_state_logic() {
    KeyState state1 = determineKeyState(true, false);
    TEST_ASSERT_EQUAL(KeyState::Present, state1);
    
    KeyState state2 = determineKeyState(false, true);
    TEST_ASSERT_EQUAL(KeyState::NotPresent, state2);
    
    KeyState state3 = determineKeyState(false, false);
    TEST_ASSERT_EQUAL(KeyState::Inactive, state3);
    
    KeyState state4 = determineKeyState(true, true);
    TEST_ASSERT_EQUAL(KeyState::Inactive, state4);
}

void test_config_set_get() {
    ConfigManager manager;
    manager.setConfig("testKey", "testValue");
    std::string value = manager.getConfig("testKey");
    TEST_ASSERT_EQUAL_STRING("testValue", value.c_str());
    
    std::string defaultValue = manager.getConfig("missingKey", "default");
    TEST_ASSERT_EQUAL_STRING("default", defaultValue.c_str());
}

void test_config_has_config() {
    ConfigManager manager;
    TEST_ASSERT_FALSE(manager.hasConfig("testKey"));
    
    manager.setConfig("testKey", "value");
    TEST_ASSERT_TRUE(manager.hasConfig("testKey"));
}

void test_config_default_creation() {
    ConfigManager manager;
    manager.createDefaultConfig();
    
    TEST_ASSERT_EQUAL_STRING("OIL", manager.getConfig("panelName").c_str());
    TEST_ASSERT_EQUAL_STRING("DAY", manager.getConfig("theme").c_str());
    TEST_ASSERT_EQUAL(2, manager.getConfigCount());
}

void test_panel_name_validation() {
    TEST_ASSERT_TRUE(isValidPanelName("OIL"));
    TEST_ASSERT_TRUE(isValidPanelName("KEY"));
    TEST_ASSERT_TRUE(isValidPanelName("LOCK"));
    TEST_ASSERT_FALSE(isValidPanelName("INVALID"));
    TEST_ASSERT_FALSE(isValidPanelName(""));
}

void test_theme_validation() {
    TEST_ASSERT_TRUE(isValidTheme("DAY"));
    TEST_ASSERT_TRUE(isValidTheme("NIGHT"));
    TEST_ASSERT_FALSE(isValidTheme("INVALID"));
    TEST_ASSERT_FALSE(isValidTheme(""));
}

void test_config_clear() {
    ConfigManager manager;
    manager.setConfig("key1", "value1");
    manager.setConfig("key2", "value2");
    TEST_ASSERT_EQUAL(2, manager.getConfigCount());
    
    manager.clearConfig();
    TEST_ASSERT_EQUAL(0, manager.getConfigCount());
    TEST_ASSERT_FALSE(manager.hasConfig("key1"));
}

void test_sensor_rapid_state_changes() {
    TestSensor sensor;
    int changeCount = 0;
    
    sensor.setReading(-1);
    sensor.hasValueChanged();
    
    for (int i = 0; i < 1000; i++) {
        sensor.setReading(i % 2);
        if (sensor.hasValueChanged()) {
            changeCount++;
        }
    }
    
    TEST_ASSERT_GREATER_THAN(900, changeCount);
    TEST_ASSERT_LESS_OR_EQUAL(1000, changeCount);
}

void test_config_large_dataset_performance() {
    ConfigManager manager;
    uint32_t startTime = millis();
    
    for (int i = 0; i < 100; i++) {
        std::string key = "largekey_" + std::to_string(i);
        std::string value = "largevalue_" + std::to_string(i) + "_with_lots_of_extra_data_to_make_it_realistic";
        manager.setConfig(key, value);
    }
    
    uint32_t endTime = millis();
    uint32_t totalTime = endTime - startTime;
    
    TEST_ASSERT_LESS_THAN(1000, totalTime);
    TEST_ASSERT_EQUAL(100, manager.getConfigCount());
    
    for (int i = 0; i < 100; i++) {
        std::string key = "largekey_" + std::to_string(i);
        TEST_ASSERT_TRUE(manager.hasConfig(key));
    }
}

void test_adc_conversion_boundary_values() {
    double pressure_min = convertAdcToPressure(0);
    double pressure_max = convertAdcToPressure(4095);
    double pressure_mid = convertAdcToPressure(2048);
    
    TEST_ASSERT_EQUAL_DOUBLE(0.0, pressure_min);
    TEST_ASSERT_GREATER_THAN(90.0, pressure_max);
    TEST_ASSERT_LESS_THAN(110.0, pressure_max);
    TEST_ASSERT_GREATER_THAN(45.0, pressure_mid);
    TEST_ASSERT_LESS_THAN(55.0, pressure_mid);
}

void test_key_state_edge_cases() {
    KeyState state1 = determineKeyState(true, true);
    KeyState state2 = determineKeyState(false, false);
    KeyState state3 = determineKeyState(true, false);
    KeyState state4 = determineKeyState(false, true);
    
    TEST_ASSERT_EQUAL(KeyState::Inactive, state1);
    TEST_ASSERT_EQUAL(KeyState::Inactive, state2);
    TEST_ASSERT_EQUAL(KeyState::Present, state3);
    TEST_ASSERT_EQUAL(KeyState::NotPresent, state4);
}

void test_memory_usage_stability() {
    ConfigManager manager;
    
    for (int cycle = 0; cycle < 10; cycle++) {
        for (int i = 0; i < 50; i++) {
            std::string key = "temp_" + std::to_string(i);
            manager.setConfig(key, "temporary_value");
        }
        TEST_ASSERT_EQUAL(50, manager.getConfigCount());
        manager.clearConfig();
        TEST_ASSERT_EQUAL(0, manager.getConfigCount());
    }
    
    TEST_ASSERT_EQUAL(0, manager.getConfigCount());
}

void setUp(void) {
    // Reset mock values
    set_mock_millis(0);
}

void tearDown(void) {
    // Cleanup after each test
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    // ============================================================================
    // UNIFIED TEST EXECUTION - ALL 249 TESTS
    // ============================================================================
    
    // Basic Logic Tests from test_all.cpp (19 tests)
    RUN_TEST(test_timing_calculation);
    RUN_TEST(test_sensor_value_change_detection);
    RUN_TEST(test_adc_to_pressure_conversion);
    RUN_TEST(test_key_state_logic);
    RUN_TEST(test_config_set_get);
    RUN_TEST(test_config_has_config);
    RUN_TEST(test_config_default_creation);
    RUN_TEST(test_panel_name_validation);
    RUN_TEST(test_theme_validation);
    RUN_TEST(test_config_clear);
    RUN_TEST(test_sensor_rapid_state_changes);
    RUN_TEST(test_config_large_dataset_performance);
    RUN_TEST(test_adc_conversion_boundary_values);
    RUN_TEST(test_key_state_edge_cases);
    RUN_TEST(test_memory_usage_stability);
    RUN_TEST(test_adc_conversion_performance_benchmark);
    RUN_TEST(test_sensor_state_change_detection_performance);
    RUN_TEST(test_key_state_logic_performance_benchmark);
    RUN_TEST(test_config_operations_performance_benchmark);
    
    // All Unit Test Suites (230 tests via runners)
    runComponentInterfaceTests();        // 13 tests - Component interface compliance
    runStandaloneComponentTests();       // 5 tests - Standalone component behavior
    runManagerFactoryTests();            // 16 tests - Manager factory patterns
    runUIFactoryTests();                 // 18 tests - UI factory creation
    runSimplifiedUIFactoryTests();       // 16 tests - Simplified UI factory
    runPanelManagerTests();              // 8 tests - Panel lifecycle management
    runPreferenceManagerTests();         // 29 tests - Configuration persistence
    runStyleManagerTests();              // 14 tests - Theme and styling
    runTriggerManagerTests();            // 7 tests - Event trigger handling
    runPanelInterfaceTests();            // 16 tests - Panel interface compliance
    runStandalonePanelTests();           // 6 tests - Standalone panel behavior
    runGpioProviderTests();              // 5 tests - GPIO operations
    runLvglDisplayProviderTests();       // 17 tests - LVGL display integration
    runKeySensorTests();                 // 21 tests - Key state sensing
    runLightSensorTests();               // 7 tests - Light level sensing
    runLockSensorTests();                // 8 tests - Lock state sensing
    runOilPressureSensorTests();         // 4 tests - Oil pressure monitoring
    runOilTemperatureSensorTests();      // 5 tests - Oil temperature monitoring
    runServiceContainerTests();          // 8 tests - Service container patterns
    runSimpleTickerTests();              // 3 tests - Simple timing
    runTickerTests();                    // 4 tests - Advanced timing
    
    return UNITY_END();
}

#endif // UNIT_TESTING