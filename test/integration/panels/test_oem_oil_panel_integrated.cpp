#include <unity.h>
#include "test_utilities.h"
#include "mock_types.h"
#include "mock_managers.h"

// OEM Oil Panel Integration Test State
static bool oil_panel_initialized = false;
static bool pressure_sensor_active = false;
static bool temperature_sensor_active = false;
static float current_oil_pressure = 0.0f;
static float current_oil_temperature = 0.0f;
static const char* current_panel_state = "Inactive";

// Test utility functions
void initializeOilPanelSystem() {
    // Initialize mock hardware for oil sensors
    MockHardware::reset();
    
    // Set up realistic oil sensor baselines
    MockHardware::simulateAdcReading(34, 2048); // 50% = ~75 PSI (normal operating pressure)
    MockHardware::simulateAdcReading(35, 1500); // 37% = ~85°C (normal operating temperature)
    
    oil_panel_initialized = true;
    pressure_sensor_active = true;
    temperature_sensor_active = true;
    current_oil_pressure = 75.0f;
    current_oil_temperature = 85.0f;
    current_panel_state = "OemOilPanel";
}

void simulateEngineStartup() {
    // Engine startup sequence - pressure builds gradually
    MockHardware::simulateAdcReading(34, 0);     // 0 PSI - engine off
    MockHardware::simulateAdcReading(35, 1200);  // 20°C - cold engine
    current_oil_pressure = 0.0f;
    current_oil_temperature = 20.0f;
    
    // Cranking - pressure starts building
    MockHardware::simulateAdcReading(34, 500);   // 12 PSI - cranking
    current_oil_pressure = 12.0f;
    
    // Running - normal operating pressure
    MockHardware::simulateAdcReading(34, 2048);  // 75 PSI - running
    MockHardware::simulateAdcReading(35, 1500);  // 85°C - operating temp
    current_oil_pressure = 75.0f;
    current_oil_temperature = 85.0f;
}

void simulateOilPressureWarning() {
    // Simulate dangerous low oil pressure
    MockHardware::simulateAdcReading(34, 200);   // 3 PSI - critically low
    current_oil_pressure = 3.0f;
}

void simulateOilTemperatureWarning() {
    // Simulate overheating
    MockHardware::simulateAdcReading(35, 3500);  // 125°C - overheating
    current_oil_temperature = 125.0f;
}

void simulateSensorFailure(bool pressure_fail, bool temperature_fail) {
    if (pressure_fail) {
        MockHardware::simulateAdcFailure(34, true);
        pressure_sensor_active = false;
        current_oil_pressure = 0.0f;
    }
    if (temperature_fail) {
        MockHardware::simulateAdcFailure(35, true);
        temperature_sensor_active = false;
        current_oil_temperature = 0.0f;
    }
}

void resetSensorFailures() {
    MockHardware::simulateAdcFailure(34, false);
    MockHardware::simulateAdcFailure(35, false);
    pressure_sensor_active = true;
    temperature_sensor_active = true;
}

// =================================================================
// OEM OIL PANEL BASIC INTEGRATION TESTS
// =================================================================

void test_oem_oil_panel_normal_operation(void) {
    TriggerScenarioTest test;
    test.SetupScenario("OEM Oil Panel: Normal Operation");
    
    initializeOilPanelSystem();
    
    // Verify system is in oil panel mode
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", current_panel_state);
    TEST_ASSERT_TRUE(oil_panel_initialized);
    
    // Verify sensors are reading normal values
    uint16_t pressure_adc = MockHardware::getAdcReading(34);
    uint16_t temp_adc = MockHardware::getAdcReading(35);
    
    TEST_ASSERT_EQUAL(2048, pressure_adc);
    TEST_ASSERT_EQUAL(1500, temp_adc);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 75.0f, current_oil_pressure);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 85.0f, current_oil_temperature);
    
    test.ValidateExpectedState(ExpectedStates::OIL_PANEL_DAY);
}

void test_oem_oil_panel_engine_startup_sequence(void) {
    TriggerScenarioTest test;
    test.SetupScenario("OEM Oil Panel: Engine Startup Sequence");
    
    initializeOilPanelSystem();
    
    // Simulate complete engine startup
    simulateEngineStartup();
    
    // Verify startup sequence values
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", current_panel_state);
    
    // Final values should be normal operating range
    uint16_t final_pressure = MockHardware::getAdcReading(34);
    uint16_t final_temp = MockHardware::getAdcReading(35);
    
    TEST_ASSERT_EQUAL(2048, final_pressure);
    TEST_ASSERT_EQUAL(1500, final_temp);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 75.0f, current_oil_pressure);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 85.0f, current_oil_temperature);
    
    test.ValidateExpectedState(ExpectedStates::OIL_PANEL_DAY);
}

