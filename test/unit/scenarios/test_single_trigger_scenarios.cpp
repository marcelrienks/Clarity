#include <unity.h>
#include "mock_utilities.h"
#include "mock_managers.h"
#include "mock_types.h"

// External declarations for mock states
extern bool mock_key_present_active;
extern bool mock_key_not_present_active;
extern bool mock_lock_active;
extern bool mock_theme_active;

// S2.2: Lock Trigger
void test_S2_2_lock_trigger(void) {
    // Initial state: Clean boot
    InitializeTriggersFromGpio();
    TEST_ASSERT_EQUAL(PANEL_OIL, GetCurrentPanel());

    // Activate lock trigger
    SetTrigger(TRIGGER_LOCK, true);
    TEST_ASSERT_TRUE(IsTriggerActive(TRIGGER_LOCK));
    TEST_ASSERT_EQUAL(PANEL_LOCK, GetCurrentPanel());

    // Deactivate lock trigger
    SetTrigger(TRIGGER_LOCK, false);
    TEST_ASSERT_FALSE(IsTriggerActive(TRIGGER_LOCK));
    TEST_ASSERT_EQUAL(PANEL_OIL, GetCurrentPanel());
}

// S2.3: Key Present Trigger
void test_S2_3_key_present_trigger(void) {
    InitializeTriggersFromGpio();
    TEST_ASSERT_EQUAL(PANEL_OIL, GetCurrentPanel());

    // Activate key present trigger
    SetTrigger(TRIGGER_KEY_PRESENT, true);
    TEST_ASSERT_TRUE(IsTriggerActive(TRIGGER_KEY_PRESENT));
    TEST_ASSERT_EQUAL(PANEL_KEY, GetCurrentPanel());
    TEST_ASSERT_TRUE(IsKeyPresent());

    // Deactivate key present trigger
    SetTrigger(TRIGGER_KEY_PRESENT, false);
    TEST_ASSERT_FALSE(IsTriggerActive(TRIGGER_KEY_PRESENT));
    TEST_ASSERT_EQUAL(PANEL_OIL, GetCurrentPanel());
}

// S2.4: Key Not Present Trigger
void test_S2_4_key_not_present_trigger(void) {
    InitializeTriggersFromGpio();
    TEST_ASSERT_EQUAL(PANEL_OIL, GetCurrentPanel());

    // Activate key not present trigger
    SetTrigger(TRIGGER_KEY_NOT_PRESENT, true);
    TEST_ASSERT_TRUE(IsTriggerActive(TRIGGER_KEY_NOT_PRESENT));
    TEST_ASSERT_EQUAL(PANEL_KEY, GetCurrentPanel());
    TEST_ASSERT_FALSE(IsKeyPresent());

    // Deactivate key not present trigger
    SetTrigger(TRIGGER_KEY_NOT_PRESENT, false);
    TEST_ASSERT_FALSE(IsTriggerActive(TRIGGER_KEY_NOT_PRESENT));
    TEST_ASSERT_EQUAL(PANEL_OIL, GetCurrentPanel());
}
