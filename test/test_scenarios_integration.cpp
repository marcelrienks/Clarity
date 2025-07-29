#include <unity.h>
#include <cstring>
#include "test_utilities.h"

// Integration test state tracking
static bool system_initialized = false;
static const char* active_panel = "OemOilPanel";
static const char* active_theme = "Day";
static bool triggers_active[4] = {false}; // key_present, key_not_present, lock_state, lights_state

// Note: setUp() and tearDown() are defined in test_main.cpp

// Mock system integration functions
void mockSystemInitialization() {
    system_initialized = true;
    active_panel = "SplashPanel";
    active_theme = "Day";
}

void mockSplashCompletion() {
    if (strcmp(active_panel, "SplashPanel") == 0) {
        active_panel = "OemOilPanel";
    }
}

void mockTriggerSystemUpdate() {
    // Simulate trigger system processing GPIO changes
    bool key_present = MockHardware::getGpioState(25);
    bool key_not_present = MockHardware::getGpioState(26);
    bool lock_state = MockHardware::getGpioState(27);
    bool lights_state = MockHardware::getGpioState(28);
    
    triggers_active[0] = key_present;
    triggers_active[1] = key_not_present;
    triggers_active[2] = lock_state;
    triggers_active[3] = lights_state;
    
    // Apply trigger priority logic
    if (key_present || key_not_present) {
        active_panel = "KeyPanel";
    } else if (lock_state) {
        active_panel = "LockPanel";
    } else {
        active_panel = "OemOilPanel";
    }
    
    // Apply theme logic
    if (lights_state) {
        active_theme = "Night";
    } else {
        active_theme = "Day";
    }
}

// =================================================================
// INTEGRATION TEST SCENARIOS FROM SCENARIOS.MD
// =================================================================

void test_integration_S1_1_clean_system_startup(void) {
    // S1.1: Clean System Startup
    TriggerScenarioTest test;
    test.SetupScenario("Integration S1.1: Clean System Startup");
    
    // System initialization
    mockSystemInitialization();
    TEST_ASSERT_TRUE(system_initialized);
    TEST_ASSERT_EQUAL_STRING("SplashPanel", active_panel);
    
    // Splash completion after timeout
    mockSplashCompletion();
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", active_panel);
    TEST_ASSERT_EQUAL_STRING("Day", active_theme);
    
    // No triggers should be active
    for (int i = 0; i < 4; i++) {
        TEST_ASSERT_FALSE(triggers_active[i]);
    }
    
    test.ValidateExpectedState(ExpectedStates::OIL_PANEL_DAY);
}

void test_integration_S1_2_startup_with_triggers(void) {
    // S1.2: Startup with Active Triggers
    TriggerScenarioTest test;
    test.SetupScenario("Integration S1.2: Startup with Key Present");
    
    // Set key present before system startup
    MockHardware::setGpioState(25, true);
    
    mockSystemInitialization();
    mockSplashCompletion();
    mockTriggerSystemUpdate();
    
    TEST_ASSERT_EQUAL_STRING("KeyPanel", active_panel);
    TEST_ASSERT_TRUE(triggers_active[0]); // key_present
    
    test.ValidateExpectedState(ExpectedStates::KEY_PANEL_GREEN);
}

void test_integration_S3_1_priority_override_complete(void) {
    // S3.1: Priority Override - Key Present Over Lock (Complete Integration)
    TriggerScenarioTest test;
    test.SetupScenario("Integration S3.1: Priority Override Complete");
    
    // Start with clean system
    mockSystemInitialization();
    mockSplashCompletion();
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", active_panel);
    
    // Step 1: Activate lock trigger
    MockHardware::setGpioState(27, true);
    mockTriggerSystemUpdate();
    TEST_ASSERT_EQUAL_STRING("LockPanel", active_panel);
    TEST_ASSERT_TRUE(triggers_active[2]); // lock_state
    
    // Step 2: Activate key present (should override)
    MockHardware::setGpioState(25, true);
    mockTriggerSystemUpdate();
    TEST_ASSERT_EQUAL_STRING("KeyPanel", active_panel);
    TEST_ASSERT_TRUE(triggers_active[0]); // key_present
    TEST_ASSERT_TRUE(triggers_active[2]); // lock_state still active
    
    // Step 3: Deactivate key present (lock should restore)
    MockHardware::setGpioState(25, false);
    mockTriggerSystemUpdate();
    TEST_ASSERT_EQUAL_STRING("LockPanel", active_panel);
    TEST_ASSERT_FALSE(triggers_active[0]); // key_present
    TEST_ASSERT_TRUE(triggers_active[2]); // lock_state
    
    // Step 4: Deactivate lock (oil panel should restore)
    MockHardware::setGpioState(27, false);
    mockTriggerSystemUpdate();
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", active_panel);
    TEST_ASSERT_FALSE(triggers_active[2]); // lock_state
    
    test.ValidateExpectedState(ExpectedStates::OIL_PANEL_DAY);
}

