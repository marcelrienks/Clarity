#include <unity.h>
#include "test_utilities.h"
#include <cstring>

// Ensure we use the exact same types as the test
typedef struct {
    uint32_t hex_value;
} local_style_lv_color_t;
    
typedef struct {
    bool initialized;
    local_style_lv_color_t bg_color;
    local_style_lv_color_t text_color;
    local_style_lv_color_t line_color;
    uint8_t bg_opa;
    uint8_t text_opa;
    uint16_t length;
    uint16_t line_width;
    uint16_t arc_width;
    bool reset_called;
} mock_lv_style_t;
    
    // Mock lv_obj_t
    typedef struct {
        bool styles_applied;
        bool invalidated;
    } mock_lv_obj_t;
    
    // Mock LVGL functions
    void mock_lv_style_init(mock_lv_style_t* style) {
        style->initialized = true;
        style->reset_called = false;
    }
    
    void mock_lv_style_reset(mock_lv_style_t* style) {
        style->reset_called = true;
        style->initialized = false;
    }
    
    void mock_lv_style_set_bg_color(mock_lv_style_t* style, mock_lv_color_t color) {
        style->bg_color = color;
    }
    
    void mock_lv_style_set_bg_opa(mock_lv_style_t* style, uint8_t opa) {
        style->bg_opa = opa;
    }
    
    void mock_lv_style_set_text_color(mock_lv_style_t* style, mock_lv_color_t color) {
        style->text_color = color;
    }
    
    void mock_lv_style_set_text_opa(mock_lv_style_t* style, uint8_t opa) {
        style->text_opa = opa;
    }
    
    void mock_lv_style_set_line_color(mock_lv_style_t* style, mock_lv_color_t color) {
        style->line_color = color;
    }
    
    void mock_lv_style_set_length(mock_lv_style_t* style, uint16_t length) {
        style->length = length;
    }
    
    void mock_lv_style_set_line_width(mock_lv_style_t* style, uint16_t width) {
        style->line_width = width;
    }
    
    void mock_lv_style_set_arc_width(mock_lv_style_t* style, uint16_t width) {
        style->arc_width = width;
    }
    
    void mock_lv_obj_add_style(mock_lv_obj_t* obj, mock_lv_style_t* style, uint32_t selector) {
        obj->styles_applied = true;
    }
    
    void mock_lv_obj_invalidate(mock_lv_obj_t* obj) {
        obj->invalidated = true;
    }
    
    // Mock screen
    mock_lv_obj_t mock_screen = {false, false};
    mock_lv_obj_t* mock_lv_scr_act() {
        return &mock_screen;
    }
    
    // Mock constants
    const uint8_t LV_OPA_COVER = 255;
    const uint32_t MAIN_DEFAULT = 0x01;
}

// Mock Themes namespace
namespace Themes {
    const char* DAY = "Day";
    const char* NIGHT = "Night";
}

// Mock ThemeColors structure
struct MockThemeColors {
    mock_lv_color_t background;
    mock_lv_color_t text;
    mock_lv_color_t primary;
    mock_lv_color_t gaugeNormal;
    mock_lv_color_t gaugeWarning;
    mock_lv_color_t gaugeDanger;
    mock_lv_color_t gaugeTicks;
    mock_lv_color_t needleNormal;
    mock_lv_color_t needleDanger;
    mock_lv_color_t keyPresent;
    mock_lv_color_t keyNotPresent;
};

// Mock StyleManager for testing
class MockStyleManager {
public:
    static MockStyleManager& GetInstance() {
        static MockStyleManager instance;
        return instance;
    }
    
    // Public data members
    const char* THEME = Themes::NIGHT;
    mock_lv_style_t backgroundStyle;
    mock_lv_style_t textStyle;
    mock_lv_style_t gaugeNormalStyle;
    mock_lv_style_t gaugeWarningStyle;
    mock_lv_style_t gaugeDangerStyle;
    mock_lv_style_t gaugeIndicatorStyle;
    mock_lv_style_t gaugeItemsStyle;
    mock_lv_style_t gaugeMainStyle;
    mock_lv_style_t gaugeDangerSectionStyle;
    
