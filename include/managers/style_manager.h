#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "utilities/types.h"
#include "interfaces/i_style_service.h"

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
    // Constructors and Destructors
    explicit StyleManager(const char* theme);
    ~StyleManager();
    StyleManager(const StyleManager&) = delete;
    StyleManager& operator=(const StyleManager&) = delete;

    // Core Functionality Methods (IStyleService implementation)
    void InitializeStyles() override;
    void SetTheme(const char *theme) override;
    void ApplyThemeToScreen(lv_obj_t *screen) override;

    // Accessor Methods  
    const ThemeColors &GetColours(const char *theme) const;
    lv_style_t& GetBackgroundStyle() override { return backgroundStyle; }
    lv_style_t& GetTextStyle() override { return textStyle; }
    lv_style_t& GetGaugeNormalStyle() override { return gaugeNormalStyle; }
    lv_style_t& GetGaugeWarningStyle() override { return gaugeWarningStyle; }
    lv_style_t& GetGaugeDangerStyle() override { return gaugeDangerStyle; }
    lv_style_t& GetGaugeIndicatorStyle() override { return gaugeIndicatorStyle; }
    lv_style_t& GetGaugeItemsStyle() override { return gaugeItemsStyle; }
    lv_style_t& GetGaugeMainStyle() override { return gaugeMainStyle; }
    lv_style_t& GetGaugeDangerSectionStyle() override { return gaugeDangerSectionStyle; }
    const char* GetCurrentTheme() const override { return THEME.c_str(); }
    const ThemeColors& GetThemeColors() const override { return GetColours(THEME.c_str()); }
    bool IsInitialized() const override { return initialized_; }
    
    // Public Data Members - Theme State
    std::string THEME = Themes::NIGHT;

    // Core Style Objects
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
    ThemeColors errorThemeColours_ = {
        .background = lv_color_hex(0x000000),    // Black background for high contrast
        .text = lv_color_hex(0xFFFFFF),          // White text for maximum readability
        .primary = lv_color_hex(0xFF0000),       // Bright red for error emphasis
        .gaugeNormal = lv_color_hex(0xFFFFFF),   // White for general elements
        .gaugeWarning = lv_color_hex(0xFFFF00),  // Yellow for warnings
        .gaugeDanger = lv_color_hex(0xFF0000),   // Red for critical errors
        .gaugeTicks = lv_color_hex(0xFFFFFF),    // White tick marks
        .needleNormal = lv_color_hex(0xFFFFFF),  // White needles
        .needleDanger = lv_color_hex(0xFF0000),  // Red for danger
        .keyPresent = lv_color_hex(0xFFFFFF),    // White (not used in error panel)
        .keyNotPresent = lv_color_hex(0xFFFFFF)  // White (not used in error panel)
    };

private:
    bool initialized_ = false;
};