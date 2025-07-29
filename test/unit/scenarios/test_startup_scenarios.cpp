#include <unity.h>
#include "mock_utilities.h"
#include "mock_managers.h"
#include "mock_types.h"

// External declarations for mock states
extern bool mock_key_present_active;
extern bool mock_key_not_present_active;
extern bool mock_lock_active;
extern bool mock_theme_active;

// S1.1: Clean System Startup
void test_S1_1_clean_system_startup(void) {
    // Initial state: Fresh boot, no triggers active
    TEST_ASSERT_FALSE(mock_key_present_active);
    TEST_ASSERT_FALSE(mock_key_not_present_active);
    TEST_ASSERT_FALSE(mock_lock_active);
    TEST_ASSERT_FALSE(mock_theme_active);

    // Simulate system startup
    InitializeTriggersFromGpio();
    
    // Validate: No trigger events during splash
    TEST_ASSERT_FALSE(IsTriggerActive(TRIGGER_KEY_PRESENT));
    TEST_ASSERT_FALSE(IsTriggerActive(TRIGGER_KEY_NOT_PRESENT));
    TEST_ASSERT_FALSE(IsTriggerActive(TRIGGER_LOCK));
    TEST_ASSERT_FALSE(IsTriggerActive(TRIGGER_THEME));

    // Should default to Oil panel with Day theme
    TEST_ASSERT_EQUAL(PANEL_OIL, GetCurrentPanel());
    TEST_ASSERT_FALSE(IsNightThemeActive());
}

// S1.2: Startup with Key Present
void test_S1_2_startup_with_key_present(void) {
    // Initial state: Boot with key_present pin HIGH
    mock_key_present_active = true;

    // Simulate system startup
    InitializeTriggersFromGpio();

    // Validate initial states
    TEST_ASSERT_TRUE(IsTriggerActive(TRIGGER_KEY_PRESENT));
    TEST_ASSERT_FALSE(IsTriggerActive(TRIGGER_KEY_NOT_PRESENT));
    TEST_ASSERT_FALSE(IsTriggerActive(TRIGGER_LOCK));
    TEST_ASSERT_FALSE(IsTriggerActive(TRIGGER_THEME));

    // Should show Key panel with green indicator
    TEST_ASSERT_EQUAL(PANEL_KEY, GetCurrentPanel());
    TEST_ASSERT_TRUE(IsKeyPresent());
}

// S1.3: Startup with Key Not Present
void test_S1_3_startup_with_key_not_present(void) {
    // Initial state: Boot with key_not_present pin HIGH
    mock_key_not_present_active = true;

    // Simulate system startup
    InitializeTriggersFromGpio();

    // Validate initial states
    TEST_ASSERT_FALSE(IsTriggerActive(TRIGGER_KEY_PRESENT));
    TEST_ASSERT_TRUE(IsTriggerActive(TRIGGER_KEY_NOT_PRESENT));
    TEST_ASSERT_FALSE(IsTriggerActive(TRIGGER_LOCK));
    TEST_ASSERT_FALSE(IsTriggerActive(TRIGGER_THEME));

    // Should show Key panel with red indicator
    TEST_ASSERT_EQUAL(PANEL_KEY, GetCurrentPanel());
    TEST_ASSERT_FALSE(IsKeyPresent());
}

// S1.4: Startup with Lock Active
void test_S1_4_startup_with_lock_active(void) {
    // Initial state: Boot with lock pin HIGH
    mock_lock_active = true;

    // Simulate system startup
    InitializeTriggersFromGpio();

    // Validate initial states
    TEST_ASSERT_FALSE(IsTriggerActive(TRIGGER_KEY_PRESENT));
    TEST_ASSERT_FALSE(IsTriggerActive(TRIGGER_KEY_NOT_PRESENT));
    TEST_ASSERT_TRUE(IsTriggerActive(TRIGGER_LOCK));
    TEST_ASSERT_FALSE(IsTriggerActive(TRIGGER_THEME));

    // Should show Lock panel
    TEST_ASSERT_EQUAL(PANEL_LOCK, GetCurrentPanel());
    TEST_ASSERT_TRUE(IsLockActive());
}

// S1.5: Startup with Theme Trigger
void test_S1_5_startup_with_theme_trigger(void) {
    // Initial state: Boot with theme pin HIGH
    mock_theme_active = true;

    // Simulate system startup
    InitializeTriggersFromGpio();

    // Validate initial states
    TEST_ASSERT_FALSE(IsTriggerActive(TRIGGER_KEY_PRESENT));
    TEST_ASSERT_FALSE(IsTriggerActive(TRIGGER_KEY_NOT_PRESENT));
    TEST_ASSERT_FALSE(IsTriggerActive(TRIGGER_LOCK));
    TEST_ASSERT_TRUE(IsTriggerActive(TRIGGER_THEME));

    // Should show default panel but with night theme
    TEST_ASSERT_EQUAL(PANEL_OIL, GetCurrentPanel());
    TEST_ASSERT_TRUE(IsNightThemeActive());
}
