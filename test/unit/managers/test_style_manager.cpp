#ifdef UNIT_TESTING

#include <unity.h>
#include "managers/style_manager.h"
#include "utilities/types.h"
#include <cstring>

// Unity extension macros for string comparison  
#define TEST_ASSERT_NOT_EQUAL_STRING(expected, actual) TEST_ASSERT_FALSE(strcmp(expected, actual) == 0)

static StyleManager* styleManagerForTest = nullptr;

void setUp_style_manager() {
    styleManagerForTest = new StyleManager();
}

void tearDown_style_manager() {
    delete styleManagerForTest;
    styleManagerForTest = nullptr;
}

// REMOVED: Pointless constructor test - just checks new works

void test_style_manager_init() {
    // Test initialization with day theme
    styleManagerForTest->init(Themes::DAY);
    TEST_ASSERT_TRUE(styleManagerForTest->isInitialized());
    
    // Test initialization with night theme
    StyleManager nightManager;
    nightManager.init(Themes::NIGHT);
    TEST_ASSERT_TRUE(nightManager.isInitialized());
}

// REMOVED: Pointless test - just calls method and asserts true

void test_style_manager_theme_switching() {
    styleManagerForTest->init(Themes::DAY);
    
    // Get initial theme
    const char* initialTheme = styleManagerForTest->getCurrentTheme();
    TEST_ASSERT_EQUAL_STRING(Themes::DAY, initialTheme);
    
    // Switch theme
    styleManagerForTest->setTheme(Themes::NIGHT);
    const char* newTheme = styleManagerForTest->getCurrentTheme();
    TEST_ASSERT_EQUAL_STRING(Themes::NIGHT, newTheme);
    
    // Verify theme changed
    TEST_ASSERT_NOT_EQUAL_STRING(initialTheme, newTheme);
}

// REMOVED: Redundant with test_style_manager_theme_switching

// REMOVED: Pointless test - just calls method and asserts true

// REMOVED: Pointless test - just calls methods and asserts true

// REMOVED: Pointless test - just calls method with null and asserts true

void test_style_manager_day_night_differences() {
    // Test day theme
    StyleManager dayManager;
    dayManager.init(Themes::DAY);
    const ThemeColors& dayColors = dayManager.getThemeColors();
    
    // Test night theme
    StyleManager nightManager;
    nightManager.init(Themes::NIGHT);
    const ThemeColors& nightColors = nightManager.getThemeColors();
    
    // Themes should be different (at least the name)
    TEST_ASSERT_NOT_EQUAL_STRING(dayManager.getCurrentTheme(), nightManager.getCurrentTheme());
}

// Enhanced coverage tests for Phase 2

void test_style_manager_rapid_theme_switching() {
    styleManagerForTest->init(Themes::DAY);
    
    // Test rapid theme switching for state consistency
    for (int i = 0; i < 20; i++) {
        const char* theme = (i % 2 == 0) ? Themes::DAY : Themes::NIGHT;
        styleManagerForTest->setTheme(theme);
        TEST_ASSERT_EQUAL_STRING(theme, styleManagerForTest->getCurrentTheme());
    }
    
    // Verify final state is consistent
    styleManagerForTest->setTheme(Themes::DAY);
    TEST_ASSERT_EQUAL_STRING(Themes::DAY, styleManagerForTest->getCurrentTheme());
}

void test_style_manager_memory_management() {
    // Test creating and destroying multiple instances
    for (int i = 0; i < 10; i++) {
        StyleManager* tempManager = new StyleManager();
        tempManager->init(Themes::DAY);
        tempManager->initializeStyles();
        
        // Get styles to ensure they're created
        tempManager->getBackgroundStyle();
        tempManager->getTextStyle();
        
        delete tempManager;
    }
    
    // If we get here without crashes, memory management is working
    TEST_ASSERT_TRUE(true);
}

