#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "utilities/types.h"

#include <lvgl.h>
#include <memory>

#define MAIN_DEFAULT LV_PART_MAIN | LV_STATE_DEFAULT
#define ITEMS_DEFAULT LV_PART_ITEMS | LV_STATE_DEFAULT
#define INDICATOR_DEFAULT LV_PART_INDICATOR | LV_STATE_DEFAULT

// Theme color definitions
struct ThemeColors
{
    lv_color_t background;      // Background color
    lv_color_t text;            // Text color
    lv_color_t primary;         // Primary accent color
    lv_color_t gauge_normal;    // Normal gauge color
    lv_color_t gauge_warning;   // Warning gauge color
    lv_color_t gauge_danger;    // Danger gauge color
    lv_color_t gauge_ticks;     // Gauge tick marks (soft off-white)
    lv_color_t needle_normal;   // Normal needle color (bright white)
    lv_color_t needle_danger;   // Danger needle color (bright red/orange)
    lv_color_t key_present;     // Normal key present clor (pure white)
    lv_color_t key_not_present; // Normal Key not present color (bright red)
};

/**
 * @class StyleManager
 * @brief Singleton theme and LVGL style management system
 *
 * @details This manager provides centralized theme management and LVGL style
 * allocation for the entire application. It implements efficient style sharing
 * to reduce memory usage and provides consistent theming across all components.
 *
 * @design_pattern Singleton - ensures consistent theming across app
 * @theme_system Day/Night themes with customizable color schemes
 * @style_sharing Shared style objects reduce memory fragmentation
 * @memory_optimization Single style instances used by multiple components
 *
 * @supported_themes:
 * - Night Theme: Dark background with red accents (default)
 * - Day Theme: Light background with white accents
 * - Extensible: Easy to add new themes
 *
 * @color_categories:
 * - background: Screen/panel background color
 * - text: General text and labels
 * - primary: Primary accent color
 * - gauge_normal: Normal gauge/indicator color
 * - gauge_warning: Warning state color (orange)
 * - gauge_danger: Critical/danger state color (red)
 *
 * @shared_styles:
 * - gauge_main_style: Main gauge background and border
 * - gauge_indicator_style: Major tick marks and indicators
 * - gauge_items_style: Minor tick marks and scale items
 * - gauge_danger_section_style: Danger zone highlighting
 *
 * @style_lifecycle:
 * 1. init(): Initialize with default theme
 * 2. set_theme(): Change theme and refresh all styles
 * 3. apply_theme_to_screen(): Apply theme to specific screens
 * 4. get_*_style(): Accessor methods for components
 *
 * @memory_efficiency:
 * - Shared style objects prevent duplication
 * - Lazy style initialization
 * - Automatic cleanup on theme changes
 *
 * @context This manager handles all theming and styling for the app.
 * Components get their styles from here to ensure consistency. The night
 * theme uses red accents while day theme uses white/neutral colors.
 */
class StyleManager
{
public:
    // Main styles that components can use
    lv_style_t background_style;    // Style for backgrounds
    lv_style_t text_style;          // Style for general text
    lv_style_t gauge_normal_style;  // Style for gauges in normal range
    lv_style_t gauge_warning_style; // Style for gauges in warning range
    lv_style_t gauge_danger_style;  // Style for gauges in danger range

    // Shared gauge component styles (replaces per-component allocation)
    lv_style_t gauge_indicator_style;      // Style for gauge major ticks
    lv_style_t gauge_items_style;          // Style for gauge minor ticks
    lv_style_t gauge_main_style;           // Style for gauge main part
    lv_style_t gauge_danger_section_style; // Style for danger zone sections

    static StyleManager &get_instance();

    void init(Themes theme);
    void set_theme(Themes theme);
    void apply_theme_to_screen(lv_obj_t *screen);

    // Shared style accessors for components
    lv_style_t *get_gauge_indicator_style() { return &gauge_indicator_style; }
    lv_style_t *get_gauge_items_style() { return &gauge_items_style; }
    lv_style_t *get_gauge_main_style() { return &gauge_main_style; }
    lv_style_t *get_gauge_danger_section_style() { return &gauge_danger_section_style; }

    const Themes &get_theme() const { return _theme; }
    const ThemeColors &get_colours(const Themes &theme) const;

private:
    Themes _theme = Themes::Night;
    ThemeColors _day_theme_colours = {
        .background = lv_color_hex(0x121212),
        .text = lv_color_hex(0xEEEEEE),
        .primary = lv_color_hex(0xEEEEEE),
        .gauge_normal = lv_color_hex(0xEEEEEE),
        .gauge_warning = lv_color_hex(0xFB8C00),
        .gauge_danger = lv_color_hex(0xB00020),
        .gauge_ticks = lv_color_hex(0xF0F0E8),    // Soft off-white for ticks
        .needle_normal = lv_color_hex(0xFFFFFF),  // Pure white for needles
        .needle_danger = lv_color_hex(0xFF0000),  // Bright red for danger
        .key_present = lv_color_hex(0xFFFFFF),    // Pure white for needles
        .key_not_present = lv_color_hex(0xFF0000) // Bright red for danger
    };
    ThemeColors _night_theme_colours = {
        .background = lv_color_hex(0x121212),
        .text = lv_color_hex(0xB00020),
        .primary = lv_color_hex(0xB00020),
        .gauge_normal = lv_color_hex(0xB00020),
        .gauge_warning = lv_color_hex(0xFB8C00),
        .gauge_danger = lv_color_hex(0xB00020),
        .gauge_ticks = lv_color_hex(0xF0F0E8),    // Soft off-white for ticks
        .needle_normal = lv_color_hex(0xFFFFFF),  // Pure white for needles
        .needle_danger = lv_color_hex(0xFF0000),  // Bright red for danger
        .key_present = lv_color_hex(0xFFFFFF),    // Pure white for needles
        .key_not_present = lv_color_hex(0xFF0000) // Bright red for danger
    };

    ~StyleManager();
    void init_styles();
    void reset_styles();
};