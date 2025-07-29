#include <unity.h>
#include "test_utilities.h"
#include <cstring>

// StyleManager-specific mock state using shared infrastructure
namespace StyleManagerMocks {
    // Mock LVGL color type
    typedef struct {
        uint32_t hex_value;
    } lv_color_t;
    
    lv_color_t lv_color_hex(uint32_t hex) {
        return {hex};
    }
    
    // Mock LVGL style type
    typedef struct {
        bool initialized;
        lv_color_t bg_color;
        lv_color_t text_color;
        lv_color_t line_color;
        uint8_t bg_opa;
        uint8_t text_opa;
        uint16_t length;
        uint16_t line_width;
        uint16_t arc_width;
        bool reset_called;
    } lv_style_t;
    
    // Mock LVGL object type
    typedef struct {
        bool styles_applied;
        bool invalidated;
    } lv_obj_t;
    
    // Mock LVGL functions
    void lv_style_init(lv_style_t* style) {
        style->initialized = true;
        style->reset_called = false;
    }
    
    void lv_style_reset(lv_style_t* style) {
        style->reset_called = true;
        style->initialized = false;
    }
    
    void lv_style_set_bg_color(lv_style_t* style, lv_color_t color) {
        style->bg_color = color;
    }
    
    void lv_style_set_bg_opa(lv_style_t* style, uint8_t opa) {
        style->bg_opa = opa;
    }
    
    void lv_style_set_text_color(lv_style_t* style, lv_color_t color) {
        style->text_color = color;
    }
    
    void lv_style_set_text_opa(lv_style_t* style, uint8_t opa) {
        style->text_opa = opa;
    }
    
    void lv_style_set_line_color(lv_style_t* style, lv_color_t color) {
        style->line_color = color;
    }
    
    void lv_style_set_length(lv_style_t* style, uint16_t length) {
        style->length = length;
    }
    
    void lv_style_set_line_width(lv_style_t* style, uint16_t width) {
        style->line_width = width;
    }
    
    void lv_style_set_arc_width(lv_style_t* style, uint16_t width) {
        style->arc_width = width;
    }
    
    void lv_obj_add_style(lv_obj_t* obj, lv_style_t* style, uint32_t selector) {
        obj->styles_applied = true;
    }
    
    void lv_obj_invalidate(lv_obj_t* obj) {
        obj->invalidated = true;
    }
    
    // Mock screen
    lv_obj_t screen = {false, false};
    lv_obj_t* lv_scr_act() {
        return &screen;
    }
    
    // Mock constants
    const uint8_t LV_OPA_COVER = 255;
    const uint32_t MAIN_DEFAULT = 0x01;
    
    void reset() {
        screen.styles_applied = false;
        screen.invalidated = false;
    }
}

// Mock Themes namespace
namespace Themes {
    const char* DAY = "Day";
    const char* NIGHT = "Night";
}

// Mock ThemeColors structure
struct ThemeColors {
    StyleManagerMocks::lv_color_t background;
    StyleManagerMocks::lv_color_t text;
    StyleManagerMocks::lv_color_t primary;
    StyleManagerMocks::lv_color_t gaugeNormal;
    StyleManagerMocks::lv_color_t gaugeWarning;
    StyleManagerMocks::lv_color_t gaugeDanger;
    StyleManagerMocks::lv_color_t gaugeTicks;
    StyleManagerMocks::lv_color_t needleNormal;
    StyleManagerMocks::lv_color_t needleDanger;
    StyleManagerMocks::lv_color_t keyPresent;
    StyleManagerMocks::lv_color_t keyNotPresent;
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
    StyleManagerMocks::lv_style_t backgroundStyle;
    StyleManagerMocks::lv_style_t textStyle;
    StyleManagerMocks::lv_style_t gaugeNormalStyle;
    StyleManagerMocks::lv_style_t gaugeWarningStyle;
    StyleManagerMocks::lv_style_t gaugeDangerStyle;
    StyleManagerMocks::lv_style_t gaugeIndicatorStyle;
    StyleManagerMocks::lv_style_t gaugeItemsStyle;
    StyleManagerMocks::lv_style_t gaugeMainStyle;
    StyleManagerMocks::lv_style_t gaugeDangerSectionStyle;
    
