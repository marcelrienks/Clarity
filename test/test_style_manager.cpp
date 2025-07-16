#ifdef UNIT_TESTING

#include <unity.h>
#include <string>
#include <cstdint>

// Mock LVGL types and functions
struct lv_color_t {
    uint32_t full;
    lv_color_t(uint32_t color) : full(color) {}
    lv_color_t() : full(0) {}
    bool operator==(const lv_color_t& other) const {
        return full == other.full;
    }
};

struct lv_style_t {
    bool initialized = false;
    lv_color_t bg_color = lv_color_t(0);
    lv_color_t text_color = lv_color_t(0);
    lv_color_t line_color = lv_color_t(0);
    int32_t line_width = 0;
    int32_t length = 0;
    int32_t arc_width = 0;
    uint8_t bg_opa = 0;
    uint8_t text_opa = 0;
};

struct lv_obj_t {
    lv_style_t* applied_style = nullptr;
    uint32_t style_selector = 0;
};

// Mock LVGL constants
#define LV_OPA_COVER 255
#define LV_PART_MAIN 0x00000000
#define LV_STATE_DEFAULT 0x00000000
#define LV_PART_ITEMS 0x00100000
#define LV_PART_INDICATOR 0x00200000
#define MAIN_DEFAULT (LV_PART_MAIN | LV_STATE_DEFAULT)
#define ITEMS_DEFAULT (LV_PART_ITEMS | LV_STATE_DEFAULT)
#define INDICATOR_DEFAULT (LV_PART_INDICATOR | LV_STATE_DEFAULT)

// Mock LVGL functions
void lv_style_init(lv_style_t* style) {
    style->initialized = true;
}