void test_integration_S3_4_theme_and_panel_triggers(void) {
    // S3.4: Theme + Panel Triggers Integration
    TriggerScenarioTest test;
    test.SetupScenario("Integration S3.4: Theme + Panel Triggers");
    
    // Start clean
    mockSystemInitialization();
    mockSplashCompletion();
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", active_panel);
    TEST_ASSERT_EQUAL_STRING("Day", active_theme);
    
    // Step 1: Activate lights (theme change)
    MockHardware::setGpioState(28, true);
    mockTriggerSystemUpdate();
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", active_panel); // Panel unchanged
    TEST_ASSERT_EQUAL_STRING("Night", active_theme); // Theme changed
    
    // Step 2: Activate key present (panel change with night theme)
    MockHardware::setGpioState(25, true);
    mockTriggerSystemUpdate();
    TEST_ASSERT_EQUAL_STRING("KeyPanel", active_panel);
    TEST_ASSERT_EQUAL_STRING("Night", active_theme); // Theme maintained
    
    // Step 3: Deactivate key present (restore oil with night theme)
    MockHardware::setGpioState(25, false);
    mockTriggerSystemUpdate();
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", active_panel);
    TEST_ASSERT_EQUAL_STRING("Night", active_theme); // Theme maintained
    
    // Step 4: Deactivate lights (restore day theme)
    MockHardware::setGpioState(28, false);
    mockTriggerSystemUpdate();
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", active_panel);
    TEST_ASSERT_EQUAL_STRING("Day", active_theme);
    
    test.ValidateExpectedState(ExpectedStates::OIL_PANEL_DAY);
}

void test_integration_S3_5_triple_trigger_activation(void) {
    // S3.5: Triple Trigger Activation Integration
    TriggerScenarioTest test;
    test.SetupScenario("Integration S3.5: Triple Trigger Activation");
    
    mockSystemInitialization();
    mockSplashCompletion();
    
    // Step 1: Activate lights (theme)
    MockHardware::setGpioState(28, true);
    mockTriggerSystemUpdate();
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", active_panel);
    TEST_ASSERT_EQUAL_STRING("Night", active_theme);
    
    // Step 2: Activate lock (panel change with night theme)
    MockHardware::setGpioState(27, true);
    mockTriggerSystemUpdate();
    TEST_ASSERT_EQUAL_STRING("LockPanel", active_panel);
    TEST_ASSERT_EQUAL_STRING("Night", active_theme);
    
    // Step 3: Activate key present (highest priority)
    MockHardware::setGpioState(25, true);
    mockTriggerSystemUpdate();
    TEST_ASSERT_EQUAL_STRING("KeyPanel", active_panel);
    TEST_ASSERT_EQUAL_STRING("Night", active_theme);
    
    // Step 4: Deactivate key present (restore lock with night theme)
    MockHardware::setGpioState(25, false);
    mockTriggerSystemUpdate();
    TEST_ASSERT_EQUAL_STRING("LockPanel", active_panel);
    TEST_ASSERT_EQUAL_STRING("Night", active_theme);
    
    // Step 5: Deactivate lock (restore oil with night theme)
    MockHardware::setGpioState(27, false);
    mockTriggerSystemUpdate();
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", active_panel);
    TEST_ASSERT_EQUAL_STRING("Night", active_theme);
    
    // Step 6: Deactivate lights (restore day theme)
    MockHardware::setGpioState(28, false);
    mockTriggerSystemUpdate();
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", active_panel);
    TEST_ASSERT_EQUAL_STRING("Day", active_theme);
    
    test.ValidateExpectedState(ExpectedStates::OIL_PANEL_DAY);
}