    // Theme colors
    ThemeColors dayThemeColours_ = {
        StyleManagerMocks::lv_color_hex(0x121212), // background
        StyleManagerMocks::lv_color_hex(0xEEEEEE), // text
        StyleManagerMocks::lv_color_hex(0xEEEEEE), // primary
        StyleManagerMocks::lv_color_hex(0xEEEEEE), // gaugeNormal
        StyleManagerMocks::lv_color_hex(0xFB8C00), // gaugeWarning
        StyleManagerMocks::lv_color_hex(0xB00020), // gaugeDanger
        StyleManagerMocks::lv_color_hex(0xF0F0E8), // gaugeTicks
        StyleManagerMocks::lv_color_hex(0xFFFFFF), // needleNormal
        StyleManagerMocks::lv_color_hex(0xDC143C), // needleDanger
        StyleManagerMocks::lv_color_hex(0x006400), // keyPresent
        StyleManagerMocks::lv_color_hex(0xDC143C)  // keyNotPresent
    };
    
    ThemeColors nightThemeColours_ = {
        StyleManagerMocks::lv_color_hex(0x000000), // background
        StyleManagerMocks::lv_color_hex(0xB00020), // text
        StyleManagerMocks::lv_color_hex(0xB00020), // primary
        StyleManagerMocks::lv_color_hex(0xB00020), // gaugeNormal
        StyleManagerMocks::lv_color_hex(0xFB8C00), // gaugeWarning
        StyleManagerMocks::lv_color_hex(0xB00020), // gaugeDanger
        StyleManagerMocks::lv_color_hex(0xB00020), // gaugeTicks
        StyleManagerMocks::lv_color_hex(0xFFFFFF), // needleNormal
        StyleManagerMocks::lv_color_hex(0xDC143C), // needleDanger
        StyleManagerMocks::lv_color_hex(0x006400), // keyPresent
        StyleManagerMocks::lv_color_hex(0xDC143C)  // keyNotPresent
    };
    
    void init(const char* theme) {
        THEME = theme;
        
        StyleManagerMocks::lv_style_init(&backgroundStyle);
        StyleManagerMocks::lv_style_init(&textStyle);
        StyleManagerMocks::lv_style_init(&gaugeNormalStyle);
        StyleManagerMocks::lv_style_init(&gaugeWarningStyle);
        StyleManagerMocks::lv_style_init(&gaugeDangerStyle);
        StyleManagerMocks::lv_style_init(&gaugeIndicatorStyle);
        StyleManagerMocks::lv_style_init(&gaugeItemsStyle);
        StyleManagerMocks::lv_style_init(&gaugeMainStyle);
        StyleManagerMocks::lv_style_init(&gaugeDangerSectionStyle);
        
        set_theme(theme);
    }
    
    void set_theme(const char* theme) {
        THEME = theme;
        
        ThemeColors colors = get_colours(theme);
        
        // Apply theme colors to styles
        StyleManagerMocks::lv_style_set_bg_color(&backgroundStyle, colors.background);
        StyleManagerMocks::lv_style_set_bg_opa(&backgroundStyle, StyleManagerMocks::LV_OPA_COVER);
        
        StyleManagerMocks::lv_style_set_text_color(&textStyle, colors.text);
        StyleManagerMocks::lv_style_set_text_opa(&textStyle, StyleManagerMocks::LV_OPA_COVER);
        
        StyleManagerMocks::lv_style_set_line_color(&gaugeNormalStyle, colors.gaugeNormal);
        StyleManagerMocks::lv_style_set_line_color(&gaugeWarningStyle, colors.gaugeWarning);
        StyleManagerMocks::lv_style_set_line_color(&gaugeDangerStyle, colors.gaugeDanger);
        
        // Configure gauge styles
        StyleManagerMocks::lv_style_set_length(&gaugeIndicatorStyle, 25);
        StyleManagerMocks::lv_style_set_line_width(&gaugeIndicatorStyle, 7);
        StyleManagerMocks::lv_style_set_line_color(&gaugeIndicatorStyle, colors.gaugeTicks);
        
        StyleManagerMocks::lv_style_set_length(&gaugeItemsStyle, 18);
        StyleManagerMocks::lv_style_set_line_width(&gaugeItemsStyle, 2);
        StyleManagerMocks::lv_style_set_line_color(&gaugeItemsStyle, colors.gaugeTicks);
        
        StyleManagerMocks::lv_style_set_arc_width(&gaugeMainStyle, 0);
        
        StyleManagerMocks::lv_style_set_line_width(&gaugeDangerSectionStyle, 5);
        StyleManagerMocks::lv_style_set_line_color(&gaugeDangerSectionStyle, colors.gaugeDanger);
        
        // Apply to current screen
        StyleManagerMocks::lv_obj_t* current_screen = StyleManagerMocks::lv_scr_act();
        if (current_screen) {
            apply_theme_to_screen(current_screen);
            StyleManagerMocks::lv_obj_invalidate(current_screen);
        }
    }
    
