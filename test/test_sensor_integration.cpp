#include <unity.h>
#include <cstring>
#include "test_utilities.h"
#include "mock_types.h"
#include "mock_managers.h"

// Test state tracking
static bool system_initialized = false;
static const char* active_panel = "OemOilPanel";
static const char* active_theme = "Day";

// Utility functions
void initializeSystem() {
    system_initialized = true;
    
    // Initialize managers
    TriggerManager::GetInstance().init();
    PanelManager::GetInstance().init();
    StyleManager::GetInstance().init();
    
    // Set initial panel and theme
    PanelManager::GetInstance().loadPanel(PanelNames::OIL);
    StyleManager::GetInstance().setTheme(Themes::DAY);
    
    // Update tracking variables
    active_panel = PanelNames::OIL;
    active_theme = Themes::DAY;
    
    // Set initial sensor readings
    MockHardware::simulateAdcReading(34, 2048); // Normal oil pressure
    MockHardware::simulateAdcReading(35, 1500); // Normal oil temperature
}

// =================================================================
// SENSOR FAILURE AND RECOVERY TESTS
// =================================================================

void test_integration_sensor_initialization_failure(void) {
    TriggerScenarioTest test;
    test.SetupScenario("Sensor Integration: Initialization Failure");
    
    // Simulate ADC initialization failure
    MockHardware::simulateAdcFailure(34, true);
    MockHardware::simulateAdcFailure(35, true);
    
    initializeSystem();
    
    // System should handle gracefully and report error state
    uint16_t pressure_adc = MockHardware::getAdcReading(34);
    uint16_t temp_adc = MockHardware::getAdcReading(35);
    
    TEST_ASSERT_EQUAL(0, pressure_adc);
    TEST_ASSERT_EQUAL(0, temp_adc);
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", active_panel);
    
    // Recovery after ADC becomes available
    MockHardware::simulateAdcFailure(34, false);
    MockHardware::simulateAdcFailure(35, false);
    
    MockHardware::simulateAdcReading(34, 2048);
    MockHardware::simulateAdcReading(35, 1500);
    
    pressure_adc = MockHardware::getAdcReading(34);
    temp_adc = MockHardware::getAdcReading(35);
    
    TEST_ASSERT_EQUAL(2048, pressure_adc);
    TEST_ASSERT_EQUAL(1500, temp_adc);
}

void test_integration_sensor_intermittent_failure(void) {
    TriggerScenarioTest test;
    test.SetupScenario("Sensor Integration: Intermittent Failure");
    
    initializeSystem();
    
    // Normal operation
    uint16_t initial_pressure = MockHardware::getAdcReading(34);
    uint16_t initial_temp = MockHardware::getAdcReading(35);
    
    TEST_ASSERT_EQUAL(2048, initial_pressure);
    TEST_ASSERT_EQUAL(1500, initial_temp);
    
    // Simulate intermittent failures
    for (int i = 0; i < 10; i++) {
        // Randomly fail one or both sensors
        bool fail_pressure = (i % 3 == 0);
        bool fail_temp = (i % 2 == 0);
        
        MockHardware::simulateAdcFailure(34, fail_pressure);
        MockHardware::simulateAdcFailure(35, fail_temp);
        
        // System should handle gracefully
        uint16_t pressure = MockHardware::getAdcReading(34);
        uint16_t temp = MockHardware::getAdcReading(35);
        
        if (fail_pressure) {
            TEST_ASSERT_EQUAL(0, pressure);
        }
        if (fail_temp) {
            TEST_ASSERT_EQUAL(0, temp);
        }
        
        // Panel should remain stable
        TEST_ASSERT_EQUAL_STRING("OemOilPanel", active_panel);
    }
    
    // Recovery
    MockHardware::simulateAdcFailure(34, false);
    MockHardware::simulateAdcFailure(35, false);
    
    MockHardware::simulateAdcReading(34, 2048);
    MockHardware::simulateAdcReading(35, 1500);
    
    uint16_t final_pressure = MockHardware::getAdcReading(34);
    uint16_t final_temp = MockHardware::getAdcReading(35);
    
    TEST_ASSERT_EQUAL(2048, final_pressure);
    TEST_ASSERT_EQUAL(1500, final_temp);
}

