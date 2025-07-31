#include <unity.h>
#include "unit/utilities/test_utilities.h"

void setUp(void) {
    // Global setup for all tests
    MockHardware::reset();
}

void tearDown(void) {
    // Global cleanup after each test
}

// Service Container Tests
extern void run_service_container_tests(void);

// Dependency Injection Coverage Tests  
extern void run_dependency_injection_coverage_tests(void);

int main(void) {
    UNITY_BEGIN();
    
    printf("\n=== SERVICE CONTAINER TESTS ===\n");
    run_service_container_tests();
    
    printf("\n=== DEPENDENCY INJECTION COVERAGE TESTS ===\n");
    run_dependency_injection_coverage_tests();
    
    printf("\n=== UTILITIES TESTS ===\n");
    // No tests implemented yet
    // Will add them as we implement each feature
    
    return UNITY_END();
}
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

// PreferenceManager Tests (18 tests)
extern void test_preference_manager_singleton_access(void);
extern void test_preference_manager_successful_initialization(void);
extern void test_preference_manager_nvs_failure_recovery(void);
extern void test_preference_manager_persistent_failure(void);
extern void test_preference_manager_load_empty_config(void);
extern void test_preference_manager_load_valid_config(void);
extern void test_preference_manager_load_corrupted_config(void);
extern void test_preference_manager_load_missing_panel_name(void);
extern void test_preference_manager_save_config(void);
extern void test_preference_manager_save_and_load_roundtrip(void);
extern void test_preference_manager_config_persistence(void);
extern void test_preference_manager_create_default_config(void);
extern void test_preference_manager_default_config_consistency(void);
extern void test_preference_manager_full_lifecycle(void);
extern void test_preference_manager_error_recovery_flow(void);
extern void test_preference_manager_rapid_save_load(void);
extern void test_preference_manager_memory_consistency(void);
extern void test_preference_manager_panel_name_validation(void);

// Utilities Tests (14 tests)
extern void test_ticker_get_elapsed_millis_initial(void);
extern void test_ticker_get_elapsed_millis_subsequent_calls(void);
extern void test_ticker_get_elapsed_millis_multiple_calls(void);
extern void test_ticker_handle_dynamic_delay_fast_processing(void);
extern void test_ticker_handle_dynamic_delay_slow_processing(void);
extern void test_ticker_handle_dynamic_delay_exact_timing(void);
extern void test_ticker_handle_lv_tasks_tick_increment(void);
extern void test_ticker_handle_lv_tasks_multiple_calls(void);
extern void test_ticker_handle_lv_tasks_no_time_elapsed(void);
extern void test_lvtools_create_blank_screen(void);
extern void test_lvtools_create_blank_screen_multiple(void);
extern void test_lvtools_reset_screen(void);
extern void test_lvtools_reset_screen_null_handling(void);
extern void test_lvtools_screen_lifecycle(void);

// StyleManager Tests (27 tests)
extern void test_style_manager_singleton_access(void);
extern void test_style_manager_initialization_day_theme(void);
extern void test_style_manager_initialization_night_theme(void);
extern void test_style_manager_all_styles_initialized(void);
extern void test_style_manager_theme_switching_day_to_night(void);
extern void test_style_manager_theme_switching_night_to_day(void);
extern void test_style_manager_multiple_theme_switches(void);
extern void test_style_manager_day_theme_colors(void);
extern void test_style_manager_night_theme_colors(void);
extern void test_style_manager_color_consistency_across_themes(void);
extern void test_style_manager_apply_theme_to_screen(void);
extern void test_style_manager_screen_invalidation_on_theme_change(void);
extern void test_style_manager_background_style_properties(void);
extern void test_style_manager_text_style_properties(void);
extern void test_style_manager_gauge_indicator_properties(void);
extern void test_style_manager_gauge_items_properties(void);
extern void test_style_manager_gauge_main_properties(void);
extern void test_style_manager_gauge_danger_section_properties(void);
extern void test_style_manager_reset_styles(void);
extern void test_style_manager_reset_all_gauge_styles(void);
extern void test_style_manager_full_lifecycle(void);
extern void test_style_manager_theme_persistence(void);
extern void test_style_manager_null_theme_handling(void);
extern void test_style_manager_invalid_theme_handling(void);
extern void test_style_manager_repeated_initialization(void);
