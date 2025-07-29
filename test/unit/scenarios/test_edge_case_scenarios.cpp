#include <unity.h>
#include "mock_utilities.h"
#include "mock_managers.h"
#include "mock_types.h"

// S4.1: Rapid Toggle Single Trigger
void test_S4_1_rapid_toggle_single_trigger(void) {
    InitializeTriggersFromGpio();

    // Rapid toggle of lock trigger
    for(int i = 0; i < 10; i++) {
        SetTrigger(TRIGGER_LOCK, true);
        TEST_ASSERT_EQUAL(PANEL_LOCK, GetCurrentPanel());
        
        SetTrigger(TRIGGER_LOCK, false);
        TEST_ASSERT_EQUAL(PANEL_OIL, GetCurrentPanel());
    }
}

// S4.2: Rapid Toggle Multiple Triggers
void test_S4_2_rapid_toggle_multiple_triggers(void) {
    InitializeTriggersFromGpio();

    // Rapid alternation between key present and lock
    for(int i = 0; i < 10; i++) {
        SetTrigger(TRIGGER_KEY_PRESENT, true);
        TEST_ASSERT_EQUAL(PANEL_KEY, GetCurrentPanel());
        
        SetTrigger(TRIGGER_LOCK, true);
        TEST_ASSERT_EQUAL(PANEL_KEY, GetCurrentPanel()); // Key has priority
        
        SetTrigger(TRIGGER_KEY_PRESENT, false);
        TEST_ASSERT_EQUAL(PANEL_LOCK, GetCurrentPanel());
        
        SetTrigger(TRIGGER_LOCK, false);
        TEST_ASSERT_EQUAL(PANEL_OIL, GetCurrentPanel());
    }
}

// S4.4: Simultaneous Deactivation
void test_S4_4_simultaneous_deactivation(void) {
    InitializeTriggersFromGpio();

    // Set up multiple triggers
    SetTrigger(TRIGGER_LOCK, true);
    SetTrigger(TRIGGER_KEY_PRESENT, true);
    SetTrigger(TRIGGER_THEME, true);

    TEST_ASSERT_EQUAL(PANEL_KEY, GetCurrentPanel());
    TEST_ASSERT_TRUE(IsNightThemeActive());

    // Deactivate all simultaneously
    SetTrigger(TRIGGER_LOCK, false);
    SetTrigger(TRIGGER_KEY_PRESENT, false);
    SetTrigger(TRIGGER_THEME, false);

    TEST_ASSERT_EQUAL(PANEL_OIL, GetCurrentPanel());
    TEST_ASSERT_FALSE(IsNightThemeActive());
}

// S4.5: Invalid Trigger Combinations
void test_S4_5_invalid_trigger_combinations(void) {
    InitializeTriggersFromGpio();

    // Try to set mutually exclusive triggers
    SetTrigger(TRIGGER_KEY_PRESENT, true);
    SetTrigger(TRIGGER_KEY_NOT_PRESENT, true);

    // Key present should take precedence
    TEST_ASSERT_TRUE(IsTriggerActive(TRIGGER_KEY_PRESENT));
    TEST_ASSERT_FALSE(IsTriggerActive(TRIGGER_KEY_NOT_PRESENT));
    TEST_ASSERT_EQUAL(PANEL_KEY, GetCurrentPanel());
    TEST_ASSERT_TRUE(IsKeyPresent());

    // Cleanup
    SetTrigger(TRIGGER_KEY_PRESENT, false);
    SetTrigger(TRIGGER_KEY_NOT_PRESENT, false);
}
