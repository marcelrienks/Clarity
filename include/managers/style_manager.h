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
    lv_color_t background;    // Background color
    lv_color_t text;          // Text color
    lv_color_t primary;       // Primary accent color
    lv_color_t gauge_normal;  // Normal gauge color
    lv_color_t gauge_warning; // Warning gauge color
    lv_color_t gauge_danger;  // Danger gauge color
};

class StyleManager
{
public:
    // Main styles that components can use
    lv_style_t background_style;    // Style for backgrounds
    lv_style_t text_style;          // Style for general text
    lv_style_t gauge_normal_style;  // Style for gauges in normal range
    lv_style_t gauge_warning_style; // Style for gauges in warning range
    lv_style_t gauge_danger_style;  // Style for gauges in danger range

    static StyleManager &get_instance();

    void init(Themes theme);
    void set_theme(Themes theme);
    void apply_theme_to_screen(lv_obj_t *screen);
    
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
        .gauge_danger = lv_color_hex(0xB00020)};
    ThemeColors _night_theme_colours = {
        .background = lv_color_hex(0x121212),
        .text = lv_color_hex(0xB00020),
        .primary = lv_color_hex(0xB00020),
        .gauge_normal = lv_color_hex(0xB00020),
        .gauge_warning = lv_color_hex(0xFB8C00),
        .gauge_danger = lv_color_hex(0xB00020)};

    ~StyleManager();
    void init_styles();
    void reset_styles();
};