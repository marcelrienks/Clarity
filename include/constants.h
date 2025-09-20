#pragma once

/**
 * @file constants.h
 * @brief Centralized constants to replace magic strings and values
 *
 * This file contains various categories of constants used throughout the application
 * to improve maintainability, reduce magic strings, and ensure consistency.
 */

/*============================================================================*/
/*                              UI CONSTANTS                                 */
/*============================================================================*/

namespace UIStrings {

    // ========== Theme Names ==========
    namespace ThemeNames {
        static constexpr const char* DAY = "Day";
        static constexpr const char* NIGHT = "Night";
        static constexpr const char* ERROR = "Error";
    }

    // ========== Config Panel Action Types ==========
    namespace ActionTypes {
        static constexpr const char* ENTER_SECTION = "enter_section";
        static constexpr const char* TOGGLE_BOOLEAN = "toggle_boolean";
        static constexpr const char* SHOW_OPTIONS = "show_options";
        static constexpr const char* SET_CONFIG_VALUE = "set_config_value";
        static constexpr const char* BACK = "back";
        static constexpr const char* NONE = "none";
        static constexpr const char* PANEL_EXIT = "panel_exit";
        static constexpr const char* PANEL_LOAD = "panel_load";
        static constexpr const char* SUBMENU = "submenu";
    }

    // ========== Menu Labels ==========
    namespace MenuLabels {
        static constexpr const char* EXIT = "Exit";
        static constexpr const char* BACK = "Back";
        static constexpr const char* CONFIGURATION = "Configuration";
        static constexpr const char* DISPLAY = "Display";
    }

    // ========== Configuration Keys ==========
    namespace ConfigKeys {
        static constexpr const char* STYLE_MANAGER_THEME = "style_manager.theme";
        static constexpr const char* STYLE_MANAGER_SECTION = "style_manager";
    }

    // ========== Color Constants (frequently used colors) ==========
    namespace Colors {
        // Night theme colors
        static constexpr uint32_t NIGHT_BACKGROUND = 0x1A0000;        // Very dark red
        static constexpr uint32_t NIGHT_TITLE_TEXT = 0xFF6666;        // Light red
        static constexpr uint32_t NIGHT_HINT_TEXT = 0x993333;         // Darker red
        static constexpr uint32_t NIGHT_SELECTED_BG = 0x4D1F1F;       // Dark red background
        static constexpr uint32_t NIGHT_SELECTED_BORDER = 0x993333;   // Red border
        static constexpr uint32_t NIGHT_SELECTED_ITEM = 0xFF0000;     // Bright red
        static constexpr uint32_t NIGHT_BASE_COLOR = 0xB00020;        // Deep red base

        // Day theme colors
        static constexpr uint32_t DAY_TITLE_TEXT = 0xCCCCCC;          // Light gray
        static constexpr uint32_t DAY_HINT_TEXT = 0x888888;           // Gray
        static constexpr uint32_t DAY_SELECTED_BG = 0x555555;         // Dark gray background
        static constexpr uint32_t DAY_SELECTED_BORDER = 0x888888;     // Gray border
        static constexpr uint32_t DAY_SELECTED_ITEM = 0xFFFFFF;       // White
        static constexpr uint32_t DAY_BASE_COLOR = 0xEEEEEE;          // Light gray base
        static constexpr uint32_t DAY_FALLBACK = 0x888888;            // Gray fallback

        // Common colors
        static constexpr uint32_t WHITE = 0xFFFFFF;                   // Pure white
        static constexpr uint32_t BLACK = 0x000000;                   // Pure black
    }

    // ========== Component Constants ==========
    namespace Component {
        // OEM Oil Component
        static constexpr uint32_t OEM_COMPONENT_PATTERN = 0xBEEFCAFE;
        static constexpr uint32_t OEM_PIVOT_MEDIUM_GRAY = 0x505050;
        static constexpr uint32_t OEM_PIVOT_DARK_GRAY = 0x2A2A2A;
        static constexpr uint32_t OEM_PIVOT_BORDER = 0x1A1A1A;
        static constexpr uint32_t OEM_PIVOT_HIGHLIGHT = 0x707070;

        // OEM Oil Panel
        static constexpr uint32_t OEM_MEMORY_PATTERN = 0xDEADBEEF;
    }

    // ========== Default Hint Text ==========
    namespace HintText {
        static constexpr const char* SHORT_LONG_PRESS = "Short: Next | Long: Select";
    }
}

/*============================================================================*/
/*                            HARDWARE CONSTANTS                             */
/*============================================================================*/

// Future: Hardware-related constants can be added here

/*============================================================================*/
/*                            NETWORK CONSTANTS                              */
/*============================================================================*/

// Future: Network-related constants can be added here

/*============================================================================*/
/*                            TIMING CONSTANTS                               */
/*============================================================================*/

// Future: Timing-related constants can be added here