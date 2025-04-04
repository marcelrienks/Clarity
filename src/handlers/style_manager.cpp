#include "handlers/style_manager.h"
#include <esp32-hal-log.h>

StyleManager::StyleManager() : _current_theme(Theme::Dark) {
    // Initialize dark theme colors
    _dark_theme_colors = {
        .background = lv_color_hex(0x121212),    // Dark background
        .text = lv_color_hex(0xEEEEEE),          // Light text
        .primary = lv_color_hex(0x0288D1),       // Blue primary
        .secondary = lv_color_hex(0x03DAC6),     // Teal secondary
        .warning = lv_color_hex(0xFFA000),       // Amber warning
        .danger = lv_color_hex(0xCF6679),        // Red-pink danger
        .gauge_normal = lv_color_hex(0x03DAC6),  // Teal for normal range
        .gauge_warning = lv_color_hex(0xFFA000), // Amber for warning range
        .gauge_danger = lv_color_hex(0xCF6679)   // Red-pink for danger range
    };

    // Initialize light theme colors
    _light_theme_colors = {
        .background = lv_color_hex(0xFAFAFA),    // Light background
        .text = lv_color_hex(0x121212),          // Dark text
        .primary = lv_color_hex(0x1976D2),       // Blue primary
        .secondary = lv_color_hex(0x018786),     // Teal secondary
        .warning = lv_color_hex(0xFB8C00),       // Amber warning
        .danger = lv_color_hex(0xB00020),        // Red danger
        .gauge_normal = lv_color_hex(0x018786),  // Teal for normal range
        .gauge_warning = lv_color_hex(0xFB8C00), // Amber for warning range
        .gauge_danger = lv_color_hex(0xB00020)   // Red for danger range
    };
}

StyleManager::~StyleManager() {
    reset_styles();
}

StyleManager& StyleManager::get_instance() {
    static StyleManager instance;//TODO: this is a very interesting way of doing things, is there value to do it elsewhere?
    return instance;
}

void StyleManager::init(Theme theme) {
    log_v("...");
    init_styles();
    apply_theme_to_styles(theme);
    _current_theme = theme;
}

void StyleManager::init_styles() {    
    // Initialize all style objects
    lv_style_init(&background_style);
    lv_style_init(&text_style);
    lv_style_init(&heading_style);
    lv_style_init(&card_style);
    lv_style_init(&button_style);
    lv_style_init(&gauge_normal_style);
    lv_style_init(&gauge_warning_style);
    lv_style_init(&gauge_danger_style);
}

void StyleManager::apply_theme_to_styles(Theme theme) {    
    // Select the current theme colors
    ThemeColors* colors = (theme == Theme::Dark) 
                        ? &_dark_theme_colors 
                        : &_light_theme_colors;
    
    // Background style
    lv_style_set_bg_color(&background_style, colors->background);
    lv_style_set_bg_opa(&background_style, LV_OPA_COVER);
    
    // Text style
    lv_style_set_text_color(&text_style, colors->text);
    lv_style_set_text_opa(&text_style, LV_OPA_COVER);
    
    // Heading style (extends text style)
    lv_style_set_text_color(&heading_style, colors->primary);
    lv_style_set_text_font(&heading_style, &lv_font_montserrat_20);
    
    // Card style
    lv_style_set_bg_color(&card_style, lv_color_lighten(colors->background, 30));
    lv_style_set_bg_opa(&card_style, LV_OPA_COVER);
    lv_style_set_radius(&card_style, 10);
    lv_style_set_pad_all(&card_style, 10);
    lv_style_set_shadow_width(&card_style, 15);
    lv_style_set_shadow_color(&card_style, lv_color_black());
    lv_style_set_shadow_opa(&card_style, LV_OPA_20);
    
    // Button style
    lv_style_set_bg_color(&button_style, colors->primary);
    lv_style_set_bg_opa(&button_style, LV_OPA_COVER);
    lv_style_set_text_color(&button_style, lv_color_white());
    lv_style_set_radius(&button_style, 5);
    lv_style_set_pad_all(&button_style, 10);
    
    // Gauge styles
    lv_style_set_line_color(&gauge_normal_style, colors->gauge_normal);
    lv_style_set_line_width(&gauge_normal_style, 3);
    
    lv_style_set_line_color(&gauge_warning_style, colors->gauge_warning);
    lv_style_set_line_width(&gauge_warning_style, 3);
    
    lv_style_set_line_color(&gauge_danger_style, colors->gauge_danger);
    lv_style_set_line_width(&gauge_danger_style, 3);
}

void StyleManager::reset_styles() {
    log_d("Resetting all styles");
    
    // Reset all style objects
    lv_style_reset(&background_style);
    lv_style_reset(&text_style);
    lv_style_reset(&heading_style);
    lv_style_reset(&card_style);
    lv_style_reset(&button_style);
    lv_style_reset(&gauge_normal_style);
    lv_style_reset(&gauge_warning_style);
    lv_style_reset(&gauge_danger_style);
}

void StyleManager::switch_theme(Theme theme) {
    log_i("Switching theme from %d to %d", _current_theme, theme);
    
    if (_current_theme == theme) {
        log_d("Theme is already set to %d, no change needed", theme);
        return;
    }
    
    apply_theme_to_styles(theme);
    _current_theme = theme;
}

void StyleManager::apply_theme_to_screen(lv_obj_t* screen) {
    log_v("...");
    
    if (screen == nullptr) {
        log_w("Cannot apply theme to null screen");
        return;
    }
    
    // Apply background style to the screen
    lv_obj_add_style(screen, &background_style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(screen, &text_style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(screen, &heading_style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(screen, &card_style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(screen, &button_style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(screen, &gauge_normal_style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(screen, &gauge_warning_style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(screen, &gauge_danger_style, LV_PART_MAIN | LV_STATE_DEFAULT);
    
    // If we're using the dark theme and INVERT is defined, we need to invert the colors
    // #ifdef INVERT
    // if (_current_theme == Theme::Dark) {
    //     // For inverted displays, we need to keep black as black and white as white
    //     lv_obj_set_style_bg_color(screen, lv_color_black(), LV_PART_MAIN);
    // }
    // #endif
    //TODO: I don't think I need the above
}