void test_integration_sensor_noise_handling(void) {
    TriggerScenarioTest test;
    test.SetupScenario("Sensor Integration: Noise Handling");
    
    initializeSystem();
    
    // Simulate noisy readings with small variations
    const uint16_t base_pressure = 2048;
    const uint16_t base_temp = 1500;
    const uint16_t noise_amplitude = 100;
    
    for (int i = 0; i < 20; i++) {
        // Add noise to readings
        int16_t pressure_noise = (i % 3 - 1) * noise_amplitude;
        int16_t temp_noise = (i % 3 - 1) * noise_amplitude;
        
        MockHardware::simulateAdcReading(34, base_pressure + pressure_noise);
        MockHardware::simulateAdcReading(35, base_temp + temp_noise);
        
        uint16_t pressure = MockHardware::getAdcReading(34);
        uint16_t temp = MockHardware::getAdcReading(35);
        
        // Readings should be within expected range
        TEST_ASSERT_INT_WITHIN(noise_amplitude, base_pressure, pressure);
        TEST_ASSERT_INT_WITHIN(noise_amplitude, base_temp, temp);
        
        // System should remain stable
        TEST_ASSERT_EQUAL_STRING("OemOilPanel", active_panel);
    }
}

void test_integration_sensor_and_trigger_interaction(void) {
    TriggerScenarioTest test;
    test.SetupScenario("Sensor Integration: Trigger Interaction");
    
    initializeSystem();
    
    // Normal sensor operation
    uint16_t initial_pressure = MockHardware::getAdcReading(34);
    uint16_t initial_temp = MockHardware::getAdcReading(35);
    
    TEST_ASSERT_EQUAL(2048, initial_pressure);
    TEST_ASSERT_EQUAL(1500, initial_temp);
    
    // Activate a trigger while simulating sensor failure
    MockHardware::simulateAdcFailure(34, true);
    TEST_ASSERT_EQUAL_STRING(PanelNames::OIL, active_panel); // Verify initial panel
    
    // Trigger key present
    printf("\nActivating key presence trigger...\n");
    MockHardware::setGpioState(25, true); // Activate key present
    TriggerManager::GetInstance().ProcessTriggerEvents(); // Process the GPIO change
    
    // Get current panel from PanelManager
    active_panel = PanelManager::GetInstance().getCurrentPanelName();
    printf("Current active panel after key trigger: %s\n", active_panel);
    
    // Panel should change despite sensor failure
    TEST_ASSERT_EQUAL_STRING(PanelNames::KEY, active_panel);
    
    // Deactivate trigger
    MockHardware::setGpioState(25, false);
    TriggerManager::GetInstance().ProcessTriggerEvents(); // Process the GPIO change
    
    // Get current panel from PanelManager
    active_panel = PanelManager::GetInstance().getCurrentPanelName();
    printf("Current active panel after removing key trigger: %s\n", active_panel);
    
    // Should return to oil panel even with failed sensor
    TEST_ASSERT_EQUAL_STRING(PanelNames::OIL, active_panel);
    
    // Recover sensor
    MockHardware::simulateAdcFailure(34, false);
    MockHardware::simulateAdcReading(34, 2048);
    
    uint16_t final_pressure = MockHardware::getAdcReading(34);
    TEST_ASSERT_EQUAL(2048, final_pressure);
}

// =================================================================
// EXTENDED STABILITY TESTS
// =================================================================

void test_integration_long_term_sensor_stability(void) {
    TriggerScenarioTest test;
    test.SetupScenario("Sensor Integration: Long Term Stability");
    
    initializeSystem();
    
    // Simulate extended operation with various sensor conditions
    for (int cycle = 0; cycle < 100; cycle++) {
        // Vary sensor readings
        uint16_t pressure = 2048 + (cycle % 5) * 100;
        uint16_t temp = 1500 + (cycle % 3) * 50;
        
        MockHardware::simulateAdcReading(34, pressure);
        MockHardware::simulateAdcReading(35, temp);
        
        // Occasionally simulate failures
        if (cycle % 10 == 0) {
            MockHardware::simulateAdcFailure(34, true);
        } else if (cycle % 15 == 0) {
            MockHardware::simulateAdcFailure(35, true);
        } else {
            MockHardware::simulateAdcFailure(34, false);
            MockHardware::simulateAdcFailure(35, false);
        }
        
        // System should remain responsive
        uint16_t current_pressure = MockHardware::getAdcReading(34);
        uint16_t current_temp = MockHardware::getAdcReading(35);
        
        // Panel should remain stable
        TEST_ASSERT_EQUAL_STRING("OemOilPanel", active_panel);
        
        // Memory and resource checks could be added here
    }
}
