#include <unity.h>
#include "test_utilities.h"

// Mock includes to avoid ESP32 dependencies
extern "C" {
    // Mock Arduino functions for unit testing
    void digitalWrite(uint8_t pin, uint8_t val) {}
    int digitalRead(uint8_t pin) { return MockHardware::getGpioState(pin) ? 1 : 0; }
    int analogRead(uint8_t pin) { return MockHardware::getAdcReading(pin); }
    unsigned long millis() { return 1000; } // Mock timestamp
    void delay(unsigned long ms) {}
}

// Note: setUp() and tearDown() are defined in test_main.cpp

// =================================================================
// 1. SYSTEM STARTUP SCENARIOS (S1.1-S1.5)
// =================================================================

void test_S1_1_clean_system_startup(void) {
    TriggerScenarioTest test;
    test.SetupScenario("S1.1: Clean System Startup");
    
    // Apply clean startup scenario (no triggers)
    auto events = TestScenarios::cleanStartup();
    test.ApplyTriggerSequence(events);
    
    // Validate expected state: Oil panel with Day theme
    test.ValidateExpectedState(ExpectedStates::OIL_PANEL_DAY);
    
    TEST_ASSERT_PANEL_LOADED("OemOilPanel");
    TEST_ASSERT_THEME_APPLIED("Day");
}

void test_S1_2_startup_with_key_present(void) {
    TriggerScenarioTest test;
    test.SetupScenario("S1.2: Startup with Key Present Active");
    
    // Apply startup with key present
    auto events = TestScenarios::startupWithKeyPresent();
    test.ApplyTriggerSequence(events);
    
    // Validate expected state: Key panel (green) with current theme
    test.ValidateExpectedState(ExpectedStates::KEY_PANEL_GREEN);
    
    TEST_ASSERT_PANEL_LOADED("KeyPanel");
    TEST_ASSERT_TRIGGER_STATE("key_present", true);
}

void test_S1_3_startup_with_key_not_present(void) {
    TriggerScenarioTest test;
    test.SetupScenario("S1.3: Startup with Key Not Present Active");
    
    auto events = TestScenarios::startupWithKeyNotPresent();
    test.ApplyTriggerSequence(events);
    
    test.ValidateExpectedState(ExpectedStates::KEY_PANEL_RED);
    
    TEST_ASSERT_PANEL_LOADED("KeyPanel");
    TEST_ASSERT_TRIGGER_STATE("key_not_present", true);
}

void test_S1_4_startup_with_lock_active(void) {
    TriggerScenarioTest test;
    test.SetupScenario("S1.4: Startup with Lock Active");
    
    auto events = TestScenarios::startupWithLock();
    test.ApplyTriggerSequence(events);
    
    test.ValidateExpectedState(ExpectedStates::LOCK_PANEL);
    
    TEST_ASSERT_PANEL_LOADED("LockPanel");
    TEST_ASSERT_TRIGGER_STATE("lock_state", true);
}

void test_S1_5_startup_with_theme_trigger(void) {
    TriggerScenarioTest test;
    test.SetupScenario("S1.5: Startup with Theme Trigger Active");
    
    auto events = TestScenarios::startupWithTheme();
    test.ApplyTriggerSequence(events);
    
    test.ValidateExpectedState(ExpectedStates::OIL_PANEL_NIGHT);
    
    TEST_ASSERT_PANEL_LOADED("OemOilPanel");
    TEST_ASSERT_THEME_APPLIED("Night");
    TEST_ASSERT_TRIGGER_STATE("lights_state", true);
}

// =================================================================
// 2. SINGLE TRIGGER SCENARIOS (S2.1-S2.4)
// =================================================================

void test_S2_2_lock_trigger(void) {
    TriggerScenarioTest test;
    test.SetupScenario("S2.2: Lock Trigger");
    
    auto events = TestScenarios::lockTrigger();
    test.ApplyTriggerSequence(events);
    
    // After sequence, lock should be deactivated, back to oil panel
    test.ValidateExpectedState(ExpectedStates::OIL_PANEL_DAY);
    
    TEST_ASSERT_PANEL_LOADED("OemOilPanel");
    TEST_ASSERT_TRIGGER_STATE("lock_state", false);
}

void test_S2_3_key_present_trigger(void) {
    TriggerScenarioTest test;
    test.SetupScenario("S2.3: Key Present Trigger");
    
    auto events = TestScenarios::keyPresentTrigger();
    test.ApplyTriggerSequence(events);
    
    // After sequence, key should be removed, back to oil panel
    test.ValidateExpectedState(ExpectedStates::OIL_PANEL_DAY);
    
    TEST_ASSERT_PANEL_LOADED("OemOilPanel");
    TEST_ASSERT_TRIGGER_STATE("key_present", false);
}

