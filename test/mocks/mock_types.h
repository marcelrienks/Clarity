#pragma once
#include <cstdint>
#include "mock_colors.h"

// MOCK: LVGL Types and Constants
typedef void* lv_obj_t;
typedef mock_lv_style_t lv_style_t;
typedef mock_lv_color_t lv_color_t;

// Panel Names from types.h
namespace PanelNames {
    constexpr const char* OIL = "OemOilPanel";
    constexpr const char* KEY = "KeyPanel";
    constexpr const char* LOCK = "LockPanel";
}

// Theme Names from types.h
namespace Themes {
    constexpr const char* DAY = "Day";
    constexpr const char* NIGHT = "Night";
}

// Type definitions needed for testing
using Reading = int32_t;
enum class KeyState { Present, NotPresent, Inactive };

// Trigger Types from types.h
constexpr const char* TRIGGER_KEY_PRESENT = "KEY_PRESENT";
constexpr const char* TRIGGER_KEY_NOT_PRESENT = "KEY_NOT_PRESENT";
constexpr const char* TRIGGER_LOCK_STATE = "LOCK_STATE";
constexpr const char* TRIGGER_THEME_STATE = "THEME_STATE";

enum class TriggerActionType { LoadPanel, ToggleTheme };
enum class TriggerPriority { CRITICAL, IMPORTANT, NORMAL };