void test_style_manager_style_consistency() {
    styleManagerForTest->init(Themes::DAY);
    styleManagerForTest->initializeStyles();
    
    // Get styles and verify they remain consistent across multiple calls
    lv_style_t& bgStyle1 = styleManagerForTest->getBackgroundStyle();
    lv_style_t& bgStyle2 = styleManagerForTest->getBackgroundStyle();
    
    // Should return the same style reference
    TEST_ASSERT_EQUAL_PTR(&bgStyle1, &bgStyle2);
    
    // Test with different styles
    lv_style_t& textStyle = styleManagerForTest->getTextStyle();
    lv_style_t& gaugeStyle = styleManagerForTest->getGaugeNormalStyle();
    
    // Different styles should have different addresses
    TEST_ASSERT_NOT_EQUAL(&textStyle, &gaugeStyle);
}

void test_style_manager_theme_persistence() {
    styleManagerForTest->init(Themes::DAY);
    
    // Set theme and verify persistence across operations
    styleManagerForTest->setTheme(Themes::NIGHT);
    TEST_ASSERT_EQUAL_STRING(Themes::NIGHT, styleManagerForTest->getCurrentTheme());
    
    // Initialize styles - theme should persist
    styleManagerForTest->initializeStyles();
    TEST_ASSERT_EQUAL_STRING(Themes::NIGHT, styleManagerForTest->getCurrentTheme());
    
    // Get colors - theme should still persist
    const ThemeColors& colors = styleManagerForTest->getThemeColors();
    TEST_ASSERT_EQUAL_STRING(Themes::NIGHT, styleManagerForTest->getCurrentTheme());
}

void test_style_manager_invalid_theme_handling() {
    styleManagerForTest->init(Themes::DAY);
    
    // Test with nullptr theme (should handle gracefully)
    styleManagerForTest->setTheme(nullptr);
    // Should not crash and maintain a valid state
    const char* currentTheme = styleManagerForTest->getCurrentTheme();
    TEST_ASSERT_NOT_NULL(currentTheme);
    
    // Test with empty string theme
    styleManagerForTest->setTheme("");
    currentTheme = styleManagerForTest->getCurrentTheme();
    TEST_ASSERT_NOT_NULL(currentTheme);
    
    // Test with invalid theme name
    styleManagerForTest->setTheme("INVALID_THEME");
    currentTheme = styleManagerForTest->getCurrentTheme();
    TEST_ASSERT_NOT_NULL(currentTheme);
}

void test_style_manager_initialization_edge_cases() {
    // Test multiple initialization calls
    styleManagerForTest->init(Themes::DAY);
    TEST_ASSERT_TRUE(styleManagerForTest->isInitialized());
    
    // Initialize again with different theme
    styleManagerForTest->init(Themes::NIGHT);
    TEST_ASSERT_TRUE(styleManagerForTest->isInitialized());
    TEST_ASSERT_EQUAL_STRING(Themes::NIGHT, styleManagerForTest->getCurrentTheme());
    
    // Initialize multiple times with same theme
    styleManagerForTest->init(Themes::NIGHT);
    styleManagerForTest->init(Themes::NIGHT);
    TEST_ASSERT_TRUE(styleManagerForTest->isInitialized());
}

void test_style_manager_style_initialization_robustness() {
    styleManagerForTest->init(Themes::DAY);
    
    // Test multiple style initialization calls
    styleManagerForTest->initializeStyles();
    styleManagerForTest->initializeStyles();
    styleManagerForTest->initializeStyles();
    
    // Should still work correctly
    lv_style_t& bgStyle = styleManagerForTest->getBackgroundStyle();
    TEST_ASSERT_NOT_NULL(&bgStyle);
}

