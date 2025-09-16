#pragma once

/**
 * @file constants.h
 * @brief Immutable constants, enums, and static configuration data for the Clarity automotive gauge system
 *
 * @details This header contains all constant, compile-time data that doesn't change during
 * runtime. This includes:
 * - Enumerations for state management and type safety
 * - Static string constants for panel names, themes, and configuration keys
 * - Sensor calibration constants and measurement ranges
 * - UI text constants and theme color definitions
 * - Trigger identifiers and system configuration
 *
 * @organization:
 * - State Enums: Runtime state definitions (LogLevels, KeyState, etc.)
 * - System Constants: String identifiers and names (PanelNames, TriggerNames, etc.)
 * - Calibration Constants: Sensor ranges and measurement constants
 * - UI Constants: Display text and theme definitions
 *
 * @note For mutable data structures and runtime types, see types.h
 * @note All structs in this file contain only static constexpr members
 */

// System/Library Includes
#ifdef LVGL_MOCK
    #include "lvgl_mock.h"
#else
    #include <lvgl.h>
#endif

//=============================================================================
// STATE ENUMERATIONS
// Runtime state definitions for type safety and clear state management
//=============================================================================


/**
 * @enum OilSensorTypes
 * @brief Types of oil monitoring sensors
 * 
 * @details Used for identifying different oil sensor types in
 * animations and callbacks within the OemOilPanel system.
 */
enum class OilSensorTypes
{
    Pressure,   ///< Oil pressure sensor (PSI)
    Temperature ///< Oil temperature sensor (degrees)
};

/**
 * @enum KeyState
 * @brief Key presence states for automotive ignition monitoring
 *
 * @details Represents the three possible states of the ignition key system:
 * - Inactive: Neither pin active (no key panel, restore previous)
 * - Present: Key inserted and detected (green key display)
 * - NotPresent: Key explicitly not detected (red key display)
 */
enum class KeyState
{
    Inactive,  ///< Neither pin active - restore previous panel
    Present,   ///< Key is present (GPIO 25 HIGH) - show green key
    NotPresent ///< Key is not present (GPIO 26 HIGH) - show red key
};

/**
 * @enum TriggerActionType
 * @brief Types of actions that triggers can request
 *
 * @details Used for dependency injection - triggers return action requests
 * that main loop processes by calling appropriate manager methods
 */
enum class TriggerActionType
{
    LoadPanel,  ///< Request to load a specific panel
    ToggleTheme ///< Request to toggle theme
};

/**
 * @brief UI state for processing decisions
 */
enum class UIState
{
    IDLE, ///< No UI activity, safe for button actions and interrupts
    BUSY  ///< UI operations in progress (loading, updating, animating), no interrupts allowed
};

inline const char* UIStateToString(UIState state)
{
    switch (state)
    {
        case UIState::IDLE: return "IDLE";
        case UIState::BUSY: return "BUSY";
        default: return "UNKNOWN";
    }
}

/**
 * @brief Priority levels for trigger messages
 */
enum class TriggerPriority
{
    CRITICAL = 0,  ///< Critical triggers (key presence, safety)
    IMPORTANT = 1, ///< Important triggers (lock state, system modes)
    NORMAL = 2     ///< Non-critical triggers (theme changes, settings)
};

/**
 * @brief Priority levels for coordinated interrupt system
 * @details Higher numeric values = higher priority (CRITICAL > IMPORTANT > NORMAL)
 */
enum class Priority
{
    NORMAL = 0,    ///< Non-critical interrupts (theme changes, button actions, preferences)
    IMPORTANT = 1, ///< Important interrupts (lock state, system modes)
    CRITICAL = 2   ///< Critical interrupts (key presence, errors, safety) - highest priority
};

/**
 * @brief Interrupt source types for coordinated handler system
 */
enum class InterruptSource
{
    TRIGGER,    ///< GPIO state monitoring (managed by TriggerHandler)
    ACTION      ///< Button event processing (managed by ActionHandler)
};

/**
 * @brief Interrupt effect types for simplified execution logic
 */
enum class InterruptEffect
{
    LOAD_PANEL,        ///< Panel switching with restoration tracking
    SET_THEME,         ///< Theme changes (non-blocking for restoration)
    SET_PREFERENCE,    ///< Configuration changes
    BUTTON_ACTION      ///< Panel-specific button functions (ACTION only)
};

/**
 * @brief Button action types detected by timing
 */
enum class ButtonAction
{
    NONE = 0,          ///< No action detected
    SHORT_PRESS = 1,   ///< Short press (50ms - 2000ms)
    LONG_PRESS = 2     ///< Long press (2000ms - 5000ms)
};

/**
 * @brief Trigger types for the new Trigger/Action architecture
 */
