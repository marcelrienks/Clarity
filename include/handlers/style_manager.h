#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "utilities/types.h"

#include <lvgl.h>
#include <memory>

#define MAIN_DEFAULT LV_PART_MAIN | LV_STATE_DEFAULT
#define INDICATOR_DEFAULT LV_PART_INDICATOR | LV_STATE_DEFAULT

// Theme color definitions
struct ThemeColors {
    lv_color_t background;     // Background color
    lv_color_t text;           // Text color
    lv_color_t primary;        // Primary accent color
    lv_color_t secondary;      // Secondary accent color
    lv_color_t warning;        // Warning color
    lv_color_t danger;         // Danger/Error color
    lv_color_t gauge_normal;   // Normal gauge color
    lv_color_t gauge_warning;  // Warning gauge color
    lv_color_t gauge_danger;   // Danger gauge color
};

class StyleManager {
public:
    StyleManager();
    ~StyleManager();

    // Initialize styles based on the current theme
    void init(Theme theme);
    
    // Switch between themes
    void switch_theme(Theme new_theme);
    
    // Get current theme
    Theme get_current_theme() const { return _current_theme; }

    // Main styles that components can use
    lv_style_t background_style;       // Style for backgrounds
    lv_style_t text_style;             // Style for general text
    lv_style_t heading_style;          // Style for headings
    lv_style_t gauge_normal_style;     // Style for gauges in normal range
    lv_style_t gauge_warning_style;    // Style for gauges in warning range
    lv_style_t gauge_danger_style;     // Style for gauges in danger range
    
    // Get current theme colors
    const ThemeColors& get_current_colors() const { 
        return _current_theme == Theme::Dark ? _dark_theme_colors : _light_theme_colors; 
    }

    // Convenience function to apply theme to the current screen
    void apply_theme_to_screen(lv_obj_t* screen);

    // Singleton pattern
    static StyleManager& get_instance();

private:
    // Current theme
    Theme _current_theme;
    
    // Theme-specific color palettes
    ThemeColors _dark_theme_colors = {
        .background = lv_color_hex(0x121212),
        .text = lv_color_hex(0xEEEEEE),
        .primary = lv_color_hex(0x121212),
        .secondary = lv_color_hex(0xEEEEEE),
        .warning = lv_color_hex(0xFB8C00),
        .danger = lv_color_hex(0xB00020),
        .gauge_normal = lv_color_hex(0xEEEEEE),
        .gauge_warning = lv_color_hex(0xFB8C00),
        .gauge_danger = lv_color_hex(0xB00020)
    };
    ThemeColors _light_theme_colors = {
        .background = lv_color_hex(0xEEEEEE),
        .text = lv_color_hex(0x121212),
        .primary = lv_color_hex(0xEEEEEE),
        .secondary = lv_color_hex(0x121212),
        .warning = lv_color_hex(0xFB8C00),
        .danger = lv_color_hex(0xB00020),
        .gauge_normal = lv_color_hex(0x121212),
        .gauge_warning = lv_color_hex(0xFB8C00),
        .gauge_danger = lv_color_hex(0xB00020)
    };
    
    // Initialize all style objects
    void init_styles();
    
    // Update styles with current theme colors
    void apply_theme_to_styles(Theme new_theme);
    
    // Clean up all styles
    void reset_styles();
};