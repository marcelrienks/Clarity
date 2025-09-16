#pragma once

/**
 * @file styles.h
 * @brief Centralized style utilities and color definitions for the Clarity automotive gauge system
 * 
 * @details This header contains style-related utilities including:
 * - Theme color structure definitions
 * - Error level color mapping functions
 * - Centralized color constants and style helpers
 * 
 * @organization:
 * - ThemeColors: Color scheme structure for different themes
 * - Style Utilities: Helper functions for color mapping and styling
 * 
 * @note This file centralizes style utilities for consistent theming across components
 * @note For compile-time constants, see constants.h; for mutable types, see types.h
 */

// System/Library Includes
#ifdef LVGL_MOCK
    #include "lvgl_mock.h"
#else
    #include <lvgl.h>
#endif

// Project Includes
#include "utilities/constants.h"

//=============================================================================
// THEME COLOR DEFINITIONS
// Color schemes and theme-specific color structures
//=============================================================================

/// @struct ThemeColors
/// @brief Color scheme structure for theme management
///
/// @details Defines all colors used by a specific theme including background,
/// text, gauges, and status indicators. Used by StyleManager to provide
/// consistent theming across all components.
///
/// @color_categories:
/// - background: Screen/panel background color
/// - text: General text and labels
/// - primary: Primary accent color
/// - gauge colors: Normal, warning, and danger state colors
/// - needle colors: Normal and danger needle colors
/// - key colors: Key presence status indicators
struct ThemeColors
{
    lv_color_t background;    ///< Background color
    lv_color_t text;          ///< Text color
    lv_color_t primary;       ///< Primary accent color
    lv_color_t gaugeNormal;   ///< Normal gauge color
    lv_color_t gaugeWarning;  ///< Warning gauge color
    lv_color_t gaugeDanger;   ///< Danger gauge color
    lv_color_t gaugeTicks;    ///< Gauge tick marks (soft off-white)
    lv_color_t needleNormal;  ///< Normal needle color (bright white)
    lv_color_t needleDanger;  ///< Danger needle color (bright red/orange)
    lv_color_t keyPresent;    ///< Normal key present color (pure white)
    lv_color_t keyNotPresent; ///< Normal Key not present color (bright red)
    lv_color_t lockEngaged;   ///< Lock engaged color (red)
};

//=============================================================================
// PREDEFINED THEME COLOR SCHEMES
// Complete theme definitions for all supported visual themes
//=============================================================================

namespace ThemeDefinitions
{
    /// @brief Day theme colors - Light background with white accents
    /// @details Provides a bright, readable interface for daytime use
    inline const ThemeColors DAY_THEME = {
        .background = lv_color_hex(0x121212),   ///< Dark gray background for readability
        .text = lv_color_hex(0xEEEEEE),         ///< Light gray text
        .primary = lv_color_hex(0xEEEEEE),      ///< Light gray primary accent
        .gaugeNormal = lv_color_hex(0xEEEEEE),  ///< Light gray for normal gauges
        .gaugeWarning = lv_color_hex(0xFB8C00), ///< Orange for warning states
        .gaugeDanger = lv_color_hex(0xB00020),  ///< Deep red for danger states
        .gaugeTicks = lv_color_hex(0xF0F0E8),   ///< Soft off-white for tick marks
        .needleNormal = lv_color_hex(0xFFFFFF), ///< Pure white for normal needles
        .needleDanger = lv_color_hex(0xDC143C), ///< Crimson red for danger needles
        .keyPresent = lv_color_hex(0x006400),   ///< Deep green for key present
        .keyNotPresent = lv_color_hex(0xDC143C), ///< Crimson red for key not present
        .lockEngaged = lv_color_hex(0xDC143C)   ///< Crimson red for lock engaged
    };

    /// @brief Night theme colors - Black background with red accents
    /// @details Optimized for nighttime driving with minimal eye strain
    inline const ThemeColors NIGHT_THEME = {
        .background = lv_color_hex(0x000000),   ///< Solid black background
        .text = lv_color_hex(0xB00020),         ///< Deep red text for night vision
        .primary = lv_color_hex(0xB00020),      ///< Deep red primary accent
        .gaugeNormal = lv_color_hex(0xB00020),  ///< Red icons for night mode
        .gaugeWarning = lv_color_hex(0xFB8C00), ///< Orange for warning states
        .gaugeDanger = lv_color_hex(0xB00020),  ///< Deep red for danger states
        .gaugeTicks = lv_color_hex(0xB00020),   ///< Red tick marks for night mode
        .needleNormal = lv_color_hex(0xFFFFFF), ///< Pure white for normal needles
        .needleDanger = lv_color_hex(0xDC143C), ///< Crimson red for danger needles
        .keyPresent = lv_color_hex(0x006400),   ///< Deep green for key present
        .keyNotPresent = lv_color_hex(0xDC143C), ///< Crimson red for key not present
        .lockEngaged = lv_color_hex(0xDC143C)   ///< Crimson red for lock engaged
    };

    /// @brief Error theme colors - High contrast for error display
    /// @details Maximum readability and attention-grabbing colors for error states
    inline const ThemeColors ERROR_THEME = {
        .background = lv_color_hex(0x000000),   ///< Black background for high contrast
        .text = lv_color_hex(0xFFFFFF),         ///< White text for maximum readability
        .primary = lv_color_hex(0xFF0000),      ///< Bright red for error emphasis
        .gaugeNormal = lv_color_hex(0xFFFFFF),  ///< White for general elements
        .gaugeWarning = lv_color_hex(0xFFFF00), ///< Yellow for warning states
        .gaugeDanger = lv_color_hex(0xFF0000),  ///< Red for critical errors
        .gaugeTicks = lv_color_hex(0xFFFFFF),   ///< White tick marks
        .needleNormal = lv_color_hex(0xFFFFFF), ///< White needles
        .needleDanger = lv_color_hex(0xFF0000), ///< Red for danger needles
        .keyPresent = lv_color_hex(0xFFFFFF),   ///< White (not used in error panel)
        .keyNotPresent = lv_color_hex(0xFFFFFF), ///< White (not used in error panel)
        .lockEngaged = lv_color_hex(0xFF0000)   ///< Bright red for lock engaged
    };
}

//=============================================================================
// STYLE UTILITY FUNCTIONS
// Helper functions for color mapping and style operations
//=============================================================================

namespace StyleUtils
{
    inline lv_color_t GetErrorColor(ErrorLevel level)
    {
        switch (level)
        {
            case ErrorLevel::CRITICAL:
                return lv_color_hex(0xFF0000); // Red
            case ErrorLevel::ERROR:
                return lv_color_hex(0xFFFF00); // Yellow
            case ErrorLevel::WARNING:
                return lv_color_hex(0xFFFFFF); // White
            default:
                return lv_color_hex(0xFFFFFF); // Default to white
        }
    }
}