    // Theme colors
    MockThemeColors dayThemeColours_ = {
        mock_lv_color_hex(0x121212), // background
        mock_lv_color_hex(0xEEEEEE), // text
        mock_lv_color_hex(0xEEEEEE), // primary
        mock_lv_color_hex(0xEEEEEE), // gaugeNormal
        mock_lv_color_hex(0xFB8C00), // gaugeWarning
        mock_lv_color_hex(0xB00020), // gaugeDanger
        mock_lv_color_hex(0xF0F0E8), // gaugeTicks
        mock_lv_color_hex(0xFFFFFF), // needleNormal
        mock_lv_color_hex(0xDC143C), // needleDanger
        mock_lv_color_hex(0x006400), // keyPresent
        mock_lv_color_hex(0xDC143C)  // keyNotPresent
    };
    
    MockThemeColors nightThemeColours_ = {
        mock_lv_color_hex(0x000000), // background
        mock_lv_color_hex(0xB00020), // text
        mock_lv_color_hex(0xB00020), // primary
        mock_lv_color_hex(0xB00020), // gaugeNormal
        mock_lv_color_hex(0xFB8C00), // gaugeWarning
        mock_lv_color_hex(0xB00020), // gaugeDanger
        mock_lv_color_hex(0xB00020), // gaugeTicks
        mock_lv_color_hex(0xFFFFFF), // needleNormal
        mock_lv_color_hex(0xDC143C), // needleDanger
        mock_lv_color_hex(0x006400), // keyPresent
        mock_lv_color_hex(0xDC143C)  // keyNotPresent
    };
    
    void init(const char* theme) {
        THEME = theme;
        
        mock_lv_style_init(&backgroundStyle);
        mock_lv_style_init(&textStyle);
        mock_lv_style_init(&gaugeNormalStyle);
        mock_lv_style_init(&gaugeWarningStyle);
        mock_lv_style_init(&gaugeDangerStyle);
        mock_lv_style_init(&gaugeIndicatorStyle);
        mock_lv_style_init(&gaugeItemsStyle);
        mock_lv_style_init(&gaugeMainStyle);
        mock_lv_style_init(&gaugeDangerSectionStyle);
        
        set_theme(theme);
    }
    
    void set_theme(const char* theme) {
        THEME = theme;
        
        MockThemeColors colors = get_colours(theme);
        
        // Apply theme colors to styles
        mock_lv_style_set_bg_color(&backgroundStyle, colors.background);
        mock_lv_style_set_bg_opa(&backgroundStyle, LV_OPA_COVER);
        
        mock_lv_style_set_text_color(&textStyle, colors.text);
        mock_lv_style_set_text_opa(&textStyle, LV_OPA_COVER);
        
        mock_lv_style_set_line_color(&gaugeNormalStyle, colors.gaugeNormal);
        mock_lv_style_set_line_color(&gaugeWarningStyle, colors.gaugeWarning);
        mock_lv_style_set_line_color(&gaugeDangerStyle, colors.gaugeDanger);
        
        // Configure gauge styles
        mock_lv_style_set_length(&gaugeIndicatorStyle, 25);
        mock_lv_style_set_line_width(&gaugeIndicatorStyle, 7);
        mock_lv_style_set_line_color(&gaugeIndicatorStyle, colors.gaugeTicks);
        
        mock_lv_style_set_length(&gaugeItemsStyle, 18);
        mock_lv_style_set_line_width(&gaugeItemsStyle, 2);
        mock_lv_style_set_line_color(&gaugeItemsStyle, colors.gaugeTicks);
        
        mock_lv_style_set_arc_width(&gaugeMainStyle, 0);
        
        mock_lv_style_set_line_width(&gaugeDangerSectionStyle, 5);
        mock_lv_style_set_line_color(&gaugeDangerSectionStyle, colors.gaugeDanger);
        
        // Apply to current screen
        mock_lv_obj_t* current_screen = mock_lv_scr_act();
        if (current_screen) {
            apply_theme_to_screen(current_screen);
            mock_lv_obj_invalidate(current_screen);
        }
    }
    
    void apply_theme_to_screen(mock_lv_obj_t* screen) {
        mock_lv_obj_add_style(screen, &backgroundStyle, MAIN_DEFAULT);
    }
    
    const MockThemeColors& get_colours(const char* theme) const {
        return (theme && strcmp(theme, Themes::NIGHT) == 0) ? nightThemeColours_ : dayThemeColours_;
    }
    
    void ResetStyles() {
        mock_lv_style_reset(&backgroundStyle);
        mock_lv_style_reset(&textStyle);
        mock_lv_style_reset(&gaugeNormalStyle);
        mock_lv_style_reset(&gaugeWarningStyle);
        mock_lv_style_reset(&gaugeDangerStyle);
        mock_lv_style_reset(&gaugeIndicatorStyle);
        mock_lv_style_reset(&gaugeItemsStyle);
        mock_lv_style_reset(&gaugeMainStyle);
        mock_lv_style_reset(&gaugeDangerSectionStyle);
    }
};