void test_S2_4_key_not_present_trigger(void) {
    TriggerScenarioTest test;
    test.SetupScenario("S2.4: Key Not Present Trigger");
    
    auto events = TestScenarios::keyNotPresentTrigger();
    test.ApplyTriggerSequence(events);
    
    // After sequence, key trigger cleared, back to oil panel
    test.ValidateExpectedState(ExpectedStates::OIL_PANEL_DAY);
    
    TEST_ASSERT_PANEL_LOADED("OemOilPanel");
    TEST_ASSERT_TRIGGER_STATE("key_not_present", false);
}

// =================================================================
// 3. MULTIPLE TRIGGER SCENARIOS (S3.1-S3.5)
// =================================================================

void test_S3_1_priority_override_key_over_lock(void) {
    TriggerScenarioTest test;
    test.SetupScenario("S3.1: Priority Override - Key Present Over Lock");
    
    auto events = TestScenarios::priorityOverrideKeyOverLock();
    test.ApplyTriggerSequence(events);
    
    // Final state should be oil panel (all triggers deactivated)
    test.ValidateExpectedState(ExpectedStates::OIL_PANEL_DAY);
    
    TEST_ASSERT_PANEL_LOADED("OemOilPanel");
    TEST_ASSERT_TRIGGER_STATE("key_present", false);
    TEST_ASSERT_TRIGGER_STATE("lock_state", false);
}

void test_S3_2_key_present_vs_key_not_present(void) {
    TriggerScenarioTest test;
    test.SetupScenario("S3.2: Key Present vs Key Not Present");
    
    auto events = TestScenarios::keyPresentVsKeyNotPresent();
    test.ApplyTriggerSequence(events);
    
    // Final state should be oil panel (all key triggers deactivated)
    test.ValidateExpectedState(ExpectedStates::OIL_PANEL_DAY);
    
    TEST_ASSERT_PANEL_LOADED("OemOilPanel");
    TEST_ASSERT_TRIGGER_STATE("key_present", false);
    TEST_ASSERT_TRIGGER_STATE("key_not_present", false);
}

void test_S3_2_intermediate_state_validation(void) {
    TriggerScenarioTest test;
    test.SetupScenario("S3.2: Key Present vs Key Not Present - Intermediate");
    
    // Test intermediate states during the sequence
    MockHardware::reset();
    
    // Step 1: Key present first
    setGpioAndUpdate(25, true); // key_present
    TEST_ASSERT_PANEL_LOADED("KeyPanel");
    TEST_ASSERT_TRIGGER_STATE("key_present", true);
    
    // Step 2: Key not present overrides (same priority FIFO)
    setGpioAndUpdate(26, true); // key_not_present
    TEST_ASSERT_PANEL_LOADED("KeyPanel");
    TEST_ASSERT_TRIGGER_STATE("key_not_present", true);
    
    // Step 3: Remove key present (not present should remain)
    setGpioAndUpdate(25, false);
    TEST_ASSERT_PANEL_LOADED("KeyPanel");
    TEST_ASSERT_TRIGGER_STATE("key_not_present", true);
    TEST_ASSERT_TRIGGER_STATE("key_present", false);
}

// =================================================================
// 4. EDGE CASE SCENARIOS (S4.1-S4.5)
// =================================================================

void test_S4_1_rapid_toggle_single_trigger(void) {
    TriggerScenarioTest test;
    test.SetupScenario("S4.1: Rapid Toggle Single Trigger");
    
    auto events = TestScenarios::rapidToggleSingle();
    test.ApplyTriggerSequence(events);
    
    // Final state should be key present active
    test.ValidateExpectedState(ExpectedStates::KEY_PANEL_GREEN);
    
    TEST_ASSERT_PANEL_LOADED("KeyPanel");
    TEST_ASSERT_TRIGGER_STATE("key_present", true);
}

void test_S4_2_rapid_toggle_multiple_triggers(void) {
    TriggerScenarioTest test;
    test.SetupScenario("S4.2: Rapid Toggle Multiple Triggers");
    
    auto events = TestScenarios::rapidToggleMultiple();
    test.ApplyTriggerSequence(events);
    
    // Final state should be key not present active (last in FIFO)
    test.ValidateExpectedState(ExpectedStates::KEY_PANEL_RED);
    
    TEST_ASSERT_PANEL_LOADED("KeyPanel");
    TEST_ASSERT_TRIGGER_STATE("key_not_present", true);
    TEST_ASSERT_TRIGGER_STATE("key_present", false);
}

