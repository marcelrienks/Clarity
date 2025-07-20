#pragma once

#include <stdint.h>

/// @brief UI state for Core 0 processing decisions
enum class UIState {
    IDLE,        ///< Safe to process all messages immediately
    UPDATING,    ///< Throttled processing (high/medium priority only)
    LOADING,     ///< No message processing
    LVGL_BUSY    ///< No message processing
};

/// @brief Priority levels for trigger messages
enum class TriggerPriority {
    CRITICAL = 0,  ///< Critical triggers (key presence, safety)
    IMPORTANT = 1, ///< Important triggers (lock state, system modes)
    NORMAL = 2     ///< Non-critical triggers (theme changes, settings)
};

/// @brief Message structure for trigger communication between cores
struct TriggerMessage {
    char trigger_id[32];    ///< Unique trigger identifier
    char action[32];        ///< Action to perform ("LoadPanel", "RestorePreviousPanel", "ChangeTheme")
    char target[32];        ///< Target panel/theme name
    TriggerPriority priority; ///< Message priority level
    uint32_t timestamp;     ///< Message creation time
    
    TriggerMessage() : priority(TriggerPriority::NORMAL), timestamp(0) {
        trigger_id[0] = '\0';
        action[0] = '\0';
        target[0] = '\0';
    }
};

/// @brief Configuration constants
constexpr int HIGH_PRIORITY_QUEUE_SIZE = 15;
constexpr int MEDIUM_PRIORITY_QUEUE_SIZE = 15;
constexpr int LOW_PRIORITY_QUEUE_SIZE = 15;
constexpr int PANEL_STATE_MUTEX_TIMEOUT = 100;
constexpr int THEME_STATE_MUTEX_TIMEOUT = 100;
constexpr TriggerPriority UPDATING_STATE_MAX_PRIORITY = TriggerPriority::IMPORTANT;

/// @brief Action constants
constexpr const char* ACTION_LOAD_PANEL = "LoadPanel";
constexpr const char* ACTION_RESTORE_PREVIOUS_PANEL = "RestorePreviousPanel";
constexpr const char* ACTION_CHANGE_THEME = "ChangeTheme";

/// @brief Panel name constants
constexpr const char* PANEL_KEY = "KeyPanel";
constexpr const char* PANEL_LOCK = "LockPanel";
constexpr const char* PANEL_OIL = "OemOilPanel";
constexpr const char* PANEL_SPLASH = "SplashPanel";

/// @brief Theme name constants
constexpr const char* THEME_DAY = "Day";
constexpr const char* THEME_NIGHT = "Night";

/// @brief Trigger ID constants
constexpr const char* TRIGGER_KEY_PRESENT = "key_present";
constexpr const char* TRIGGER_LOCK_STATE = "lock_state";
constexpr const char* TRIGGER_THEME_SWITCH = "theme_switch";