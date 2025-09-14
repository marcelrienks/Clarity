#pragma once

#include "interfaces/i_style_service.h"
#include "interfaces/i_preference_service.h"
#include "utilities/styles.h"
#include "utilities/types.h"

#include <lvgl.h>
#include <memory>
#include <string>

#define MAIN_DEFAULT LV_PART_MAIN | LV_STATE_DEFAULT
#define ITEMS_DEFAULT LV_PART_ITEMS | LV_STATE_DEFAULT
#define INDICATOR_DEFAULT LV_PART_INDICATOR | LV_STATE_DEFAULT

/**
 * @class StyleManager
 * @brief Theme and LVGL style management service
 *
 * @details This service provides centralized theme management and LVGL style
 * allocation for the entire application. It implements efficient style sharing
 * to reduce memory usage and provides consistent theming across all components.
 *
 * @design_pattern Service with Factory Pattern - created by ManagerFactory
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
 * 1. Init(): Initialize with default theme
 * 2. SetTheme(): Change theme and refresh all styles
 * 3. ApplyThemeToScreen(): Apply theme to specific screens
 * 4. Get*Style(): Accessor methods for components
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
class StyleManager : public IStyleService
{
public:
    // ========== Constructors and Destructor ==========
    /**
     * @brief Construct StyleManager with initial theme
     * @param theme Initial theme name (e.g., "night", "day")
     */
    explicit StyleManager(const char *theme);
    StyleManager(const StyleManager &) = delete;
    StyleManager &operator=(const StyleManager &) = delete;
    /**
     * @brief Destructor - cleans up LVGL style objects
     */
    ~StyleManager();

    // ========== Static Methods ==========
    /**
     * @brief Get singleton instance for interrupt architecture
     * @return Reference to the global StyleManager instance
     */
    static StyleManager& Instance();

    // ========== Public Interface Methods ==========
    /**
     * @brief Initialize all LVGL style objects and apply initial theme
     * @details Must be called after LVGL initialization
     */
    void InitializeStyles() override;

    /**
     * @brief Change the current theme and refresh all styles
     * @param theme Theme name to switch to (e.g., "night", "day")
     */
    void SetTheme(const char *theme) override;

    /**
     * @brief Apply background theme styles to a specific screen
     * @param screen LVGL screen object to apply theme to
     */
    void ApplyThemeToScreen(lv_obj_t *screen) override;

    /**
     * @brief Refresh theme from preferences and apply changes
     * @details Reads current theme from preference service and updates if changed
     */
    void ApplyCurrentTheme() override;

    /**
     * @brief Get color scheme for a specified theme
     * @param theme Theme name to get colors for
     * @return ThemeColors structure containing all theme colors
     */
    const ThemeColors &GetColours(const std::string& theme) const;

    /**
     * @brief Get shared background style object
     * @return Reference to the background LVGL style
     */
    lv_style_t &GetBackgroundStyle() override
    {
        return backgroundStyle_;
    }

    /**
     * @brief Get shared text style object
     * @return Reference to the text LVGL style
     */
    lv_style_t &GetTextStyle() override
    {
        return textStyle_;
    }

    /**
     * @brief Get gauge style for normal/safe operating range
     * @return Reference to the normal gauge LVGL style
     */
    lv_style_t &GetGaugeNormalStyle() override
    {
        return gaugeNormalStyle_;
    }

    /**
     * @brief Get gauge style for warning operating range
     * @return Reference to the warning gauge LVGL style
     */
    lv_style_t &GetGaugeWarningStyle() override
    {
        return gaugeWarningStyle_;
    }

    /**
     * @brief Get gauge style for danger/critical operating range
     * @return Reference to the danger gauge LVGL style
     */
    lv_style_t &GetGaugeDangerStyle() override
    {
        return gaugeDangerStyle_;
    }

    /**
     * @brief Get shared gauge indicator style for major tick marks
     * @return Reference to the gauge indicator LVGL style
     */
    lv_style_t &GetGaugeIndicatorStyle() override
    {
        return gaugeIndicatorStyle_;
    }

    /**
     * @brief Get shared gauge items style for minor tick marks
     * @return Reference to the gauge items LVGL style
     */
    lv_style_t &GetGaugeItemsStyle() override
    {
        return gaugeItemsStyle_;
    }

    /**
     * @brief Get shared gauge main style for arc background
     * @return Reference to the gauge main LVGL style
     */
    lv_style_t &GetGaugeMainStyle() override
    {
        return gaugeMainStyle_;
    }

    /**
     * @brief Get shared gauge danger section highlighting style
     * @return Reference to the gauge danger section LVGL style
     */
    lv_style_t &GetGaugeDangerSectionStyle() override
    {
        return gaugeDangerSectionStyle_;
    }

    /**
     * @brief Get name of currently active theme
     * @return String reference to current theme name
     */
    const std::string& GetCurrentTheme() const override;

    /**
     * @brief Get color scheme of currently active theme
     * @return Reference to current theme's color structure
     */
    const ThemeColors &GetThemeColors() const override
    {
        return GetColours(GetCurrentTheme());
    }

    /**
     * @brief Check if StyleManager has been properly initialized
     * @return true if initialized, false otherwise
     */
    bool IsInitialized() const override
    {
        return initialized_;
    }

    /**
     * @brief Switch to a different theme immediately
     * @param themeName Name of theme to switch to
     * @details Direct theme switching for panel actions
     */
    void SwitchTheme(const char* themeName);

    /**
     * @brief Inject preference service for theme persistence
     * @param preferenceService Pointer to preference service instance
     */
    void SetPreferenceService(IPreferenceService* preferenceService);

private:
    // ========== Private Methods ==========
    /**
     * @brief Reset all LVGL style objects to default state
     */
    void ResetStyles();

    // ========== Private Data Members ==========
    std::string theme_ = Themes::NIGHT;
    bool initialized_ = false;

    // Core Style Objects
    lv_style_t backgroundStyle_;   // Style for backgrounds
    lv_style_t textStyle_;         // Style for general text
    lv_style_t gaugeNormalStyle_;  // Style for gauges in normal range
    lv_style_t gaugeWarningStyle_; // Style for gauges in warning range
    lv_style_t gaugeDangerStyle_;  // Style for gauges in danger range

    // Shared gauge component styles (replaces per-component allocation)
    lv_style_t gaugeIndicatorStyle_;     // Style for gauge major ticks
    lv_style_t gaugeItemsStyle_;         // Style for gauge minor ticks
    lv_style_t gaugeMainStyle_;          // Style for gauge main part
    lv_style_t gaugeDangerSectionStyle_; // Style for danger zone sections

    // Theme color references from styles.h
    const ThemeColors& dayThemeColours_ = ThemeDefinitions::DAY_THEME;
    const ThemeColors& nightThemeColours_ = ThemeDefinitions::NIGHT_THEME;
    const ThemeColors& errorThemeColours_ = ThemeDefinitions::ERROR_THEME;

    // Direct preference reading support
    IPreferenceService* preferenceService_ = nullptr;
};