enum class TriggerType
{
    PANEL,     ///< Panel loading triggers
    STYLE,     ///< Style/theme changing triggers
    FUNCTION   ///< General function triggers
};

/**
 * @brief Action press types for button duration detection
 */
enum class ActionPress
{
    SHORT,     ///< Short press (50ms - 2000ms)
    LONG       ///< Long press (2000ms - 5000ms)
};

/**
 * @brief Trigger execution state enumeration
 */
enum class TriggerExecutionState
{
    INIT = 0,    ///< Initial state - no action required during system startup
    ACTIVE = 1,  ///< Active state - execute action function
    INACTIVE = 2 ///< Inactive state - execute restore function
};

/**
 * @enum ErrorLevel
 * @brief Severity levels for application errors
 */
enum class ErrorLevel
{
    WARNING, ///< Non-critical issues that don't affect core functionality
    ERROR,   ///< Significant issues that may impact features
    CRITICAL ///< Critical issues requiring immediate attention
};

//=============================================================================
// SYSTEM CONSTANTS
// String identifiers, names, and configuration keys used throughout the system
//=============================================================================

/**
 * @struct Themes
 * @brief Static constants for visual theme options
 *
 * @details Defines available visual themes with different color schemes
 * as string constants. Managed by StyleManager for consistent
 * application-wide theming.
 */
struct Themes
{
    static constexpr const char *NIGHT = "Night"; ///< Dark theme with red accents (default)
    static constexpr const char *DAY = "Day";     ///< Light theme with white accents
    static constexpr const char *ERROR = "Error"; ///< Error-specific theme with high contrast for alerts
};

/**
 * @struct PanelNames
 * @brief Static constants for panel type identifiers
 *
 * @details Defines the available panel types used in the PanelManager
 * factory system and configuration as string constants for direct use
 * without conversion overhead.
 */
struct PanelNames
{
    static constexpr const char *SPLASH = "SplashPanel"; ///< Startup splash screen
    static constexpr const char *OIL = "OemOilPanel";    ///< Oil monitoring dashboard
    static constexpr const char *KEY = "KeyPanel";       ///< Key status panel
    static constexpr const char *LOCK = "LockPanel";     ///< Lock status panel
    static constexpr const char *ERROR = "ErrorPanel";   ///< Error display panel
    static constexpr const char *CONFIG = "ConfigPanel"; ///< Configuration settings panel
};

/**
 * @struct TriggerNames
 * @brief String constants for trigger identification
 *
 * @details Provides consistent string identifiers for trigger types
 * used in the InterruptManager registration system.
 */
struct TriggerNames
{
    static constexpr const char *KEY_PRESENT = "key_present_trigger";         ///< Key present detection trigger
    static constexpr const char *KEY_NOT_PRESENT = "key_not_present_trigger"; ///< Key not present detection trigger
    static constexpr const char *LOCK = "lock_trigger";                       ///< Lock detection trigger
    static constexpr const char *LIGHTS_STATE = "lights_state_trigger";       ///< Lights state detection trigger
    static constexpr const char *ERROR_OCCURRED = "error_occurred_trigger";   ///< Error occurred trigger
};

/**
 * @struct TriggerIds
 * @brief Short trigger ID constants for internal use
 *
 * @details Short identifiers used internally, different from the
 * longer TriggerNames used for registration.
 */
struct TriggerIds
{
    static constexpr const char *KEY_PRESENT = "key_present";         ///< Key present trigger ID
    static constexpr const char *KEY_NOT_PRESENT = "key_not_present"; ///< Key not present trigger ID
    static constexpr const char *LOCK_STATE = "lock_state";           ///< Lock state trigger ID
    static constexpr const char *LIGHTS_STATE = "lights_state";       ///< Lights state trigger ID
    static constexpr const char *ERROR_OCCURRED = "error_occurred";   ///< Error occurred trigger ID
    static constexpr const char *SHORT_PRESS = "universal_short_press"; ///< Short press button ID
    static constexpr const char *LONG_PRESS = "universal_long_press";   ///< Long press button ID
};

/**
 * @enum class InterruptFlags
 * @brief Type-safe bit flag enumeration for interrupt state management
 *
 * @details Provides type-safe flag operations for interrupt state tracking.
 * Uses enum class for type safety while supporting bitwise operations.
 */
enum class InterruptFlags : uint8_t
{
    NONE = 0x00,           ///< No flags set
    ACTIVE = 0x01,         ///< Interrupt is active (bit 0)
    NEEDS_EXECUTION = 0x02,///< Needs execution (bit 1) 
    STATE_CHANGED = 0x04,  ///< State has changed (bit 2)
    ALWAYS_EXECUTE = 0x08  ///< Always execute regardless of state (bit 3)
};

