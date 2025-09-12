#include "managers/style_manager.h"
#include "utilities/logging.h"  // For log_t()
#include <cstring>
#include <esp32-hal-log.h>

// Static instance for singleton pattern
static StyleManager* styleInstancePtr_ = nullptr;

// Constructors and Destructors
StyleManager::StyleManager(const char *theme) : THEME(theme), initialized_(false)
{
    log_v("StyleManager() constructor called");
    
    // Set singleton instance
    styleInstancePtr_ = this;
}

StyleManager::~StyleManager()
{
    log_v("~StyleManager() destructor called");
    ResetStyles();
    
    // Clear singleton instance
    if (styleInstancePtr_ == this) {
        styleInstancePtr_ = nullptr;
    }
}

// Core Functionality Methods

void StyleManager::InitializeStyles()
{
    log_v("InitializeStyles() called");
    if (initialized_)
    {
        return;
    }


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
    // Pull theme from preferences if available, otherwise use default
    if (preferenceService_) {
        const auto& config = preferenceService_->GetConfig();
        SetTheme(config.theme.c_str());
    } else {
        SetTheme(THEME.c_str());
    }
    initialized_ = true;

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
    log_t("SetTheme() called");
    // Handle invalid theme gracefully
    if (!theme || strlen(theme) == 0)
    {
        return;
    }


    // Note: THEME member is no longer updated here - it serves only as a fallback
    // The source of truth is always the preference service

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
const ThemeColors &StyleManager::GetColours(const std::string& theme) const
{
    log_v("GetColours() called");
    if (theme.empty())
        return dayThemeColours_;

    if (theme == Themes::NIGHT) return nightThemeColours_;
    if (theme == Themes::ERROR) return errorThemeColours_;
    
    return dayThemeColours_;
}

/// @brief Singleton access for new interrupt architecture
StyleManager& StyleManager::Instance() {
    if (!styleInstancePtr_) {
        log_e("StyleManager::Instance() called before initialization");
        // In embedded systems, we need a valid instance
        // This should be set during ManagerFactory initialization
    }
    return *styleInstancePtr_;
}

/// @brief Get function for theme switching that panels can use in their actions
void StyleManager::SwitchTheme(const char* themeName)
{
    log_v("SwitchTheme() called");
    log_v("Direct theme switch requested to: %s", themeName);
    SetTheme(themeName);
}

/// @brief Inject PreferenceService for direct preference reading
void StyleManager::SetPreferenceService(IPreferenceService* preferenceService)
{
    log_v("SetPreferenceService() called");
    preferenceService_ = preferenceService;
}

/// @brief Apply current theme from preferences and refresh styles
void StyleManager::ApplyCurrentTheme()
{
    log_i("StyleManager::ApplyCurrentTheme() called");
    
    if (!preferenceService_) {
        log_e("StyleManager: No PreferenceService available - cannot read theme from preferences!");
        log_v("Current cached theme is: %s", THEME.c_str());
        return;
    }
    
    // Read theme directly from preferences
    const auto& config = preferenceService_->GetConfig();
    log_v("Read theme from preferences: %s (cached: %s)", config.theme.c_str(), THEME.c_str());
    
    // Apply the theme if it's different from cached value
    if (THEME != config.theme) {
        log_t("StyleManager: Theme changed from %s to %s - applying new theme", THEME.c_str(), config.theme.c_str());
        SetTheme(config.theme.c_str());
    } else {
        log_v("Theme unchanged (%s) - no update needed", config.theme.c_str());
    }
}

const std::string& StyleManager::GetCurrentTheme() const
{
    // Always pull theme directly from preferences to ensure consistency
    if (preferenceService_) {
        const auto& config = preferenceService_->GetConfig();
        return config.theme;
    }
    
    // Fallback to cached value if preference service not available
    log_w("StyleManager: PreferenceService not available, using cached theme: %s", THEME.c_str());
    return THEME;
}