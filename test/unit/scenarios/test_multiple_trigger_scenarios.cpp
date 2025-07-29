#include <unity.h>
#include "mock_utilities.h"
#include "mock_managers.h"
#include "mock_types.h"

// S3.1: Priority Override - Key over Lock
void test_S3_1_priority_override_key_over_lock(void) {
    InitializeTriggersFromGpio();

    // Activate lock trigger first
    SetTrigger(TRIGGER_LOCK, true);
    TEST_ASSERT_EQUAL(PANEL_LOCK, GetCurrentPanel());

    // Activate key present trigger - should override lock
    SetTrigger(TRIGGER_KEY_PRESENT, true);
    TEST_ASSERT_EQUAL(PANEL_KEY, GetCurrentPanel());
    TEST_ASSERT_TRUE(IsKeyPresent());

    // Deactivate key present - should return to lock panel
    SetTrigger(TRIGGER_KEY_PRESENT, false);
    TEST_ASSERT_EQUAL(PANEL_LOCK, GetCurrentPanel());

    // Cleanup
    SetTrigger(TRIGGER_LOCK, false);
}

// S3.2: Key Present vs Key Not Present
void test_S3_2_key_present_vs_key_not_present(void) {
    InitializeTriggersFromGpio();

    // Activate key not present first
    SetTrigger(TRIGGER_KEY_NOT_PRESENT, true);
    TEST_ASSERT_EQUAL(PANEL_KEY, GetCurrentPanel());
    TEST_ASSERT_FALSE(IsKeyPresent());

    // Activate key present - should override key not present
    SetTrigger(TRIGGER_KEY_PRESENT, true);
    TEST_ASSERT_EQUAL(PANEL_KEY, GetCurrentPanel());
    TEST_ASSERT_TRUE(IsKeyPresent());

    // Deactivate key present - should return to key not present state
    SetTrigger(TRIGGER_KEY_PRESENT, false);
    TEST_ASSERT_EQUAL(PANEL_KEY, GetCurrentPanel());
    TEST_ASSERT_FALSE(IsKeyPresent());

    // Cleanup
    SetTrigger(TRIGGER_KEY_NOT_PRESENT, false);
}

// S3.2: Intermediate State Validation
void test_S3_2_intermediate_state_validation(void) {
    InitializeTriggersFromGpio();

    // Set up complex trigger sequence
    SetTrigger(TRIGGER_LOCK, true);
    TEST_ASSERT_EQUAL(PANEL_LOCK, GetCurrentPanel());

    SetTrigger(TRIGGER_KEY_PRESENT, true);
    TEST_ASSERT_EQUAL(PANEL_KEY, GetCurrentPanel());
    TEST_ASSERT_TRUE(IsKeyPresent());

    SetTrigger(TRIGGER_THEME, true);
    TEST_ASSERT_EQUAL(PANEL_KEY, GetCurrentPanel());
    TEST_ASSERT_TRUE(IsNightThemeActive());

    // Remove triggers in reverse order
    SetTrigger(TRIGGER_THEME, false);
    TEST_ASSERT_EQUAL(PANEL_KEY, GetCurrentPanel());
    TEST_ASSERT_FALSE(IsNightThemeActive());

    SetTrigger(TRIGGER_KEY_PRESENT, false);
    TEST_ASSERT_EQUAL(PANEL_LOCK, GetCurrentPanel());

    SetTrigger(TRIGGER_LOCK, false);
    TEST_ASSERT_EQUAL(PANEL_OIL, GetCurrentPanel());
}