/**
 * @brief Bitwise OR operator for InterruptFlags
 */
constexpr InterruptFlags operator|(InterruptFlags lhs, InterruptFlags rhs) {
    return static_cast<InterruptFlags>(
        static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs)
    );
}

/**
 * @brief Bitwise AND operator for InterruptFlags
 */  
constexpr InterruptFlags operator&(InterruptFlags lhs, InterruptFlags rhs) {
    return static_cast<InterruptFlags>(
        static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs)
    );
}

/**
 * @brief Bitwise NOT operator for InterruptFlags
 */
constexpr InterruptFlags operator~(InterruptFlags flags) {
    return static_cast<InterruptFlags>(~static_cast<uint8_t>(flags));
}

/**
 * @brief Bitwise OR-assign operator for InterruptFlags
 */
constexpr InterruptFlags& operator|=(InterruptFlags& lhs, InterruptFlags rhs) {
    lhs = lhs | rhs;
    return lhs;
}

/**
 * @brief Bitwise AND-assign operator for InterruptFlags
 */  
constexpr InterruptFlags& operator&=(InterruptFlags& lhs, InterruptFlags rhs) {
    lhs = lhs & rhs;
    return lhs;
}

/**
 * @struct JsonDocNames
 * @brief JSON field names for configuration serialization
 *
 * @details Defines consistent field names used in JSON configuration
 * documents for PreferenceManager serialization.
 */
struct JsonDocNames
{
    static constexpr const char *PANEL_NAME = "panel_name";           ///< Default panel setting
    static constexpr const char *SHOW_SPLASH = "show_splash";         ///< Show splash screen setting
    static constexpr const char *SPLASH_DURATION = "splash_duration"; ///< Splash duration setting
    static constexpr const char *THEME = "theme";                     ///< Theme setting
    static constexpr const char *UPDATE_RATE = "update_rate";         ///< Update rate setting
    static constexpr const char *PRESSURE_UNIT = "pressure_unit";     ///< Pressure unit setting
    static constexpr const char *TEMP_UNIT = "temp_unit";             ///< Temperature unit setting
    static constexpr const char *PRESSURE_OFFSET = "pressure_offset"; ///< Pressure sensor offset calibration
    static constexpr const char *PRESSURE_SCALE = "pressure_scale";   ///< Pressure sensor scale calibration
    static constexpr const char *TEMP_OFFSET = "temp_offset";         ///< Temperature sensor offset calibration
    static constexpr const char *TEMP_SCALE = "temp_scale";           ///< Temperature sensor scale calibration
};

//=============================================================================
// CALIBRATION CONSTANTS
// Sensor measurement ranges, calibration values, and system configuration
//=============================================================================

/**
 * @struct SystemConstants
 * @brief Static constants for system configuration
 *
 * @details Defines system-level constants like preferences namespace
 * and configuration keys.
 */
struct SystemConstants
{
    static constexpr const char *PREFERENCES_NAMESPACE = "clarity"; ///< NVS preferences namespace
};

/**
 * @struct SensorConstants
 * @brief Static constants for sensor calibration and ranges
 *
 * @details Defines calibration constants for oil temperature and pressure
 * sensors including their measurement ranges in various units.
 */
struct SensorConstants
{
    // Temperature sensor calibration constants
    static constexpr int32_t TEMPERATURE_MAX_CELSIUS = 120;    ///< Maximum temperature reading in Celsius
    static constexpr int32_t TEMPERATURE_MIN_FAHRENHEIT = 32;  ///< 0°C in Fahrenheit
    static constexpr int32_t TEMPERATURE_MAX_FAHRENHEIT = 248; ///< 120°C in Fahrenheit

    // Pressure sensor calibration constants
    static constexpr int32_t PRESSURE_MAX_BAR = 10;   ///< Maximum pressure reading in Bar
    static constexpr int32_t PRESSURE_MAX_PSI = 145;  ///< Maximum pressure reading in PSI (10 Bar * 14.5)
    static constexpr int32_t PRESSURE_MAX_KPA = 1000; ///< Maximum pressure reading in kPa (10 Bar * 100)
};

//=============================================================================
// UI CONSTANTS
// User interface text, labels, and visual theme definitions
//=============================================================================

/**
 * @struct UIConstants
 * @brief Static constants for UI text and labels
 *
 * @details Defines reusable UI text constants to avoid magic strings
 * and ensure consistency across components.
 */
struct UIConstants
{
    static constexpr const char *APP_NAME = "Clarity";   ///< Application name
    static constexpr const char *GAUGE_LOW_LABEL = "L";  ///< Low gauge indicator
    static constexpr const char *GAUGE_HIGH_LABEL = "H"; ///< High gauge indicator
};

