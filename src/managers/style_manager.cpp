#include "managers/style_manager.h"
#include <cstring>
#include <esp32-hal-log.h>

// Constructors and Destructors
StyleManager::StyleManager(const char *theme) : THEME(theme), initialized_(false)
{
    log_v("StyleManager() constructor called");
    log_d("Creating StyleManager with theme: %s (LVGL styles will be initialized later)", theme);
}

StyleManager::~StyleManager()
{
    log_v("~StyleManager() destructor called");
    ResetStyles();
}

// Core Functionality Methods

void StyleManager::InitializeStyles()
{
    log_v("InitializeStyles() called");
    if (initialized_)
    {
        log_d("StyleManager styles already initialized");
        return;
    }

    log_d("Initializing LVGL styles now that LVGL is ready: %s", THEME.c_str());

    // Initialize LVGL style objects (must be done after LVGL init)
    lv_style_init(&backgroundStyle);
    lv_style_init(&textStyle);
    lv_style_init(&gaugeNormalStyle);
    lv_style_init(&gaugeWarningStyle);
    lv_style_init(&gaugeDangerStyle);

    // Initialize shared gauge component styles
    lv_style_init(&gaugeIndicatorStyle);
    lv_style_init(&gaugeItemsStyle);
    lv_style_init(&gaugeMainStyle);
    lv_style_init(&gaugeDangerSectionStyle);

    // Apply theme colors after style objects are initialized
    SetTheme(THEME.c_str());
    initialized_ = true;

    log_d("StyleManager styles initialized successfully");
}

/// @brief Apply the current theme to a specific screen
/// @param screen the screen to which the theme will be applied
void StyleManager::ApplyThemeToScreen(lv_obj_t *screen)
{
    log_v("ApplyThemeToScreen() called");

    // Safety checks
    if (!screen)
    {
        log_w("Cannot apply theme to null screen");
        return;
    }

    if (!initialized_)
    {
        log_w("StyleManager not initialized, skipping theme application");
        return;
    }

    // Only apply the background style to screens
    // Other styles should be applied to specific components that need them
    lv_obj_add_style(screen, &backgroundStyle, MAIN_DEFAULT);

    // Don't apply all styles to the same screen - this can cause conflicts
    // Components should apply their own specific styles as needed
}

/// @brief Initialises the styles for the application
/// @param theme the theme to be applied

/// @brief Reset all styles to their default state
void StyleManager::ResetStyles()
{
    log_v("ResetStyles() called");
    log_d("Resetting all LVGL styles to default state");

    // Reset all style objects
    lv_style_reset(&backgroundStyle);
    lv_style_reset(&textStyle);
    lv_style_reset(&gaugeNormalStyle);
    lv_style_reset(&gaugeWarningStyle);
    lv_style_reset(&gaugeDangerStyle);

    // Reset shared gauge component styles
    lv_style_reset(&gaugeIndicatorStyle);
    lv_style_reset(&gaugeItemsStyle);
    lv_style_reset(&gaugeMainStyle);
    lv_style_reset(&gaugeDangerSectionStyle);
}

/// @brief Apply a specified theme to the styles
/// @param theme the theme to be applied
void StyleManager::SetTheme(const char *theme)
{
    log_v("SetTheme() called");
    // Handle invalid theme gracefully
    if (!theme || strlen(theme) == 0)
    {
        log_d("Invalid theme provided, keeping current theme: %s", THEME.c_str());
        return;
    }

    log_d("Switching application theme to: %s", theme);

    // Update current theme
    this->THEME = theme;

    // Select the current theme colors
    ThemeColors colours = GetColours(theme);

    // Background style
    lv_style_set_bg_color(&backgroundStyle, colours.background);
    lv_style_set_bg_opa(&backgroundStyle, LV_OPA_COVER);

    // Text style
    lv_style_set_text_color(&textStyle, colours.text);
    lv_style_set_text_opa(&textStyle, LV_OPA_COVER);

    // Gauge styles
    lv_style_set_line_color(&gaugeNormalStyle, colours.gaugeNormal);
    lv_style_set_line_color(&gaugeWarningStyle, colours.gaugeWarning);
    lv_style_set_line_color(&gaugeDangerStyle, colours.gaugeDanger);

    // Configure shared gauge component styles
    // Indicator style (Major ticks)
    lv_style_set_length(&gaugeIndicatorStyle, 25);
    lv_style_set_line_width(&gaugeIndicatorStyle, 7);
    lv_style_set_line_color(&gaugeIndicatorStyle, colours.gaugeTicks);

    // Items style (Minor ticks)
    lv_style_set_length(&gaugeItemsStyle, 18);
    lv_style_set_line_width(&gaugeItemsStyle, 2);
    lv_style_set_line_color(&gaugeItemsStyle, colours.gaugeTicks);

    // Main style (Arc)
    lv_style_set_arc_width(&gaugeMainStyle, 0);

    // Danger section style
    lv_style_set_line_width(&gaugeDangerSectionStyle, 5);
    lv_style_set_line_color(&gaugeDangerSectionStyle, colours.gaugeDanger);

    lv_obj_t *current_screen = lv_scr_act();
    if (current_screen != nullptr) {
        ApplyThemeToScreen(current_screen);
        // Force complete refresh of all child objects recursively
        lv_obj_refresh_style(current_screen, LV_PART_ANY, LV_STYLE_PROP_ANY);
        // Force LVGL to refresh the display
        lv_obj_invalidate(current_screen);
    }
}

// Accessor Methods
/// @brief Get the colours scheme for the supplied theme
/// @param theme the theme to retrieve the colour scheme for
/// @return the colour scheme for the specified theme
const ThemeColors &StyleManager::GetColours(const char *theme) const
{
    log_v("GetColours() called");
    if (!theme)
        return dayThemeColours_;

    if (strcmp(theme, Themes::NIGHT) == 0) return nightThemeColours_;
    if (strcmp(theme, Themes::ERROR) == 0) return errorThemeColours_;
    
    return dayThemeColours_;
}

/// @brief Get function for theme switching that panels can use in their actions
std::function<void(const char *)> StyleManager::GetThemeSwitchFunction()
{
    log_v("GetThemeSwitchFunction() called");
    return [this](const char *theme)
    {
        log_i("StyleManager: Direct theme switch requested to: %s", theme);
        this->SetTheme(theme);
    };
}