    void apply_theme_to_screen(StyleManagerMocks::lv_obj_t* screen) {
        StyleManagerMocks::lv_obj_add_style(screen, &backgroundStyle, StyleManagerMocks::MAIN_DEFAULT);
    }
    
    const ThemeColors& get_colours(const char* theme) const {
        return (theme && strcmp(theme, Themes::NIGHT) == 0) ? nightThemeColours_ : dayThemeColours_;
    }
    
    void ResetStyles() {
        StyleManagerMocks::lv_style_reset(&backgroundStyle);
        StyleManagerMocks::lv_style_reset(&textStyle);
        StyleManagerMocks::lv_style_reset(&gaugeNormalStyle);
        StyleManagerMocks::lv_style_reset(&gaugeWarningStyle);
        StyleManagerMocks::lv_style_reset(&gaugeDangerStyle);
        StyleManagerMocks::lv_style_reset(&gaugeIndicatorStyle);
        StyleManagerMocks::lv_style_reset(&gaugeItemsStyle);
        StyleManagerMocks::lv_style_reset(&gaugeMainStyle);
        StyleManagerMocks::lv_style_reset(&gaugeDangerSectionStyle);
    }
};

// Reset function for style manager tests
void resetStyleManagerMockState() {
    StyleManagerMocks::reset();
}

// =================================================================
// STYLE MANAGER TESTS
// =================================================================

void test_style_manager_singleton_access(void) {
    MockStyleManager& sm1 = MockStyleManager::GetInstance();
    MockStyleManager& sm2 = MockStyleManager::GetInstance();
    
    TEST_ASSERT_EQUAL_PTR(&sm1, &sm2);
}

void test_style_manager_initialization_day_theme(void) {
    resetStyleManagerMockState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    sm.init(Themes::DAY);
    
    TEST_ASSERT_EQUAL_STRING(Themes::DAY, sm.THEME);
    TEST_ASSERT_TRUE(sm.backgroundStyle.initialized);
    TEST_ASSERT_TRUE(sm.textStyle.initialized);
    TEST_ASSERT_TRUE(sm.gaugeNormalStyle.initialized);
}

void test_style_manager_initialization_night_theme(void) {
    resetStyleManagerMockState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    sm.init(Themes::NIGHT);
    
    TEST_ASSERT_EQUAL_STRING(Themes::NIGHT, sm.THEME);
    TEST_ASSERT_TRUE(sm.backgroundStyle.initialized);
    TEST_ASSERT_TRUE(sm.textStyle.initialized);
    TEST_ASSERT_TRUE(sm.gaugeNormalStyle.initialized);
}