// Note: setUp() and tearDown() are defined in test_main.cpp

void resetMockStyleState() {
    mock_screen.styles_applied = false;
    mock_screen.invalidated = false;
}

// =================================================================
// STYLE MANAGER INITIALIZATION TESTS
// =================================================================

void test_style_manager_singleton_access(void) {
    MockStyleManager& sm1 = MockStyleManager::GetInstance();
    MockStyleManager& sm2 = MockStyleManager::GetInstance();
    
    TEST_ASSERT_EQUAL_PTR(&sm1, &sm2);
}

void test_style_manager_initialization_day_theme(void) {
    resetMockStyleState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    sm.init(Themes::DAY);
    
    TEST_ASSERT_EQUAL_STRING(Themes::DAY, sm.THEME);
    TEST_ASSERT_TRUE(sm.backgroundStyle.initialized);
    TEST_ASSERT_TRUE(sm.textStyle.initialized);
    TEST_ASSERT_TRUE(sm.gaugeNormalStyle.initialized);
}

void test_style_manager_initialization_night_theme(void) {
    resetMockStyleState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    sm.init(Themes::NIGHT);
    
    TEST_ASSERT_EQUAL_STRING(Themes::NIGHT, sm.THEME);
    TEST_ASSERT_TRUE(sm.backgroundStyle.initialized);
    TEST_ASSERT_TRUE(sm.textStyle.initialized);
    TEST_ASSERT_TRUE(sm.gaugeNormalStyle.initialized);
}

void test_style_manager_all_styles_initialized(void) {
    resetMockStyleState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    sm.init(Themes::DAY);
    
    // Check all styles are initialized
    TEST_ASSERT_TRUE(sm.backgroundStyle.initialized);
    TEST_ASSERT_TRUE(sm.textStyle.initialized);
    TEST_ASSERT_TRUE(sm.gaugeNormalStyle.initialized);
    TEST_ASSERT_TRUE(sm.gaugeWarningStyle.initialized);
    TEST_ASSERT_TRUE(sm.gaugeDangerStyle.initialized);
    TEST_ASSERT_TRUE(sm.gaugeIndicatorStyle.initialized);
    TEST_ASSERT_TRUE(sm.gaugeItemsStyle.initialized);
    TEST_ASSERT_TRUE(sm.gaugeMainStyle.initialized);
    TEST_ASSERT_TRUE(sm.gaugeDangerSectionStyle.initialized);
}

// =================================================================
// THEME SWITCHING TESTS
// =================================================================

void test_style_manager_theme_switching_day_to_night(void) {
    resetMockStyleState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    sm.init(Themes::DAY);
    TEST_ASSERT_EQUAL_STRING(Themes::DAY, sm.THEME);
    
    sm.set_theme(Themes::NIGHT);
    TEST_ASSERT_EQUAL_STRING(Themes::NIGHT, sm.THEME);
}

void test_style_manager_theme_switching_night_to_day(void) {
    resetMockStyleState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    sm.init(Themes::NIGHT);
    TEST_ASSERT_EQUAL_STRING(Themes::NIGHT, sm.THEME);
    
    sm.set_theme(Themes::DAY);
    TEST_ASSERT_EQUAL_STRING(Themes::DAY, sm.THEME);
}

void test_style_manager_multiple_theme_switches(void) {
    resetMockStyleState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    const char* themes[] = {Themes::DAY, Themes::NIGHT, Themes::DAY, Themes::NIGHT};
    
    for (int i = 0; i < 4; i++) {
        sm.set_theme(themes[i]);
        TEST_ASSERT_EQUAL_STRING(themes[i], sm.THEME);
    }
}

// =================================================================
// COLOR SCHEME TESTS
// =================================================================

void test_style_manager_day_theme_colors(void) {
    resetMockStyleState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    MockThemeColors colors = sm.get_colours(Themes::DAY);
    
    TEST_ASSERT_EQUAL_HEX32(0x121212, colors.background.hex_value);
    TEST_ASSERT_EQUAL_HEX32(0xEEEEEE, colors.text.hex_value);
    TEST_ASSERT_EQUAL_HEX32(0xFB8C00, colors.gaugeWarning.hex_value);
    TEST_ASSERT_EQUAL_HEX32(0xB00020, colors.gaugeDanger.hex_value);
}