void lv_style_reset(lv_style_t* style) {
    style->initialized = false;
    style->bg_color = lv_color_t(0);
    style->text_color = lv_color_t(0);
    style->line_color = lv_color_t(0);
    style->line_width = 0;
    style->length = 0;
    style->arc_width = 0;
    style->bg_opa = 0;
    style->text_opa = 0;
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

void lv_style_set_line_width(lv_style_t* style, int32_t width) {
    style->line_width = width;
}

void lv_style_set_length(lv_style_t* style, int32_t length) {
    style->length = length;
}

void lv_style_set_arc_width(lv_style_t* style, int32_t width) {
    style->arc_width = width;
}

void lv_obj_add_style(lv_obj_t* obj, lv_style_t* style, uint32_t selector) {
    obj->applied_style = style;
    obj->style_selector = selector;
}

lv_color_t lv_color_hex(uint32_t hex) {
    return lv_color_t(hex);
}

// Mock types from utilities/types.h
enum class Themes {
    Night = 0,
    Day = 1
};

// Mock ThemeColors structure
struct ThemeColors {
    lv_color_t background;
    lv_color_t text;
    lv_color_t primary;
    lv_color_t gauge_normal;
    lv_color_t gauge_warning;
    lv_color_t gauge_danger;
    lv_color_t gauge_ticks;
    lv_color_t needle_normal;
    lv_color_t needle_danger;
    lv_color_t key_present;
    lv_color_t key_not_present;
};

// Mock StyleManager implementation for testing
class MockStyleManager {
private:
    Themes _theme = Themes::Night;
    
    ThemeColors _day_theme_colours = {
        .background = lv_color_hex(0x121212),
        .text = lv_color_hex(0xEEEEEE),
        .primary = lv_color_hex(0xEEEEEE),
        .gauge_normal = lv_color_hex(0xEEEEEE),
        .gauge_warning = lv_color_hex(0xFB8C00),
        .gauge_danger = lv_color_hex(0xB00020),
        .gauge_ticks = lv_color_hex(0xF0F0E8),
        .needle_normal = lv_color_hex(0xFFFFFF),
        .needle_danger = lv_color_hex(0xDC143C),
        .key_present = lv_color_hex(0x006400),
        .key_not_present = lv_color_hex(0xDC143C)
    };
    
    ThemeColors _night_theme_colours = {
        .background = lv_color_hex(0x121212),
        .text = lv_color_hex(0xB00020),
        .primary = lv_color_hex(0xB00020),
        .gauge_normal = lv_color_hex(0xB00020),
        .gauge_warning = lv_color_hex(0xFB8C00),
        .gauge_danger = lv_color_hex(0xB00020),
        .gauge_ticks = lv_color_hex(0xF0F0E8),
        .needle_normal = lv_color_hex(0xFFFFFF),
        .needle_danger = lv_color_hex(0xDC143C),
        .key_present = lv_color_hex(0x006400),
        .key_not_present = lv_color_hex(0xDC143C)
    };

public:
    // Public style objects
    lv_style_t background_style;
    lv_style_t text_style;
    lv_style_t gauge_normal_style;
    lv_style_t gauge_warning_style;
    lv_style_t gauge_danger_style;
    lv_style_t gauge_indicator_style;
    lv_style_t gauge_items_style;
    lv_style_t gauge_main_style;
    lv_style_t gauge_danger_section_style;

    void init(Themes theme) {
        _theme = theme;
        
        lv_style_init(&background_style);
        lv_style_init(&text_style);
        lv_style_init(&gauge_normal_style);
        lv_style_init(&gauge_warning_style);
        lv_style_init(&gauge_danger_style);
        lv_style_init(&gauge_indicator_style);
        lv_style_init(&gauge_items_style);
        lv_style_init(&gauge_main_style);
        lv_style_init(&gauge_danger_section_style);
        
        set_theme(_theme);
    }

    void set_theme(Themes theme) {
        _theme = theme;
        ThemeColors colours = get_colours(theme);
        
        // Background style
        lv_style_set_bg_color(&background_style, colours.background);
        lv_style_set_bg_opa(&background_style, LV_OPA_COVER);
        
        // Text style
        lv_style_set_text_color(&text_style, colours.text);
        lv_style_set_text_opa(&text_style, LV_OPA_COVER);
        
        // Gauge styles
        lv_style_set_line_color(&gauge_normal_style, colours.gauge_normal);
        lv_style_set_line_color(&gauge_warning_style, colours.gauge_warning);
        lv_style_set_line_color(&gauge_danger_style, colours.gauge_danger);
        
        // Configure shared gauge component styles
        lv_style_set_length(&gauge_indicator_style, 25);
        lv_style_set_line_width(&gauge_indicator_style, 7);
        lv_style_set_line_color(&gauge_indicator_style, colours.gauge_ticks);
        
        lv_style_set_length(&gauge_items_style, 18);
        lv_style_set_line_width(&gauge_items_style, 2);
        lv_style_set_line_color(&gauge_items_style, colours.gauge_ticks);
        
        lv_style_set_arc_width(&gauge_main_style, 0);
        
        lv_style_set_line_width(&gauge_danger_section_style, 5);
        lv_style_set_line_color(&gauge_danger_section_style, colours.gauge_danger);
    }

    void apply_theme_to_screen(lv_obj_t* screen) {
        lv_obj_add_style(screen, &background_style, MAIN_DEFAULT);
    }

    void reset_styles() {
        lv_style_reset(&background_style);
        lv_style_reset(&text_style);
        lv_style_reset(&gauge_normal_style);
        lv_style_reset(&gauge_warning_style);
        lv_style_reset(&gauge_danger_style);
        lv_style_reset(&gauge_indicator_style);
        lv_style_reset(&gauge_items_style);
        lv_style_reset(&gauge_main_style);
        lv_style_reset(&gauge_danger_section_style);
    }

    const ThemeColors& get_colours(const Themes& theme) const {
        return theme == Themes::Night ? _night_theme_colours : _day_theme_colours;
    }

    const Themes& get_theme() const {
        return _theme;
    }

    // Style accessor methods
    lv_style_t* get_gauge_indicator_style() { return &gauge_indicator_style; }
    lv_style_t* get_gauge_items_style() { return &gauge_items_style; }
    lv_style_t* get_gauge_main_style() { return &gauge_main_style; }
    lv_style_t* get_gauge_danger_section_style() { return &gauge_danger_section_style; }
};

// Test fixtures
MockStyleManager* test_style_manager = nullptr;

void style_manager_setUp(void) {
    test_style_manager = new MockStyleManager();
}

void style_manager_tearDown(void) {
    delete test_style_manager;
    test_style_manager = nullptr;
}

// Test cases
void test_style_manager_initialization(void) {
    test_style_manager->init(Themes::Night);
    
    TEST_ASSERT_TRUE(test_style_manager->background_style.initialized);
    TEST_ASSERT_TRUE(test_style_manager->text_style.initialized);
    TEST_ASSERT_TRUE(test_style_manager->gauge_normal_style.initialized);
    TEST_ASSERT_TRUE(test_style_manager->gauge_warning_style.initialized);
    TEST_ASSERT_TRUE(test_style_manager->gauge_danger_style.initialized);
    TEST_ASSERT_TRUE(test_style_manager->gauge_indicator_style.initialized);
    TEST_ASSERT_TRUE(test_style_manager->gauge_items_style.initialized);
    TEST_ASSERT_TRUE(test_style_manager->gauge_main_style.initialized);
    TEST_ASSERT_TRUE(test_style_manager->gauge_danger_section_style.initialized);
}

void test_night_theme_colors(void) {
    test_style_manager->init(Themes::Night);
    
    const ThemeColors& colors = test_style_manager->get_colours(Themes::Night);
    
    TEST_ASSERT_EQUAL(0x121212, colors.background.full);
    TEST_ASSERT_EQUAL(0xB00020, colors.text.full);
    TEST_ASSERT_EQUAL(0xB00020, colors.primary.full);
    TEST_ASSERT_EQUAL(0xB00020, colors.gauge_normal.full);
    TEST_ASSERT_EQUAL(0xFB8C00, colors.gauge_warning.full);
    TEST_ASSERT_EQUAL(0xB00020, colors.gauge_danger.full);
}

void test_day_theme_colors(void) {
    test_style_manager->init(Themes::Day);
    
    const ThemeColors& colors = test_style_manager->get_colours(Themes::Day);
    
    TEST_ASSERT_EQUAL(0x121212, colors.background.full);
    TEST_ASSERT_EQUAL(0xEEEEEE, colors.text.full);
    TEST_ASSERT_EQUAL(0xEEEEEE, colors.primary.full);
    TEST_ASSERT_EQUAL(0xEEEEEE, colors.gauge_normal.full);
    TEST_ASSERT_EQUAL(0xFB8C00, colors.gauge_warning.full);
    TEST_ASSERT_EQUAL(0xB00020, colors.gauge_danger.full);
}

void test_background_style_application(void) {
    test_style_manager->init(Themes::Night);
    
    TEST_ASSERT_EQUAL(0x121212, test_style_manager->background_style.bg_color.full);
    TEST_ASSERT_EQUAL(LV_OPA_COVER, test_style_manager->background_style.bg_opa);
}

void test_text_style_application(void) {
    test_style_manager->init(Themes::Night);
    
    TEST_ASSERT_EQUAL(0xB00020, test_style_manager->text_style.text_color.full);
    TEST_ASSERT_EQUAL(LV_OPA_COVER, test_style_manager->text_style.text_opa);
}

void test_gauge_styles_configuration(void) {
    test_style_manager->init(Themes::Night);
    
    // Test gauge indicator style
    TEST_ASSERT_EQUAL(25, test_style_manager->gauge_indicator_style.length);
    TEST_ASSERT_EQUAL(7, test_style_manager->gauge_indicator_style.line_width);
    TEST_ASSERT_EQUAL(0xF0F0E8, test_style_manager->gauge_indicator_style.line_color.full);
    
    // Test gauge items style
    TEST_ASSERT_EQUAL(18, test_style_manager->gauge_items_style.length);
    TEST_ASSERT_EQUAL(2, test_style_manager->gauge_items_style.line_width);
    TEST_ASSERT_EQUAL(0xF0F0E8, test_style_manager->gauge_items_style.line_color.full);
    
    // Test gauge main style
    TEST_ASSERT_EQUAL(0, test_style_manager->gauge_main_style.arc_width);
    
    // Test gauge danger section style
    TEST_ASSERT_EQUAL(5, test_style_manager->gauge_danger_section_style.line_width);
    TEST_ASSERT_EQUAL(0xB00020, test_style_manager->gauge_danger_section_style.line_color.full);
}

void test_theme_switching(void) {
    test_style_manager->init(Themes::Night);
    
    // Initially night theme
    TEST_ASSERT_EQUAL(0xB00020, test_style_manager->text_style.text_color.full);
    
    // Switch to day theme
    test_style_manager->set_theme(Themes::Day);
    
    TEST_ASSERT_EQUAL(0xEEEEEE, test_style_manager->text_style.text_color.full);
    TEST_ASSERT_EQUAL(static_cast<int>(Themes::Day), static_cast<int>(test_style_manager->get_theme()));
}

void test_screen_theme_application(void) {
    test_style_manager->init(Themes::Night);
    
    lv_obj_t screen;
    screen.applied_style = nullptr;
    screen.style_selector = 0;
    
    test_style_manager->apply_theme_to_screen(&screen);
    
    TEST_ASSERT_NOT_NULL(screen.applied_style);
    TEST_ASSERT_EQUAL(MAIN_DEFAULT, screen.style_selector);
    TEST_ASSERT_EQUAL(&test_style_manager->background_style, screen.applied_style);
}

void test_style_reset_functionality(void) {
    test_style_manager->init(Themes::Night);
    
    // Verify styles are initialized
    TEST_ASSERT_TRUE(test_style_manager->background_style.initialized);
    TEST_ASSERT_TRUE(test_style_manager->text_style.initialized);
    
    // Reset styles
    test_style_manager->reset_styles();
    
    // Verify styles are reset
    TEST_ASSERT_FALSE(test_style_manager->background_style.initialized);
    TEST_ASSERT_FALSE(test_style_manager->text_style.initialized);
    TEST_ASSERT_FALSE(test_style_manager->gauge_normal_style.initialized);
    TEST_ASSERT_FALSE(test_style_manager->gauge_warning_style.initialized);
    TEST_ASSERT_FALSE(test_style_manager->gauge_danger_style.initialized);
}

void test_style_accessors(void) {
    test_style_manager->init(Themes::Night);
    
    TEST_ASSERT_EQUAL(&test_style_manager->gauge_indicator_style, test_style_manager->get_gauge_indicator_style());
    TEST_ASSERT_EQUAL(&test_style_manager->gauge_items_style, test_style_manager->get_gauge_items_style());
    TEST_ASSERT_EQUAL(&test_style_manager->gauge_main_style, test_style_manager->get_gauge_main_style());
    TEST_ASSERT_EQUAL(&test_style_manager->gauge_danger_section_style, test_style_manager->get_gauge_danger_section_style());
}

void test_gauge_warning_and_danger_colors(void) {
    test_style_manager->init(Themes::Night);
    
    // Test gauge color styles
    TEST_ASSERT_EQUAL(0xB00020, test_style_manager->gauge_normal_style.line_color.full);
    TEST_ASSERT_EQUAL(0xFB8C00, test_style_manager->gauge_warning_style.line_color.full);
    TEST_ASSERT_EQUAL(0xB00020, test_style_manager->gauge_danger_style.line_color.full);
}

void test_key_colors_in_theme(void) {
    test_style_manager->init(Themes::Night);
    
    const ThemeColors& colors = test_style_manager->get_colours(Themes::Night);
    
    TEST_ASSERT_EQUAL(0x006400, colors.key_present.full);        // Deep green
    TEST_ASSERT_EQUAL(0xDC143C, colors.key_not_present.full);   // Crimson red
}

void test_style_manager_main() {
    // Each test needs its own setup/teardown
    style_manager_setUp(); RUN_TEST(test_style_manager_initialization); style_manager_tearDown();
    style_manager_setUp(); RUN_TEST(test_night_theme_colors); style_manager_tearDown();
    style_manager_setUp(); RUN_TEST(test_day_theme_colors); style_manager_tearDown();
    style_manager_setUp(); RUN_TEST(test_background_style_application); style_manager_tearDown();
    style_manager_setUp(); RUN_TEST(test_text_style_application); style_manager_tearDown();
    style_manager_setUp(); RUN_TEST(test_gauge_styles_configuration); style_manager_tearDown();
    style_manager_setUp(); RUN_TEST(test_theme_switching); style_manager_tearDown();
    style_manager_setUp(); RUN_TEST(test_screen_theme_application); style_manager_tearDown();
    style_manager_setUp(); RUN_TEST(test_style_reset_functionality); style_manager_tearDown();
    style_manager_setUp(); RUN_TEST(test_style_accessors); style_manager_tearDown();
    style_manager_setUp(); RUN_TEST(test_gauge_warning_and_danger_colors); style_manager_tearDown();
    style_manager_setUp(); RUN_TEST(test_key_colors_in_theme); style_manager_tearDown();
}

#endif