#include <unity.h>
#include "managers/style_manager.h"
#include "utilities/types.h"
#include <cstring>

// Unity extension macros for string comparison  
#define TEST_ASSERT_NOT_EQUAL_STRING(expected, actual) TEST_ASSERT_FALSE(strcmp(expected, actual) == 0)

StyleManager* styleManager = nullptr;

void setUp_style_manager() {
    styleManager = new StyleManager();
}

void tearDown_style_manager() {
    delete styleManager;
    styleManager = nullptr;
}

void test_style_manager_construction() {
    // Test that manager can be created and destroyed
    TEST_ASSERT_NOT_NULL(styleManager);
}

void test_style_manager_init() {
    // Test initialization with day theme
    styleManager->init(Themes::DAY);
    TEST_ASSERT_TRUE(styleManager->isInitialized());
    
    // Test initialization with night theme
    StyleManager nightManager;
    nightManager.init(Themes::NIGHT);
    TEST_ASSERT_TRUE(nightManager.isInitialized());
}

void test_style_manager_initialize_styles() {
    styleManager->init(Themes::DAY);
    
    // Initialize styles should not crash
    styleManager->initializeStyles();
    TEST_ASSERT_TRUE(true);
}

void test_style_manager_theme_switching() {
    styleManager->init(Themes::DAY);
    
    // Get initial theme
    const char* initialTheme = styleManager->getCurrentTheme();
    TEST_ASSERT_EQUAL_STRING(Themes::DAY, initialTheme);
    
    // Switch theme
    styleManager->setTheme(Themes::NIGHT);
    const char* newTheme = styleManager->getCurrentTheme();
    TEST_ASSERT_EQUAL_STRING(Themes::NIGHT, newTheme);
    
    // Verify theme changed
    TEST_ASSERT_NOT_EQUAL_STRING(initialTheme, newTheme);
}

void test_style_manager_set_theme() {
    styleManager->init(Themes::DAY);
    
    // Set to night theme
    styleManager->set_theme(Themes::NIGHT);
    
    // Set back to day theme  
    styleManager->set_theme(Themes::DAY);
    
    // Test passed if no crashes occurred
    TEST_ASSERT_TRUE(true);
}

void test_style_manager_get_theme_colors() {
    styleManager->init(Themes::DAY);
    
    // Should be able to get theme colors without crashing
    const ThemeColors& colors = styleManager->getThemeColors();
    TEST_ASSERT_TRUE(true); // If we get here without crashing, test passes
}

void test_style_manager_get_styles() {
    styleManager->init(Themes::DAY);
    styleManager->initializeStyles();
    
    // Test getting various styles (these should not crash)
    lv_style_t& bgStyle = styleManager->getBackgroundStyle();
    lv_style_t& textStyle = styleManager->getTextStyle();
    lv_style_t& gaugeStyle = styleManager->getGaugeNormalStyle();
    lv_style_t& warningStyle = styleManager->getGaugeWarningStyle();
    lv_style_t& dangerStyle = styleManager->getGaugeDangerStyle();
    lv_style_t& indicatorStyle = styleManager->getGaugeIndicatorStyle();
    lv_style_t& itemsStyle = styleManager->getGaugeItemsStyle();
    lv_style_t& mainStyle = styleManager->getGaugeMainStyle();
    
    TEST_ASSERT_TRUE(true); // If we get here without crashing, test passes
}

void test_style_manager_apply_theme() {
    styleManager->init(Themes::DAY);
    
    // Create a mock screen object (nullptr is acceptable for this test)
    lv_obj_t* mockScreen = nullptr;
    
    // Apply theme should not crash even with null screen
    styleManager->applyThemeToScreen(mockScreen);
    TEST_ASSERT_TRUE(true);
}

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

void runStyleManagerTests() {
    setUp_style_manager();
    RUN_TEST(test_style_manager_construction);
    RUN_TEST(test_style_manager_init);
    RUN_TEST(test_style_manager_initialize_styles);
    RUN_TEST(test_style_manager_theme_switching);
    RUN_TEST(test_style_manager_set_theme);
    RUN_TEST(test_style_manager_get_theme_colors);
    RUN_TEST(test_style_manager_get_styles);
    RUN_TEST(test_style_manager_apply_theme);
    RUN_TEST(test_style_manager_day_night_differences);
    tearDown_style_manager();
}