void test_style_manager_night_theme_colors(void) {
    resetMockStyleState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    MockThemeColors colors = sm.get_colours(Themes::NIGHT);
    
    TEST_ASSERT_EQUAL_HEX32(0x000000, colors.background.hex_value);
    TEST_ASSERT_EQUAL_HEX32(0xB00020, colors.text.hex_value);
    TEST_ASSERT_EQUAL_HEX32(0xFB8C00, colors.gaugeWarning.hex_value);
    TEST_ASSERT_EQUAL_HEX32(0xB00020, colors.gaugeDanger.hex_value);
}

void test_style_manager_color_consistency_across_themes(void) {
    resetMockStyleState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    MockThemeColors dayColors = sm.get_colours(Themes::DAY);
    MockThemeColors nightColors = sm.get_colours(Themes::NIGHT);
    
    // Warning and danger colors should be consistent
    TEST_ASSERT_EQUAL_HEX32(dayColors.gaugeWarning.hex_value, nightColors.gaugeWarning.hex_value);
    TEST_ASSERT_EQUAL_HEX32(dayColors.needleDanger.hex_value, nightColors.needleDanger.hex_value);
    TEST_ASSERT_EQUAL_HEX32(dayColors.keyPresent.hex_value, nightColors.keyPresent.hex_value);
}

// =================================================================
// STYLE APPLICATION TESTS
// =================================================================

void test_style_manager_apply_theme_to_screen(void) {
    resetMockStyleState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    sm.init(Themes::DAY);
    
    mock_lv_obj_t test_screen = {false, false};
    sm.apply_theme_to_screen(&test_screen);
    
    TEST_ASSERT_TRUE(test_screen.styles_applied);
}

void test_style_manager_screen_invalidation_on_theme_change(void) {
    resetMockStyleState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    sm.init(Themes::DAY);
    sm.set_theme(Themes::NIGHT);
    
    TEST_ASSERT_TRUE(mock_screen.invalidated);
}

void test_style_manager_background_style_properties(void) {
    resetMockStyleState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    sm.init(Themes::DAY);
    
    TEST_ASSERT_EQUAL_HEX32(0x121212, sm.backgroundStyle.bg_color.hex_value);
    TEST_ASSERT_EQUAL_UINT8(LV_OPA_COVER, sm.backgroundStyle.bg_opa);
}

void test_style_manager_text_style_properties(void) {
    resetMockStyleState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    sm.init(Themes::DAY);
    
    TEST_ASSERT_EQUAL_HEX32(0xEEEEEE, sm.textStyle.text_color.hex_value);
    TEST_ASSERT_EQUAL_UINT8(LV_OPA_COVER, sm.textStyle.text_opa);
}

// =================================================================
// GAUGE STYLE TESTS
// =================================================================

void test_style_manager_gauge_indicator_properties(void) {
    resetMockStyleState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    sm.init(Themes::DAY);
    
    TEST_ASSERT_EQUAL_UINT16(25, sm.gaugeIndicatorStyle.length);
    TEST_ASSERT_EQUAL_UINT16(7, sm.gaugeIndicatorStyle.line_width);
}

void test_style_manager_gauge_items_properties(void) {
    resetMockStyleState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    sm.init(Themes::DAY);
    
    TEST_ASSERT_EQUAL_UINT16(18, sm.gaugeItemsStyle.length);
    TEST_ASSERT_EQUAL_UINT16(2, sm.gaugeItemsStyle.line_width);
}

void test_style_manager_gauge_main_properties(void) {
    resetMockStyleState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    sm.init(Themes::DAY);
    
    TEST_ASSERT_EQUAL_UINT16(0, sm.gaugeMainStyle.arc_width);
}

void test_style_manager_gauge_danger_section_properties(void) {
    resetMockStyleState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    sm.init(Themes::DAY);
    
    TEST_ASSERT_EQUAL_UINT16(5, sm.gaugeDangerSectionStyle.line_width);
    TEST_ASSERT_EQUAL_HEX32(0xB00020, sm.gaugeDangerSectionStyle.line_color.hex_value);
}

// =================================================================
// STYLE RESET TESTS
// =================================================================

void test_style_manager_reset_styles(void) {
    resetMockStyleState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    sm.init(Themes::DAY);
    TEST_ASSERT_TRUE(sm.backgroundStyle.initialized);
    
    sm.ResetStyles();
    
    TEST_ASSERT_TRUE(sm.backgroundStyle.reset_called);
    TEST_ASSERT_TRUE(sm.textStyle.reset_called);
    TEST_ASSERT_TRUE(sm.gaugeNormalStyle.reset_called);
    TEST_ASSERT_TRUE(sm.gaugeWarningStyle.reset_called);
    TEST_ASSERT_TRUE(sm.gaugeDangerStyle.reset_called);
}