void test_S4_5_invalid_trigger_combinations(void) {
    TriggerScenarioTest test;
    test.SetupScenario("S4.5: Invalid Trigger Combinations");
    
    auto events = TestScenarios::invalidTriggerCombinations();
    test.ApplyTriggerSequence(events);
    
    // System should handle invalid state (both keys active)
    // Last trigger wins (FIFO behavior)
    TEST_ASSERT_PANEL_LOADED("KeyPanel");
    TEST_ASSERT_TRIGGER_STATE("key_present", true);
    TEST_ASSERT_TRIGGER_STATE("key_not_present", true);
}

void test_S4_4_simultaneous_deactivation(void) {
    TriggerScenarioTest test;
    test.SetupScenario("S4.4: Simultaneous Deactivation");
    
    auto events = TestScenarios::simultaneousDeactivation();
    test.ApplyTriggerSequence(events);
    
    // All triggers deactivated, should restore to default
    test.ValidateExpectedState(ExpectedStates::OIL_PANEL_DAY);
    
    TEST_ASSERT_PANEL_LOADED("OemOilPanel");
    TEST_ASSERT_THEME_APPLIED("Day");
    TEST_ASSERT_TRIGGER_STATE("key_present", false);
    TEST_ASSERT_TRIGGER_STATE("lock_state", false);
    TEST_ASSERT_TRIGGER_STATE("lights_state", false);
}

// =================================================================
// 5. PERFORMANCE TEST SCENARIOS
// =================================================================

void test_S5_1_high_frequency_trigger_events(void) {
    TriggerScenarioTest test;
    test.SetupScenario("S5.1: High Frequency Trigger Events");
    
    // Simulate 100 trigger events
    std::vector<TriggerEvent> events;
    for (int i = 0; i < 100; i++) {
        events.push_back({"key_present", i % 2 == 0, static_cast<uint32_t>(i * 10)});
    }
    
    measureResponseTime([&]() {
        test.ApplyTriggerSequence(events);
    });
    
    // System should handle high frequency events without issues
    // Final state should reflect last event (odd index = false)
    TEST_ASSERT_TRIGGER_STATE("key_present", false);
}

void test_S5_3_panel_load_performance(void) {
    TriggerScenarioTest test;
    test.SetupScenario("S5.3: Panel Load Performance");
    
    // Rapid panel switching
    std::vector<TriggerEvent> events = {
        {"key_present", true, 100},
        {"key_present", false, 200},
        {"lock_state", true, 300},
        {"lock_state", false, 400},
        {"key_not_present", true, 500},
        {"key_not_present", false, 600}
    };
    
    measureResponseTime([&]() {
        test.ApplyTriggerSequence(events);
    });
    
    // Should end at oil panel
    TEST_ASSERT_PANEL_LOADED("OemOilPanel");
}

// =================================================================
// 6. RESTORATION CHAIN VERIFICATION
// =================================================================

void test_complex_restoration_chain(void) {
    TriggerScenarioTest test;
    test.SetupScenario("Complex Restoration Chain");
    
    MockHardware::reset();
    
    // Build complex trigger stack
    setGpioAndUpdate(27, true); // lock
    TEST_ASSERT_PANEL_LOADED("LockPanel");
    
    setGpioAndUpdate(26, true); // key_not_present (higher priority)
    TEST_ASSERT_PANEL_LOADED("KeyPanel");
    
    setGpioAndUpdate(25, true); // key_present (same priority, FIFO)
    TEST_ASSERT_PANEL_LOADED("KeyPanel");
    TEST_ASSERT_TRIGGER_STATE("key_present", true);
    
    // Unwind the stack
    setGpioAndUpdate(25, false); // Remove key_present -> key_not_present
    TEST_ASSERT_PANEL_LOADED("KeyPanel");
    TEST_ASSERT_TRIGGER_STATE("key_not_present", true);
    
    setGpioAndUpdate(26, false); // Remove key_not_present -> lock
    TEST_ASSERT_PANEL_LOADED("LockPanel");
    TEST_ASSERT_TRIGGER_STATE("lock_state", true);
    
    setGpioAndUpdate(27, false); // Remove lock -> oil panel
    TEST_ASSERT_PANEL_LOADED("OemOilPanel");
}

// Note: PlatformIO will automatically discover and run test_ functions