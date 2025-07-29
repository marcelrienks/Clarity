#include <unity.h>
#include "test_utilities.h"

void setUp(void) {
    // Global setup for all tests
    MockHardware::reset();
    
    // Reset panel manager mock state (declared in test_panel_manager.cpp)
    extern std::vector<const char*> panel_creation_history;
    extern std::vector<const char*> panel_load_history;
    extern bool panel_loaded;
    extern bool panel_initialized;
    panel_creation_history.clear();
    panel_load_history.clear();
    panel_loaded = false;
    panel_initialized = false;
    
    // Reset sensor mock state (declared in test_sensors.cpp)
    extern bool sensor_initialized;
    extern uint32_t last_update_time;
    extern int32_t current_oil_pressure;
    extern int32_t current_oil_temperature;
    sensor_initialized = false;
    last_update_time = 0;
    current_oil_pressure = 0;
    current_oil_temperature = 0;
    resetSensorMockTiming();
}

void tearDown(void) {
    // Global cleanup after each test
}

// =================================================================
// EXTERNAL TEST FUNCTION DECLARATIONS
// =================================================================

// Trigger System Tests (18 tests - all scenarios from docs/scenarios.md)
extern void test_S1_1_clean_system_startup(void);
extern void test_S1_2_startup_with_key_present(void);
extern void test_S1_3_startup_with_key_not_present(void);
extern void test_S1_4_startup_with_lock_active(void);
extern void test_S1_5_startup_with_theme_trigger(void);
extern void test_S2_2_lock_trigger(void);
extern void test_S2_3_key_present_trigger(void);
extern void test_S2_4_key_not_present_trigger(void);
extern void test_S3_1_priority_override_key_over_lock(void);
extern void test_S3_2_key_present_vs_key_not_present(void);
extern void test_S3_2_intermediate_state_validation(void);
extern void test_S4_1_rapid_toggle_single_trigger(void);
extern void test_S4_2_rapid_toggle_multiple_triggers(void);
extern void test_S4_5_invalid_trigger_combinations(void);
extern void test_S4_4_simultaneous_deactivation(void);
extern void test_S5_1_high_frequency_trigger_events(void);
extern void test_S5_3_panel_load_performance(void);
extern void test_complex_restoration_chain(void);

// Panel Manager Tests (16 tests)
extern void test_panel_manager_initialization(void);
extern void test_panel_registration(void);
extern void test_panel_creation_and_loading(void);
extern void test_panel_cleanup_on_switch(void);
extern void test_panel_lifecycle_init_load_update(void);
extern void test_splash_panel_lifecycle(void);
extern void test_trigger_driven_panel_switch(void);
extern void test_panel_restoration_chain(void);
extern void test_rapid_panel_switching(void);
extern void test_panel_state_consistency(void);
extern void test_panel_memory_management(void);
extern void test_invalid_panel_creation(void);
extern void test_panel_creation_failure_recovery(void);
extern void test_panel_trigger_integration(void);
extern void test_multiple_trigger_panel_priority(void);
extern void test_panel_switching_performance(void);

// Sensor Tests (15 tests)
extern void test_oil_pressure_sensor_initialization(void);
extern void test_oil_temperature_sensor_initialization(void);
extern void test_oil_pressure_reading_accuracy(void);
extern void test_oil_temperature_reading_accuracy(void);
extern void test_sensor_reading_bounds(void);
extern void test_sensor_update_interval(void);
extern void test_sensor_reading_consistency(void);
extern void test_sensor_reading_without_initialization(void);
extern void test_sensor_adc_failure_handling(void);
extern void test_dual_sensor_operation(void);
extern void test_sensor_value_change_detection(void);
extern void test_sensor_reading_performance(void);
extern void test_sensor_memory_usage(void);
extern void test_engine_startup_scenario(void);
extern void test_sensor_fault_simulation(void);