// =================================================================
// EDGE CASE INTEGRATION TESTS
// =================================================================

void test_integration_S4_5_invalid_combinations(void) {
    // S4.5: Invalid Trigger Combinations Integration
    TriggerScenarioTest test;
    test.SetupScenario("Integration S4.5: Invalid Combinations");
    
    mockSystemInitialization();
    mockSplashCompletion();
    
    // Activate both key triggers simultaneously (invalid hardware state)
    MockHardware::setGpioState(25, true); // key_present
    MockHardware::setGpioState(26, true); // key_not_present
    mockTriggerSystemUpdate();
    
    // System should handle gracefully (FIFO behavior)
    TEST_ASSERT_EQUAL_STRING("KeyPanel", active_panel);
    TEST_ASSERT_TRUE(triggers_active[0] || triggers_active[1]); // One or both active
    
    // Deactivate one trigger
    MockHardware::setGpioState(25, false);
    mockTriggerSystemUpdate();
    TEST_ASSERT_EQUAL_STRING("KeyPanel", active_panel);
    TEST_ASSERT_TRUE(triggers_active[1]); // key_not_present should remain
    
    // Deactivate remaining trigger
    MockHardware::setGpioState(26, false);
    mockTriggerSystemUpdate();
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", active_panel);
    
    test.ValidateExpectedState(ExpectedStates::OIL_PANEL_DAY);
}

void test_integration_S4_4_simultaneous_deactivation(void) {
    // S4.4: Simultaneous Deactivation Integration
    TriggerScenarioTest test;
    test.SetupScenario("Integration S4.4: Simultaneous Deactivation");
    
    mockSystemInitialization();
    mockSplashCompletion();
    
    // Activate multiple triggers
    MockHardware::setGpioState(25, true); // key_present
    MockHardware::setGpioState(27, true); // lock_state
    MockHardware::setGpioState(28, true); // lights_state
    mockTriggerSystemUpdate();
    
    TEST_ASSERT_EQUAL_STRING("KeyPanel", active_panel); // Highest priority
    TEST_ASSERT_EQUAL_STRING("Night", active_theme);
    
    // Deactivate all triggers simultaneously
    MockHardware::setGpioState(25, false);
    MockHardware::setGpioState(27, false);
    MockHardware::setGpioState(28, false);
    mockTriggerSystemUpdate();
    
    // Should restore to default state
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", active_panel);
    TEST_ASSERT_EQUAL_STRING("Day", active_theme);
    
    for (int i = 0; i < 4; i++) {
        TEST_ASSERT_FALSE(triggers_active[i]);
    }
    
    test.ValidateExpectedState(ExpectedStates::OIL_PANEL_DAY);
}

// =================================================================
// SENSOR INTEGRATION TESTS
// =================================================================

void test_integration_sensor_and_trigger_system(void) {
    // Test sensors working alongside trigger system
    TriggerScenarioTest test;
    test.SetupScenario("Integration: Sensors + Triggers");
    
    mockSystemInitialization();
    mockSplashCompletion();
    
    // Set sensor values
    MockHardware::simulateAdcReading(34, 2000); // Oil pressure
    MockHardware::simulateAdcReading(35, 1500); // Oil temperature
    
    // System should be on oil panel with sensor readings
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", active_panel);
    
    // Activate trigger while sensors are active
    MockHardware::setGpioState(25, true);
    mockTriggerSystemUpdate();
    TEST_ASSERT_EQUAL_STRING("KeyPanel", active_panel);
    
    // Deactivate trigger - should return to oil panel with sensors
    MockHardware::setGpioState(25, false);
    mockTriggerSystemUpdate();
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", active_panel);
    
    // Sensor readings should remain valid
    uint16_t pressure_adc = MockHardware::getAdcReading(34);
    uint16_t temp_adc = MockHardware::getAdcReading(35);
    TEST_ASSERT_EQUAL(2000, pressure_adc);
    TEST_ASSERT_EQUAL(1500, temp_adc);
    
    test.ValidateExpectedState(ExpectedStates::OIL_PANEL_DAY);
}

// =================================================================
// LONG RUNNING INTEGRATION TESTS
// =================================================================