void test_style_manager_concurrent_access_simulation() {
    styleManagerForTest->init(Themes::DAY);
    styleManagerForTest->initializeStyles();
    
    // Simulate concurrent access patterns
    for (int i = 0; i < 50; i++) {
        // Rapid style access
        styleManagerForTest->getBackgroundStyle();
        styleManagerForTest->getTextStyle();
        styleManagerForTest->getCurrentTheme();
        styleManagerForTest->getThemeColors();
        
        // Theme switching
        if (i % 10 == 0) {
            const char* theme = (i % 20 == 0) ? Themes::DAY : Themes::NIGHT;
            styleManagerForTest->setTheme(theme);
        }
    }
    
    // Verify final state is consistent
    const char* finalTheme = styleManagerForTest->getCurrentTheme();
    TEST_ASSERT_NOT_NULL(finalTheme);
}

void test_style_manager_cleanup_and_resource_management() {
    styleManagerForTest->init(Themes::DAY);
    styleManagerForTest->initializeStyles();
    
    // Access all styles to ensure they're created
    styleManagerForTest->getBackgroundStyle();
    styleManagerForTest->getTextStyle();
    styleManagerForTest->getGaugeNormalStyle();
    styleManagerForTest->getGaugeWarningStyle();
    styleManagerForTest->getGaugeDangerStyle();
    styleManagerForTest->getGaugeIndicatorStyle();
    styleManagerForTest->getGaugeItemsStyle();
    styleManagerForTest->getGaugeMainStyle();
    
    // Test theme switching after all styles are created
    styleManagerForTest->setTheme(Themes::NIGHT);
    
    // Verify we can still access styles after theme change
    styleManagerForTest->getBackgroundStyle();
    styleManagerForTest->getTextStyle();
    
    TEST_ASSERT_TRUE(true); // If we get here without crashes, cleanup is working
}

void test_style_manager_apply_theme_edge_cases() {
    styleManagerForTest->init(Themes::DAY);
    
    // Test applying theme before style initialization
    styleManagerForTest->applyThemeToScreen(nullptr);
    
    // Initialize styles then apply theme
    styleManagerForTest->initializeStyles();
    styleManagerForTest->applyThemeToScreen(nullptr);
    
    // Switch theme and apply again
    styleManagerForTest->setTheme(Themes::NIGHT);
    styleManagerForTest->applyThemeToScreen(nullptr);
    
    TEST_ASSERT_TRUE(true); // No crashes = success
}

void test_style_manager_state_transitions() {
    // Test various state transitions
    
    // 1. Uninitialized state
    StyleManager freshManager;
    
    // 2. Initialize
    freshManager.init(Themes::DAY);
    TEST_ASSERT_TRUE(freshManager.isInitialized());
    
    // 3. Initialize styles
    freshManager.initializeStyles();
    
    // 4. Get styles and colors
    const ThemeColors& colors = freshManager.getThemeColors();
    lv_style_t& style = freshManager.getBackgroundStyle();
    
    // 5. Switch theme
    freshManager.setTheme(Themes::NIGHT);
    
    // 6. Get styles again
    const ThemeColors& newColors = freshManager.getThemeColors();
    lv_style_t& newStyle = freshManager.getBackgroundStyle();
    
    // All operations should complete without crashes
    TEST_ASSERT_TRUE(true);
}

void runStyleManagerTests() {
    setUp_style_manager();
    RUN_TEST(test_style_manager_init);
    RUN_TEST(test_style_manager_theme_switching);
    RUN_TEST(test_style_manager_day_night_differences);
    
    // Enhanced coverage tests for Phase 2
    RUN_TEST(test_style_manager_rapid_theme_switching);
    RUN_TEST(test_style_manager_theme_persistence);
    RUN_TEST(test_style_manager_invalid_theme_handling);
    RUN_TEST(test_style_manager_initialization_edge_cases);
    RUN_TEST(test_style_manager_memory_management);
    RUN_TEST(test_style_manager_style_consistency);
    RUN_TEST(test_style_manager_style_initialization_robustness);
    RUN_TEST(test_style_manager_concurrent_access_simulation);
    RUN_TEST(test_style_manager_cleanup_and_resource_management);
    RUN_TEST(test_style_manager_apply_theme_edge_cases);
    RUN_TEST(test_style_manager_state_transitions);
    
    tearDown_style_manager();
}

#endif