// Integration Tests (11 tests)
extern void test_integration_S1_1_clean_system_startup(void);
extern void test_integration_S1_2_startup_with_triggers(void);
extern void test_integration_S3_1_priority_override_complete(void);
extern void test_integration_S3_4_theme_and_panel_triggers(void);
extern void test_integration_S3_5_triple_trigger_activation(void);
extern void test_integration_S4_5_invalid_combinations(void);
extern void test_integration_S4_4_simultaneous_deactivation(void);
extern void test_integration_sensor_and_trigger_system(void);
extern void test_integration_long_running_stability(void);
extern void test_integration_rapid_state_changes(void);
extern void test_integration_system_recovery(void);

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    // =================================================================
    // TRIGGER SYSTEM TESTS (18 tests - all scenarios S1-S5)
    // =================================================================
    printf("\n=== TRIGGER SYSTEM TESTS ===\n");
    
    // System Startup Scenarios (S1.1-S1.5)
    RUN_TEST(test_S1_1_clean_system_startup);
    RUN_TEST(test_S1_2_startup_with_key_present);
    RUN_TEST(test_S1_3_startup_with_key_not_present);
    RUN_TEST(test_S1_4_startup_with_lock_active);
    RUN_TEST(test_S1_5_startup_with_theme_trigger);
    
    // Single Trigger Scenarios (S2.2-S2.4)
    RUN_TEST(test_S2_2_lock_trigger);
    RUN_TEST(test_S2_3_key_present_trigger);
    RUN_TEST(test_S2_4_key_not_present_trigger);
    
    // Multiple Trigger Scenarios (S3.1-S3.2)
    RUN_TEST(test_S3_1_priority_override_key_over_lock);
    RUN_TEST(test_S3_2_key_present_vs_key_not_present);
    RUN_TEST(test_S3_2_intermediate_state_validation);
    
    // Edge Case Scenarios (S4.1-S4.5)
    RUN_TEST(test_S4_1_rapid_toggle_single_trigger);
    RUN_TEST(test_S4_2_rapid_toggle_multiple_triggers);
    RUN_TEST(test_S4_5_invalid_trigger_combinations);
    RUN_TEST(test_S4_4_simultaneous_deactivation);
    
    // Performance Scenarios (S5.1-S5.3)
    RUN_TEST(test_S5_1_high_frequency_trigger_events);
    RUN_TEST(test_S5_3_panel_load_performance);
    
    // Complex Scenarios
    RUN_TEST(test_complex_restoration_chain);
    
    // =================================================================
    // PANEL MANAGER TESTS (16 tests)
    // =================================================================
    printf("\n=== PANEL MANAGER TESTS ===\n");
    
    // Core functionality
    RUN_TEST(test_panel_manager_initialization);
    RUN_TEST(test_panel_registration);
    RUN_TEST(test_panel_creation_and_loading);
    RUN_TEST(test_panel_cleanup_on_switch);
    
    // Lifecycle tests
    RUN_TEST(test_panel_lifecycle_init_load_update);
    RUN_TEST(test_splash_panel_lifecycle);
    
    // Panel switching
    RUN_TEST(test_trigger_driven_panel_switch);
    RUN_TEST(test_panel_restoration_chain);
    RUN_TEST(test_rapid_panel_switching);
    
    // State management
    RUN_TEST(test_panel_state_consistency);
    RUN_TEST(test_panel_memory_management);
    
    // Error handling
    RUN_TEST(test_invalid_panel_creation);
    RUN_TEST(test_panel_creation_failure_recovery);
    
    // Integration tests
    RUN_TEST(test_panel_trigger_integration);
    RUN_TEST(test_multiple_trigger_panel_priority);
    
    // Performance tests
    RUN_TEST(test_panel_switching_performance);
    
    // =================================================================
    // SENSOR TESTS (15 tests)
    // =================================================================
    printf("\n=== SENSOR TESTS ===\n");
    
    // Initialization tests
    RUN_TEST(test_oil_pressure_sensor_initialization);
    RUN_TEST(test_oil_temperature_sensor_initialization);
    
    // Reading accuracy tests
    RUN_TEST(test_oil_pressure_reading_accuracy);
    RUN_TEST(test_oil_temperature_reading_accuracy);
    RUN_TEST(test_sensor_reading_bounds);
    
    // Timing tests
    RUN_TEST(test_sensor_update_interval);
    RUN_TEST(test_sensor_reading_consistency);
    
    // Error handling tests
    RUN_TEST(test_sensor_reading_without_initialization);
    RUN_TEST(test_sensor_adc_failure_handling);
    
    // Integration tests
    RUN_TEST(test_dual_sensor_operation);
    RUN_TEST(test_sensor_value_change_detection);
    
    // Performance tests
    RUN_TEST(test_sensor_reading_performance);
    RUN_TEST(test_sensor_memory_usage);
    
    // Realistic scenarios
    RUN_TEST(test_engine_startup_scenario);
    RUN_TEST(test_sensor_fault_simulation);
    
    // =================================================================
    // INTEGRATION TESTS (11 tests)
    // =================================================================
    printf("\n=== INTEGRATION TESTS ===\n");
    
    // Basic integration scenarios
    RUN_TEST(test_integration_S1_1_clean_system_startup);
    RUN_TEST(test_integration_S1_2_startup_with_triggers);
    
    // Multi-trigger integration
    RUN_TEST(test_integration_S3_1_priority_override_complete);
    RUN_TEST(test_integration_S3_4_theme_and_panel_triggers);
    RUN_TEST(test_integration_S3_5_triple_trigger_activation);
    
    // Edge case integration
    RUN_TEST(test_integration_S4_5_invalid_combinations);
    RUN_TEST(test_integration_S4_4_simultaneous_deactivation);
    
    // Sensor integration
    RUN_TEST(test_integration_sensor_and_trigger_system);
    
    // Long running tests
    RUN_TEST(test_integration_long_running_stability);
    RUN_TEST(test_integration_rapid_state_changes);
    
    // Recovery tests
    RUN_TEST(test_integration_system_recovery);
    
    printf("\n=== ALL TESTS COMPLETE ===\n");
    printf("Total: 60 comprehensive tests covering all scenarios from docs/scenarios.md\n");
    
    return UNITY_END();
}