#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "utilities/types.h"
#include "interfaces/i_style_service.h"

#include <lvgl.h>
#include <memory>

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
 * @design_pattern Service with Dependency Injection - managed by ServiceContainer
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
class StyleManager : public IStyleService
{
public:
    // Constructors and Destructors
    StyleManager() : initialized_(false) {}
    ~StyleManager();
    StyleManager(const StyleManager&) = delete;
    StyleManager& operator=(const StyleManager&) = delete;

    // Removed singleton pattern - StyleManager is now managed by ServiceContainer

    // Core Functionality Methods (IStyleService implementation)
    void initializeStyles() override;
    void init(const char *theme) override;
    void set_theme(const char *theme);
    void apply_theme_to_screen(lv_obj_t *screen);

    // Accessor Methods  
    const ThemeColors &get_colours(const char *theme) const;
    
    // IStyleService Interface Implementation (delegate to existing methods)
    // Note: We already have init() method, so no need to add override
    void applyThemeToScreen(lv_obj_t* screen) override { apply_theme_to_screen(screen); }
    void setTheme(const char* theme) override { set_theme(theme); }
    lv_style_t& getBackgroundStyle() override { return backgroundStyle; }
    lv_style_t& getTextStyle() override { return textStyle; }
    lv_style_t& getGaugeNormalStyle() override { return gaugeNormalStyle; }
    lv_style_t& getGaugeWarningStyle() override { return gaugeWarningStyle; }
    lv_style_t& getGaugeDangerStyle() override { return gaugeDangerStyle; }
    lv_style_t& getGaugeIndicatorStyle() override { return gaugeIndicatorStyle; }
    lv_style_t& getGaugeItemsStyle() override { return gaugeItemsStyle; }
    lv_style_t& getGaugeMainStyle() override { return gaugeMainStyle; }
    lv_style_t& getGaugeDangerSectionStyle() override { return gaugeDangerSectionStyle; }
    const char* getCurrentTheme() const override { return THEME; }
    const ThemeColors& getThemeColors() const override { return get_colours(THEME); }
    
    // Public Data Members - Theme State
    const char *THEME = Themes::NIGHT;

    // Public Data Members
    lv_style_t backgroundStyle;    // Style for backgrounds
    lv_style_t textStyle;          // Style for general text
    lv_style_t gaugeNormalStyle;  // Style for gauges in normal range
    lv_style_t gaugeWarningStyle; // Style for gauges in warning range
    lv_style_t gaugeDangerStyle;  // Style for gauges in danger range

    // Shared gauge component styles (replaces per-component allocation)
    lv_style_t gaugeIndicatorStyle;      // Style for gauge major ticks
    lv_style_t gaugeItemsStyle;          // Style for gauge minor ticks
    lv_style_t gaugeMainStyle;           // Style for gauge main part
    lv_style_t gaugeDangerSectionStyle; // Style for danger zone sections

private:

    // Core Functionality Methods
    void InitStyles();
    void ResetStyles();

    // Instance Data Members
    ThemeColors dayThemeColours_ = {
        .background = lv_color_hex(0x121212),
        .text = lv_color_hex(0xEEEEEE),
        .primary = lv_color_hex(0xEEEEEE),
        .gaugeNormal = lv_color_hex(0xEEEEEE),
        .gaugeWarning = lv_color_hex(0xFB8C00),
        .gaugeDanger = lv_color_hex(0xB00020),
        .gaugeTicks = lv_color_hex(0xF0F0E8),    // Soft off-white for ticks
        .needleNormal = lv_color_hex(0xFFFFFF),  // Pure white for needles
        .needleDanger = lv_color_hex(0xDC143C),  // Crimson red for danger
        .keyPresent = lv_color_hex(0x006400),    // Deep green for key present
        .keyNotPresent = lv_color_hex(0xDC143C) // Crimson red for key not present
    };
    ThemeColors nightThemeColours_ = {
        .background = lv_color_hex(0x000000),    // Solid black background
        .text = lv_color_hex(0xB00020),
        .primary = lv_color_hex(0xB00020),
        .gaugeNormal = lv_color_hex(0xB00020),   // Red icons in night mode
        .gaugeWarning = lv_color_hex(0xFB8C00),
        .gaugeDanger = lv_color_hex(0xB00020),
        .gaugeTicks = lv_color_hex(0xB00020),    // Red tick marks for night mode
        .needleNormal = lv_color_hex(0xFFFFFF),  // Pure white for needles
        .needleDanger = lv_color_hex(0xDC143C),  // Crimson red for danger
        .keyPresent = lv_color_hex(0x006400),    // Deep green for key present
        .keyNotPresent = lv_color_hex(0xDC143C) // Crimson red for key not present
    };

private:
    bool initialized_ = false;
};