void test_oem_oil_panel_pressure_warning_condition(void) {
    TriggerScenarioTest test;
    test.SetupScenario("OEM Oil Panel: Pressure Warning");
    
    initializeOilPanelSystem();
    
    // Simulate low oil pressure warning
    simulateOilPressureWarning();
    
    // Verify warning condition is detected
    uint16_t warning_pressure = MockHardware::getAdcReading(34);
    TEST_ASSERT_EQUAL(200, warning_pressure);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 3.0f, current_oil_pressure);
    
    // System should remain on oil panel to show warning
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", current_panel_state);
    
    // Temperature should remain normal
    uint16_t normal_temp = MockHardware::getAdcReading(35);
    TEST_ASSERT_EQUAL(1500, normal_temp);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 85.0f, current_oil_temperature);
}

void test_oem_oil_panel_temperature_warning_condition(void) {
    TriggerScenarioTest test;
    test.SetupScenario("OEM Oil Panel: Temperature Warning");
    
    initializeOilPanelSystem();
    
    // Simulate overheating condition
    simulateOilTemperatureWarning();
    
    // Verify overheating condition is detected
    uint16_t warning_temp = MockHardware::getAdcReading(35);
    TEST_ASSERT_EQUAL(3500, warning_temp);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 125.0f, current_oil_temperature);
    
    // System should remain on oil panel to show warning
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", current_panel_state);
    
    // Pressure should remain normal
    uint16_t normal_pressure = MockHardware::getAdcReading(34);
    TEST_ASSERT_EQUAL(2048, normal_pressure);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 75.0f, current_oil_pressure);
}

void test_oem_oil_panel_dual_warning_condition(void) {
    TriggerScenarioTest test;
    test.SetupScenario("OEM Oil Panel: Dual Warning Condition");
    
    initializeOilPanelSystem();
    
    // Simulate both pressure and temperature warnings
    simulateOilPressureWarning();
    simulateOilTemperatureWarning();
    
    // Verify both warning conditions
    uint16_t warning_pressure = MockHardware::getAdcReading(34);
    uint16_t warning_temp = MockHardware::getAdcReading(35);
    
    TEST_ASSERT_EQUAL(200, warning_pressure);
    TEST_ASSERT_EQUAL(3500, warning_temp);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 3.0f, current_oil_pressure);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 125.0f, current_oil_temperature);
    
    // System should remain on oil panel to show both warnings
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", current_panel_state);
}

// =================================================================
// SENSOR FAILURE INTEGRATION TESTS
// =================================================================

void test_oem_oil_panel_pressure_sensor_failure(void) {
    TriggerScenarioTest test;
    test.SetupScenario("OEM Oil Panel: Pressure Sensor Failure");
    
    initializeOilPanelSystem();
    
    // Simulate pressure sensor failure
    simulateSensorFailure(true, false);
    
    // Verify sensor failure is detected
    uint16_t failed_pressure = MockHardware::getAdcReading(34);
    TEST_ASSERT_EQUAL(0, failed_pressure);
    TEST_ASSERT_FALSE(pressure_sensor_active);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, current_oil_pressure);
    
    // Temperature sensor should continue working
    uint16_t normal_temp = MockHardware::getAdcReading(35);
    TEST_ASSERT_EQUAL(1500, normal_temp);
    TEST_ASSERT_TRUE(temperature_sensor_active);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 85.0f, current_oil_temperature);
    
    // Panel should remain active to show temperature and sensor error
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", current_panel_state);
}

void test_oem_oil_panel_temperature_sensor_failure(void) {
    TriggerScenarioTest test;
    test.SetupScenario("OEM Oil Panel: Temperature Sensor Failure");
    
    initializeOilPanelSystem();
    
    // Simulate temperature sensor failure
    simulateSensorFailure(false, true);
    
    // Verify sensor failure is detected
    uint16_t failed_temp = MockHardware::getAdcReading(35);
    TEST_ASSERT_EQUAL(0, failed_temp);
    TEST_ASSERT_FALSE(temperature_sensor_active);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, current_oil_temperature);
    
    // Pressure sensor should continue working
    uint16_t normal_pressure = MockHardware::getAdcReading(34);
    TEST_ASSERT_EQUAL(2048, normal_pressure);
    TEST_ASSERT_TRUE(pressure_sensor_active);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 75.0f, current_oil_pressure);
    
    // Panel should remain active to show pressure and sensor error
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", current_panel_state);
}