void test_style_manager_reset_all_gauge_styles(void) {
    resetMockStyleState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    sm.init(Themes::DAY);
    sm.ResetStyles();
    
    TEST_ASSERT_TRUE(sm.gaugeIndicatorStyle.reset_called);
    TEST_ASSERT_TRUE(sm.gaugeItemsStyle.reset_called);
    TEST_ASSERT_TRUE(sm.gaugeMainStyle.reset_called);
    TEST_ASSERT_TRUE(sm.gaugeDangerSectionStyle.reset_called);
}

// =================================================================
// INTEGRATION TESTS
// =================================================================

void test_style_manager_full_lifecycle(void) {
    resetMockStyleState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    // Initialize with day theme
    sm.init(Themes::DAY);
    TEST_ASSERT_EQUAL_STRING(Themes::DAY, sm.THEME);
    TEST_ASSERT_TRUE(sm.backgroundStyle.initialized);
    
    // Switch to night theme
    sm.set_theme(Themes::NIGHT);
    TEST_ASSERT_EQUAL_STRING(Themes::NIGHT, sm.THEME);
    
    // Reset styles
    sm.ResetStyles();
    TEST_ASSERT_TRUE(sm.backgroundStyle.reset_called);
}

void test_style_manager_theme_persistence(void) {
    resetMockStyleState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    // Set night theme
    sm.set_theme(Themes::NIGHT);
    const char* theme1 = sm.THEME;
    
    // Apply to screen (shouldn't change theme)
    mock_lv_obj_t test_screen = {false, false};
    sm.apply_theme_to_screen(&test_screen);
    const char* theme2 = sm.THEME;
    
    TEST_ASSERT_EQUAL_STRING(theme1, theme2);
    TEST_ASSERT_TRUE(test_screen.styles_applied);
}

// =================================================================
// ERROR HANDLING TESTS
// =================================================================

void test_style_manager_null_theme_handling(void) {
    resetMockStyleState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    // Should default to day theme for null input
    MockThemeColors colors = sm.get_colours(nullptr);
    MockThemeColors dayColors = sm.get_colours(Themes::DAY);
    
    TEST_ASSERT_EQUAL_HEX32(dayColors.background.hex_value, colors.background.hex_value);
}

void test_style_manager_invalid_theme_handling(void) {
    resetMockStyleState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    // Should default to day theme for invalid input
    MockThemeColors colors = sm.get_colours("InvalidTheme");
    MockThemeColors dayColors = sm.get_colours(Themes::DAY);
    
    TEST_ASSERT_EQUAL_HEX32(dayColors.background.hex_value, colors.background.hex_value);
}

void test_style_manager_repeated_initialization(void) {
    resetMockStyleState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    // Initialize multiple times should be safe
    sm.init(Themes::DAY);
    const char* theme1 = sm.THEME;
    
    sm.init(Themes::NIGHT);
    const char* theme2 = sm.THEME;
    
    sm.init(Themes::DAY);
    const char* theme3 = sm.THEME;
    
    TEST_ASSERT_EQUAL_STRING(Themes::DAY, theme1);
    TEST_ASSERT_EQUAL_STRING(Themes::NIGHT, theme2);
    TEST_ASSERT_EQUAL_STRING(Themes::DAY, theme3);
}

// =================================================================
// PERFORMANCE TESTS
// =================================================================

void test_style_manager_rapid_theme_switching(void) {
    resetMockStyleState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    sm.init(Themes::DAY);
    
    // Rapid theme switching should be stable
    for (int i = 0; i < 20; i++) {
        const char* theme = (i % 2 == 0) ? Themes::NIGHT : Themes::DAY;
        sm.set_theme(theme);
        TEST_ASSERT_EQUAL_STRING(theme, sm.THEME);
    }
}

void test_style_manager_memory_consistency(void) {
    resetMockStyleState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    sm.init(Themes::DAY);
    
    // Styles should remain properly initialized after multiple operations
    sm.set_theme(Themes::NIGHT);
    sm.set_theme(Themes::DAY);
    
    TEST_ASSERT_TRUE(sm.backgroundStyle.initialized);
    TEST_ASSERT_TRUE(sm.textStyle.initialized);
    TEST_ASSERT_TRUE(sm.gaugeNormalStyle.initialized);
}

// Note: PlatformIO will automatically discover and run test_ functions