void test_integration_long_running_stability(void) {
    // Test system stability over extended operation
    TriggerScenarioTest test;
    test.SetupScenario("Integration: Long Running Stability");
    
    mockSystemInitialization();
    mockSplashCompletion();
    
    // Simulate extended operation with various trigger patterns
    for (int cycle = 0; cycle < 50; cycle++) {
        // Vary trigger patterns
        bool activate_key = (cycle % 3 == 0);
        bool activate_lock = (cycle % 5 == 0);
        bool activate_lights = (cycle % 7 == 0);
        
        MockHardware::setGpioState(25, activate_key);
        MockHardware::setGpioState(27, activate_lock);
        MockHardware::setGpioState(28, activate_lights);
        
        mockTriggerSystemUpdate();
        
        // System should remain stable
        TEST_ASSERT_TRUE(strlen(active_panel) > 0);
        TEST_ASSERT_TRUE(strlen(active_theme) > 0);
        
        // Panel should be appropriate for triggers
        if (activate_key) {
            TEST_ASSERT_EQUAL_STRING("KeyPanel", active_panel);
        } else if (activate_lock) {
            TEST_ASSERT_EQUAL_STRING("LockPanel", active_panel);
        } else {
            TEST_ASSERT_EQUAL_STRING("OemOilPanel", active_panel);
        }
        
        // Theme should be appropriate
        if (activate_lights) {
            TEST_ASSERT_EQUAL_STRING("Night", active_theme);
        } else {
            TEST_ASSERT_EQUAL_STRING("Day", active_theme);
        }
    }
    
    // Reset to clean state
    MockHardware::reset();
    mockTriggerSystemUpdate();
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", active_panel);
    TEST_ASSERT_EQUAL_STRING("Day", active_theme);
}

void test_integration_rapid_state_changes(void) {
    // Test rapid state changes integration
    TriggerScenarioTest test;
    test.SetupScenario("Integration: Rapid State Changes");
    
    mockSystemInitialization();
    mockSplashCompletion();
    
    // Perform rapid trigger changes
    for (int i = 0; i < 20; i++) {
        // Alternate between different triggers rapidly
        MockHardware::setGpioState(25, i % 2 == 0);        // key_present
        MockHardware::setGpioState(26, i % 3 == 0);        // key_not_present
        MockHardware::setGpioState(27, i % 4 == 0);        // lock_state
        MockHardware::setGpioState(28, i % 5 == 0);        // lights_state
        
        mockTriggerSystemUpdate();
        
        // System should handle rapid changes gracefully
        TEST_ASSERT_TRUE(strlen(active_panel) > 0);
        TEST_ASSERT_TRUE(strlen(active_theme) > 0);
    }
    
    // Final state should be stable
    MockHardware::reset();
    mockTriggerSystemUpdate();
    test.ValidateExpectedState(ExpectedStates::OIL_PANEL_DAY);
}

// =================================================================
// SYSTEM RECOVERY INTEGRATION TESTS
// =================================================================

void test_integration_system_recovery(void) {
    // Test system recovery from various fault conditions
    TriggerScenarioTest test;
    test.SetupScenario("Integration: System Recovery");
    
    mockSystemInitialization();
    mockSplashCompletion();
    
    // Simulate various fault recovery scenarios
    
    // Scenario 1: All triggers active simultaneously (system overload)
    MockHardware::setGpioState(25, true);
    MockHardware::setGpioState(26, true);
    MockHardware::setGpioState(27, true);
    MockHardware::setGpioState(28, true);
    mockTriggerSystemUpdate();
    
    // System should handle gracefully (highest priority wins)
    TEST_ASSERT_EQUAL_STRING("KeyPanel", active_panel);
    TEST_ASSERT_EQUAL_STRING("Night", active_theme);
    
    // Scenario 2: Rapid recovery to normal state
    MockHardware::reset();
    mockTriggerSystemUpdate();
    
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", active_panel);
    TEST_ASSERT_EQUAL_STRING("Day", active_theme);
    
    // Scenario 3: Verify system is fully operational
    MockHardware::setGpioState(27, true);
    mockTriggerSystemUpdate();
    TEST_ASSERT_EQUAL_STRING("LockPanel", active_panel);
    
    MockHardware::setGpioState(27, false);
    mockTriggerSystemUpdate();
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", active_panel);
    
    test.ValidateExpectedState(ExpectedStates::OIL_PANEL_DAY);
}

// Note: PlatformIO will automatically discover and run test_ functions