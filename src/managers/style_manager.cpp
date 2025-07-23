#include "managers/style_manager.h"
#include <esp32-hal-log.h>
#include <cstring>

// Constructors and Destructors
StyleManager::~StyleManager()
{
    reset_styles();
}

// Static Methods
/// @brief Get the singleton instance of StyleManager
/// @return instance of StyleManager
StyleManager &StyleManager::get_instance()
{
    static StyleManager instance; // this ensures that the instance is created only once
    return instance;
}

// Core Functionality Methods
/// @brief Apply the current theme to a specific screen
/// @param screen the screen to which the theme will be applied
void StyleManager::apply_theme_to_screen(lv_obj_t *screen)
{
    log_d("...");

    // Only apply the background style to screens
    // Other styles should be applied to specific components that need them

    lv_obj_add_style(screen, &background_style, MAIN_DEFAULT);

    // Don't apply all styles to the same screen - this can cause conflicts
    // Components should apply their own specific styles as needed
}

/// @brief Initialises the styles for the application
/// @param theme the theme to be applied
void StyleManager::init(const char* theme)
{
    log_d("...");

    this->theme = theme;

    lv_style_init(&background_style);
    lv_style_init(&text_style);
    lv_style_init(&gauge_normal_style);
    lv_style_init(&gauge_warning_style);
    lv_style_init(&gauge_danger_style);
    
    // Initialize shared gauge component styles
    lv_style_init(&gauge_indicator_style);
    lv_style_init(&gauge_items_style);
    lv_style_init(&gauge_main_style);
    lv_style_init(&gauge_danger_section_style);

    set_theme(theme);

    // Don't apply to lv_scr_act() here - it might not be ready
    // apply_theme_to_screen(lv_scr_act()); // Remove this line
}

/// @brief Reset all styles to their default state
void StyleManager::reset_styles()
{
    log_d("...");

    // Reset all style objects
    lv_style_reset(&background_style);
    lv_style_reset(&text_style);
    lv_style_reset(&gauge_normal_style);
    lv_style_reset(&gauge_warning_style);
    lv_style_reset(&gauge_danger_style);
    
    // Reset shared gauge component styles
    lv_style_reset(&gauge_indicator_style);
    lv_style_reset(&gauge_items_style);
    lv_style_reset(&gauge_main_style);
    lv_style_reset(&gauge_danger_section_style);
}

/// @brief Apply a specified theme to the styles
/// @param theme the theme to be applied
void StyleManager::set_theme(const char* theme)
{
    log_d("...");

    // Select the current theme colors
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
    // Indicator style (Major ticks)
    lv_style_set_length(&gauge_indicator_style, 25);
    lv_style_set_line_width(&gauge_indicator_style, 7);
    lv_style_set_line_color(&gauge_indicator_style, colours.gauge_ticks);
    
    // Items style (Minor ticks)
    lv_style_set_length(&gauge_items_style, 18);
    lv_style_set_line_width(&gauge_items_style, 2);
    lv_style_set_line_color(&gauge_items_style, colours.gauge_ticks);
    
    // Main style (Arc)
    lv_style_set_arc_width(&gauge_main_style, 0);
    
    // Danger section style
    lv_style_set_line_width(&gauge_danger_section_style, 5);
    lv_style_set_line_color(&gauge_danger_section_style, colours.gauge_danger);
}

// Accessor Methods
/// @brief Get the colours scheme for the supplied theme
/// @param theme the theme to retrieve the colour scheme for
/// @return the colour scheme for the specified theme
const ThemeColors &StyleManager::get_colours(const char* theme) const
{
    return (theme && strcmp(theme, Themes::Night) == 0) ? _night_theme_colours : _day_theme_colours;
}