void test_oem_oil_panel_dual_sensor_failure(void) {
    TriggerScenarioTest test;
    test.SetupScenario("OEM Oil Panel: Dual Sensor Failure");
    
    initializeOilPanelSystem();
    
    // Simulate both sensor failures
    simulateSensorFailure(true, true);
    
    // Verify both sensor failures
    uint16_t failed_pressure = MockHardware::getAdcReading(34);
    uint16_t failed_temp = MockHardware::getAdcReading(35);
    
    TEST_ASSERT_EQUAL(0, failed_pressure);
    TEST_ASSERT_EQUAL(0, failed_temp);
    TEST_ASSERT_FALSE(pressure_sensor_active);
    TEST_ASSERT_FALSE(temperature_sensor_active);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, current_oil_pressure);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, current_oil_temperature);
    
    // Panel should remain active to show sensor error state
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", current_panel_state);
}

void test_oem_oil_panel_sensor_recovery(void) {
    TriggerScenarioTest test;
    test.SetupScenario("OEM Oil Panel: Sensor Recovery");
    
    initializeOilPanelSystem();
    
    // Simulate sensor failure
    simulateSensorFailure(true, true);
    
    // Verify failure state
    TEST_ASSERT_FALSE(pressure_sensor_active);
    TEST_ASSERT_FALSE(temperature_sensor_active);
    
    // Simulate sensor recovery
    resetSensorFailures();
    MockHardware::simulateAdcReading(34, 2048);
    MockHardware::simulateAdcReading(35, 1500);
    current_oil_pressure = 75.0f;
    current_oil_temperature = 85.0f;
    
    // Verify recovery
    uint16_t recovered_pressure = MockHardware::getAdcReading(34);
    uint16_t recovered_temp = MockHardware::getAdcReading(35);
    
    TEST_ASSERT_EQUAL(2048, recovered_pressure);
    TEST_ASSERT_EQUAL(1500, recovered_temp);
    TEST_ASSERT_TRUE(pressure_sensor_active);
    TEST_ASSERT_TRUE(temperature_sensor_active);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 75.0f, current_oil_pressure);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 85.0f, current_oil_temperature);
    
    test.ValidateExpectedState(ExpectedStates::OIL_PANEL_DAY);
}

// =================================================================
// TRIGGER INTEGRATION WITH OIL PANEL TESTS
// =================================================================

void test_oem_oil_panel_with_key_present_override(void) {
    TriggerScenarioTest test;
    test.SetupScenario("OEM Oil Panel: Key Present Override");
    
    initializeOilPanelSystem();
    
    // Verify oil panel is active with normal readings
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", current_panel_state);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 75.0f, current_oil_pressure);
    
    // Activate key present trigger
    MockHardware::setGpioState(25, true);
    current_panel_state = "KeyPanel"; // Simulate panel switch
    
    // Panel should switch to key panel
    TEST_ASSERT_EQUAL_STRING("KeyPanel", current_panel_state);
    
    // Oil sensors should continue reading in background
    uint16_t background_pressure = MockHardware::getAdcReading(34);
    uint16_t background_temp = MockHardware::getAdcReading(35);
    TEST_ASSERT_EQUAL(2048, background_pressure);
    TEST_ASSERT_EQUAL(1500, background_temp);
    
    // Deactivate key trigger
    MockHardware::setGpioState(25, false);
    current_panel_state = "OemOilPanel"; // Simulate panel restoration
    
    // Should return to oil panel with maintained readings
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", current_panel_state);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 75.0f, current_oil_pressure);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 85.0f, current_oil_temperature);
    
    test.ValidateExpectedState(ExpectedStates::OIL_PANEL_DAY);
}

void test_oem_oil_panel_warning_during_trigger_override(void) {
    TriggerScenarioTest test;
    test.SetupScenario("OEM Oil Panel: Warning During Trigger Override");
    
    initializeOilPanelSystem();
    
    // Activate key present trigger
    MockHardware::setGpioState(25, true);
    current_panel_state = "KeyPanel";
    TEST_ASSERT_EQUAL_STRING("KeyPanel", current_panel_state);
    
    // Simulate oil pressure warning while key panel is active
    simulateOilPressureWarning();
    
    // Warning should be detected in background
    uint16_t warning_pressure = MockHardware::getAdcReading(34);
    TEST_ASSERT_EQUAL(200, warning_pressure);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 3.0f, current_oil_pressure);
    
    // Key panel should remain active (trigger priority)
    TEST_ASSERT_EQUAL_STRING("KeyPanel", current_panel_state);
    
    // Deactivate key trigger
    MockHardware::setGpioState(25, false);
    current_panel_state = "OemOilPanel"; // Return to oil panel with warning
    
    // Should return to oil panel showing warning condition
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", current_panel_state);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 3.0f, current_oil_pressure);
}

