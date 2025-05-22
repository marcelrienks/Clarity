#include "managers/style_manager.h"
#include <esp32-hal-log.h>

StyleManager::~StyleManager()
{
    reset_styles();
}

/// @brief Get the singleton instance of StyleManager
/// @return instance of StyleManager
StyleManager &StyleManager::get_instance()
{
    static StyleManager instance; // this ensures that the instance is created only once
    return instance;
}

/// @brief Initialises the styles for the application
/// @param theme the theme to be applied
void StyleManager::init(Theme &theme)
{
    log_d("...");

    _theme = theme;

    lv_style_init(&background_style);
    lv_style_init(&text_style);
    lv_style_init(&gauge_normal_style);
    lv_style_init(&gauge_warning_style);
    lv_style_init(&gauge_danger_style);

    apply_theme(_theme);
    apply_theme_to_screen(lv_scr_act());
}

/// @brief Apply a specified theme to the styles
/// @param theme the theme to be applied
void StyleManager::apply_theme(Theme &theme)
{
    log_d("...");

    // Select the current theme colors
    ThemeColors colours = StyleManager::get_colours(theme);

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
}

/// @brief Apply the current theme to a specific screen
/// @param screen the screen to which the theme will be applied
void StyleManager::apply_theme_to_screen(lv_obj_t *screen)
{
    log_d("...");

    // Apply style to the screen
    lv_obj_add_style(screen, &background_style, MAIN_DEFAULT);
    lv_obj_add_style(screen, &text_style, MAIN_DEFAULT);
    lv_obj_add_style(screen, &gauge_normal_style, MAIN_DEFAULT);
    lv_obj_add_style(screen, &gauge_warning_style, MAIN_DEFAULT);
    lv_obj_add_style(screen, &gauge_danger_style, MAIN_DEFAULT);
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
}

/// @brief Get the colours scheme for the supplied theme
/// @param theme the theme to retrieve the colour scheme for
/// @return the colour scheme for the specified theme
const ThemeColors &StyleManager::get_colours(const Theme &theme) const
{
    return theme == Theme::Night ? _night_theme_colours : _day_theme_colours;
}