void test_style_manager_all_styles_initialized(void) {
    resetStyleManagerMockState();
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

void test_style_manager_theme_switching_day_to_night(void) {
    resetStyleManagerMockState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    sm.init(Themes::DAY);
    TEST_ASSERT_EQUAL_STRING(Themes::DAY, sm.THEME);
    
    sm.set_theme(Themes::NIGHT);
    TEST_ASSERT_EQUAL_STRING(Themes::NIGHT, sm.THEME);
}

void test_style_manager_theme_switching_night_to_day(void) {
    resetStyleManagerMockState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    sm.init(Themes::NIGHT);
    TEST_ASSERT_EQUAL_STRING(Themes::NIGHT, sm.THEME);
    
    sm.set_theme(Themes::DAY);
    TEST_ASSERT_EQUAL_STRING(Themes::DAY, sm.THEME);
}

void test_style_manager_multiple_theme_switches(void) {
    resetStyleManagerMockState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    const char* themes[] = {Themes::DAY, Themes::NIGHT, Themes::DAY, Themes::NIGHT};
    
    for (int i = 0; i < 4; i++) {
        sm.set_theme(themes[i]);
        TEST_ASSERT_EQUAL_STRING(themes[i], sm.THEME);
    }
}

void test_style_manager_day_theme_colors(void) {
    resetStyleManagerMockState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    ThemeColors colors = sm.get_colours(Themes::DAY);
    
    TEST_ASSERT_EQUAL_HEX32(0x121212, colors.background.hex_value);
    TEST_ASSERT_EQUAL_HEX32(0xEEEEEE, colors.text.hex_value);
    TEST_ASSERT_EQUAL_HEX32(0xFB8C00, colors.gaugeWarning.hex_value);
    TEST_ASSERT_EQUAL_HEX32(0xB00020, colors.gaugeDanger.hex_value);
}

void test_style_manager_night_theme_colors(void) {
    resetStyleManagerMockState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    ThemeColors colors = sm.get_colours(Themes::NIGHT);
    
    TEST_ASSERT_EQUAL_HEX32(0x000000, colors.background.hex_value);
    TEST_ASSERT_EQUAL_HEX32(0xB00020, colors.text.hex_value);
    TEST_ASSERT_EQUAL_HEX32(0xFB8C00, colors.gaugeWarning.hex_value);
    TEST_ASSERT_EQUAL_HEX32(0xB00020, colors.gaugeDanger.hex_value);
}

void test_style_manager_color_consistency_across_themes(void) {
    resetStyleManagerMockState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    ThemeColors dayColors = sm.get_colours(Themes::DAY);
    ThemeColors nightColors = sm.get_colours(Themes::NIGHT);
    
    // Warning and danger colors should be consistent
    TEST_ASSERT_EQUAL_HEX32(dayColors.gaugeWarning.hex_value, nightColors.gaugeWarning.hex_value);
    TEST_ASSERT_EQUAL_HEX32(dayColors.needleDanger.hex_value, nightColors.needleDanger.hex_value);
    TEST_ASSERT_EQUAL_HEX32(dayColors.keyPresent.hex_value, nightColors.keyPresent.hex_value);
}

void test_style_manager_apply_theme_to_screen(void) {
    resetStyleManagerMockState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    sm.init(Themes::DAY);
    
    StyleManagerMocks::lv_obj_t test_screen = {false, false};
    sm.apply_theme_to_screen(&test_screen);
    
    TEST_ASSERT_TRUE(test_screen.styles_applied);
}

void test_style_manager_screen_invalidation_on_theme_change(void) {
    resetStyleManagerMockState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    sm.init(Themes::DAY);
    sm.set_theme(Themes::NIGHT);
    
    TEST_ASSERT_TRUE(StyleManagerMocks::screen.invalidated);
}

void test_style_manager_background_style_properties(void) {
    resetStyleManagerMockState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    sm.init(Themes::DAY);
    
    TEST_ASSERT_EQUAL_HEX32(0x121212, sm.backgroundStyle.bg_color.hex_value);
    TEST_ASSERT_EQUAL_UINT8(StyleManagerMocks::LV_OPA_COVER, sm.backgroundStyle.bg_opa);
}

void test_style_manager_text_style_properties(void) {
    resetStyleManagerMockState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    sm.init(Themes::DAY);
    
    TEST_ASSERT_EQUAL_HEX32(0xEEEEEE, sm.textStyle.text_color.hex_value);
    TEST_ASSERT_EQUAL_UINT8(StyleManagerMocks::LV_OPA_COVER, sm.textStyle.text_opa);
}

void test_style_manager_gauge_indicator_properties(void) {
    resetStyleManagerMockState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    sm.init(Themes::DAY);
    
    TEST_ASSERT_EQUAL_UINT16(25, sm.gaugeIndicatorStyle.length);
    TEST_ASSERT_EQUAL_UINT16(7, sm.gaugeIndicatorStyle.line_width);
}

void test_style_manager_gauge_items_properties(void) {
    resetStyleManagerMockState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    sm.init(Themes::DAY);
    
    TEST_ASSERT_EQUAL_UINT16(18, sm.gaugeItemsStyle.length);
    TEST_ASSERT_EQUAL_UINT16(2, sm.gaugeItemsStyle.line_width);
}

void test_style_manager_gauge_main_properties(void) {
    resetStyleManagerMockState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    sm.init(Themes::DAY);
    
    TEST_ASSERT_EQUAL_UINT16(0, sm.gaugeMainStyle.arc_width);
}

void test_style_manager_gauge_danger_section_properties(void) {
    resetStyleManagerMockState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    sm.init(Themes::DAY);
    
    TEST_ASSERT_EQUAL_UINT16(5, sm.gaugeDangerSectionStyle.line_width);
    TEST_ASSERT_EQUAL_HEX32(0xB00020, sm.gaugeDangerSectionStyle.line_color.hex_value);
}

void test_style_manager_reset_styles(void) {
    resetStyleManagerMockState();
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
    resetStyleManagerMockState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    sm.init(Themes::DAY);
    sm.ResetStyles();
    
    TEST_ASSERT_TRUE(sm.gaugeIndicatorStyle.reset_called);
    TEST_ASSERT_TRUE(sm.gaugeItemsStyle.reset_called);
    TEST_ASSERT_TRUE(sm.gaugeMainStyle.reset_called);
    TEST_ASSERT_TRUE(sm.gaugeDangerSectionStyle.reset_called);
}

void test_style_manager_full_lifecycle(void) {
    resetStyleManagerMockState();
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
    resetStyleManagerMockState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    // Set night theme
    sm.set_theme(Themes::NIGHT);
    const char* theme1 = sm.THEME;
    
    // Apply to screen (shouldn't change theme)
    StyleManagerMocks::lv_obj_t test_screen = {false, false};
    sm.apply_theme_to_screen(&test_screen);
    const char* theme2 = sm.THEME;
    
    TEST_ASSERT_EQUAL_STRING(theme1, theme2);
    TEST_ASSERT_TRUE(test_screen.styles_applied);
}

void test_style_manager_null_theme_handling(void) {
    resetStyleManagerMockState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    // Should default to day theme for null input
    ThemeColors colors = sm.get_colours(nullptr);
    ThemeColors dayColors = sm.get_colours(Themes::DAY);
    
    TEST_ASSERT_EQUAL_HEX32(dayColors.background.hex_value, colors.background.hex_value);
}

void test_style_manager_invalid_theme_handling(void) {
    resetStyleManagerMockState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    // Should default to day theme for invalid input
    ThemeColors colors = sm.get_colours("InvalidTheme");
    ThemeColors dayColors = sm.get_colours(Themes::DAY);
    
    TEST_ASSERT_EQUAL_HEX32(dayColors.background.hex_value, colors.background.hex_value);
}

void test_style_manager_repeated_initialization(void) {
    resetStyleManagerMockState();
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

void test_style_manager_rapid_theme_switching(void) {
    resetStyleManagerMockState();
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
    resetStyleManagerMockState();
    MockStyleManager& sm = MockStyleManager::GetInstance();
    
    sm.init(Themes::DAY);
    
    // Styles should remain properly initialized after multiple operations
    sm.set_theme(Themes::NIGHT);
    sm.set_theme(Themes::DAY);
    
    TEST_ASSERT_TRUE(sm.backgroundStyle.initialized);
    TEST_ASSERT_TRUE(sm.textStyle.initialized);
    TEST_ASSERT_TRUE(sm.gaugeNormalStyle.initialized);
}

// Only include the function count listed - 27 tests total
// Note: PlatformIO will automatically discover and run test_ functions