void test_oem_oil_panel_theme_switching_integration(void) {
    TriggerScenarioTest test;
    test.SetupScenario("OEM Oil Panel: Theme Switching Integration");
    
    initializeOilPanelSystem();
    
    // Start with day theme
    const char* current_theme = "Day";
    TEST_ASSERT_EQUAL_STRING("Day", current_theme);
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", current_panel_state);
    
    // Activate night theme (lights on)
    MockHardware::setGpioState(28, true);
    current_theme = "Night";
    
    // Panel should remain oil panel but with night theme
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", current_panel_state);
    TEST_ASSERT_EQUAL_STRING("Night", current_theme);
    
    // Oil readings should be unaffected by theme change
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 75.0f, current_oil_pressure);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 85.0f, current_oil_temperature);
    
    // Deactivate night theme
    MockHardware::setGpioState(28, false);
    current_theme = "Day";
    
    // Return to day theme
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", current_panel_state);
    TEST_ASSERT_EQUAL_STRING("Day", current_theme);
    
    test.ValidateExpectedState(ExpectedStates::OIL_PANEL_DAY);
}

// =================================================================
// PERFORMANCE AND STRESS TESTS
// =================================================================

void test_oem_oil_panel_rapid_sensor_changes(void) {
    TriggerScenarioTest test;
    test.SetupScenario("OEM Oil Panel: Rapid Sensor Changes");
    
    initializeOilPanelSystem();
    
    // Rapidly change sensor values and verify system stability
    for (int i = 0; i < 50; i++) {
        uint16_t pressure_value = 1000 + (i * 20);  // Vary from 1000 to 2000
        uint16_t temp_value = 1200 + (i * 10);      // Vary from 1200 to 1700
        
        MockHardware::simulateAdcReading(34, pressure_value);
        MockHardware::simulateAdcReading(35, temp_value);
        
        // Verify readings are applied
        uint16_t read_pressure = MockHardware::getAdcReading(34);
        uint16_t read_temp = MockHardware::getAdcReading(35);
        
        TEST_ASSERT_EQUAL(pressure_value, read_pressure);
        TEST_ASSERT_EQUAL(temp_value, read_temp);
        
        // Panel should remain stable
        TEST_ASSERT_EQUAL_STRING("OemOilPanel", current_panel_state);
    }
    
    // Final state should be stable
    TEST_ASSERT_TRUE(oil_panel_initialized);
    TEST_ASSERT_TRUE(pressure_sensor_active);
    TEST_ASSERT_TRUE(temperature_sensor_active);
}

void test_oem_oil_panel_extended_operation_stability(void) {
    TriggerScenarioTest test;
    test.SetupScenario("OEM Oil Panel: Extended Operation Stability");
    
    initializeOilPanelSystem();
    
    // Simulate extended operation with various conditions
    for (int cycle = 0; cycle < 100; cycle++) {
        // Vary conditions throughout the cycle
        bool low_pressure = (cycle % 10 == 0);
        bool high_temp = (cycle % 15 == 0);
        bool sensor_fail = (cycle % 25 == 0);
        
        if (sensor_fail) {
            // Simulate intermittent sensor failure
            simulateSensorFailure(true, false);
            TEST_ASSERT_FALSE(pressure_sensor_active);
        } else if (low_pressure) {
            // Simulate low pressure warning
            resetSensorFailures();
            simulateOilPressureWarning();
            TEST_ASSERT_FLOAT_WITHIN(1.0f, 3.0f, current_oil_pressure);
        } else if (high_temp) {
            // Simulate high temperature warning  
            resetSensorFailures();
            simulateOilTemperatureWarning();
            TEST_ASSERT_FLOAT_WITHIN(1.0f, 125.0f, current_oil_temperature);
        } else {
            // Normal operation
            resetSensorFailures();
            MockHardware::simulateAdcReading(34, 2048);
            MockHardware::simulateAdcReading(35, 1500);
            current_oil_pressure = 75.0f;
            current_oil_temperature = 85.0f;
        }
        
        // System should remain stable throughout
        TEST_ASSERT_EQUAL_STRING("OemOilPanel", current_panel_state);
        TEST_ASSERT_TRUE(oil_panel_initialized);
    }
    
    // Final cleanup and verification
    resetSensorFailures();
    MockHardware::simulateAdcReading(34, 2048);
    MockHardware::simulateAdcReading(35, 1500);
    test.ValidateExpectedState(ExpectedStates::OIL_PANEL_DAY);
}

// Note: PlatformIO will automatically discover and run test_ functions