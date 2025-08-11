#pragma once // preventing duplicate definitions, alternative to the traditional include guards

/**
 * @typedef Reading
 * @brief Variant type for sensor readings supporting multiple data types
 *
 * @details This variant can hold different types of sensor data:
 * - std::monostate: Uninitialized/invalid reading
 * - int32_t: Integer values (pressure, temperature, etc.)
 * - double: Floating-point values (precise measurements)
 * - std::string: Text/status readings
 * - bool: Boolean states (switches, alarms)
 *
 * @usage_examples:
 * - Oil pressure: int32_t (0-10 Bar)
 * - Oil temperature: int32_t (0-120°C)
 * - Key status: bool (true/false)
 * - Error messages: std::string
 */

// System/Library Includes
#ifdef LVGL_MOCK
#include "lvgl_mock.h"
#else
#include <lvgl.h>
#endif
#include <string>
#include <variant>
#include <vector>
#include <functional>
#include <stdint.h>

// Types/Structs/Enums
/// @typedef Reading
/// @brief Variant type for sensor readings supporting multiple data types
///
/// @details This variant can hold different types of sensor data:
/// - std::monostate: Uninitialized/invalid reading
/// - int32_t: Integer values (pressure, temperature, etc.)
/// - double: Floating-point values (precise measurements)
/// - std::string: Text/status readings
/// - bool: Boolean states (switches, alarms)
///
/// @usage_examples:
/// - Oil pressure: int32_t (0-10 Bar)
/// - Oil temperature: int32_t (0-120°C)
/// - Key status: bool (true/false)
/// - Error messages: std::string
using Reading = std::variant<std::monostate, int32_t, double, std::string, bool>;

/// @enum LogLevels
/// @brief Logging verbosity levels for debug output
///
/// @details Defines the hierarchy of logging levels from most verbose
/// to most critical. Used by the logging system to filter output.
enum class LogLevels
{
    Verbose, ///< Detailed trace information
    Debug,   ///< Debug information for development
    Info,    ///< General information messages
    Warning, ///< Warning conditions that should be noted
    Error    ///< Error conditions requiring attention
};

/// @struct Themes
/// @brief Static constants for visual theme options
///
/// @details Defines available visual themes with different color schemes
/// as string constants. Managed by StyleManager for consistent 
/// application-wide theming.
struct Themes
{
    static constexpr const char* NIGHT = "Night"; ///< Dark theme with red accents (default)
    static constexpr const char* DAY = "Day";     ///< Light theme with white accents
    static constexpr const char* ERROR = "Error"; ///< Error-specific theme with high contrast for alerts
};

/// @enum OilSensorTypes
/// @brief Types of oil monitoring sensors
///
/// @details Used for identifying different oil sensor types in
/// animations and callbacks within the OemOilPanel system.
enum class OilSensorTypes
{
    Pressure,   ///< Oil pressure sensor (PSI)
    Temperature ///< Oil temperature sensor (degrees)
};

/// @enum KeyState
/// @brief Key presence states for automotive ignition monitoring
///
/// @details Represents the three possible states of the ignition key system:
/// - Inactive: Neither pin active (no key panel, restore previous)
/// - Present: Key inserted and detected (green key display)
/// - NotPresent: Key explicitly not detected (red key display)  
enum class KeyState
{
    Inactive,   ///< Neither pin active - restore previous panel
    Present,    ///< Key is present (GPIO 25 HIGH) - show green key
    NotPresent  ///< Key is not present (GPIO 26 HIGH) - show red key
};

/// @enum TriggerActionType
/// @brief Types of actions that triggers can request
///
/// @details Used for dependency injection - triggers return action requests
/// that main loop processes by calling appropriate manager methods
enum class TriggerActionType
{
    LoadPanel,      ///< Request to load a specific panel
    ToggleTheme     ///< Request to toggle theme
};


/// @struct PanelNames
/// @brief Static constants for panel type identifiers
///
/// @details Defines the available panel types used in the PanelManager
/// factory system and configuration as string constants for direct use
/// without conversion overhead.
struct PanelNames
{
    static constexpr const char* SPLASH = "SplashPanel"; ///< Startup splash screen
    static constexpr const char* OIL = "OemOilPanel";    ///< Oil monitoring dashboard
    static constexpr const char* KEY = "KeyPanel";       ///< Key status panel
    static constexpr const char* LOCK = "LockPanel";     ///< Lock status panel
    static constexpr const char* ERROR = "ErrorPanel";   ///< Error display panel
    static constexpr const char* CONFIG = "ConfigPanel"; ///< Configuration settings panel
};

/// @struct TriggerNames
/// @brief String constants for trigger identification
///
/// @details Provides consistent string identifiers for trigger types
/// used in the InterruptManager registration system.
struct TriggerNames
{
    static constexpr const char *KEY_PRESENT = "key_present_trigger";        ///< Key present detection trigger
    static constexpr const char *KEY_NOT_PRESENT = "key_not_present_trigger"; ///< Key not present detection trigger
    static constexpr const char *LOCK = "lock_trigger";                     ///< Lock detection trigger
};

/// @struct JsonDocNames
/// @brief JSON field names for configuration serialization
///
/// @details Defines consistent field names used in JSON configuration
/// documents for PreferenceManager serialization.
struct JsonDocNames
{
    static constexpr const char *PANEL_NAME = "panel_name";       ///< Default panel setting
    static constexpr const char *SHOW_SPLASH = "show_splash";     ///< Show splash screen setting
    static constexpr const char *SPLASH_DURATION = "splash_duration"; ///< Splash duration setting
    static constexpr const char *THEME = "theme";                 ///< Theme setting
    static constexpr const char *UPDATE_RATE = "update_rate";     ///< Update rate setting
    static constexpr const char *PRESSURE_UNIT = "pressure_unit"; ///< Pressure unit setting
    static constexpr const char *TEMP_UNIT = "temp_unit";         ///< Temperature unit setting
};

/// @struct SystemConstants
/// @brief Static constants for system configuration
///
/// @details Defines system-level constants like preferences namespace
/// and configuration keys.
struct SystemConstants
{
    static constexpr const char* PREFERENCES_NAMESPACE = "clarity"; ///< NVS preferences namespace
};


/// @struct UIConstants
/// @brief Static constants for UI text and labels
///
/// @details Defines reusable UI text constants to avoid magic strings
/// and ensure consistency across components.
struct UIConstants
{
    static constexpr const char* APP_NAME = "Clarity";          ///< Application name
    static constexpr const char* GAUGE_LOW_LABEL = "L";        ///< Low gauge indicator
    static constexpr const char* GAUGE_HIGH_LABEL = "H";       ///< High gauge indicator
};

/// @struct ComponentLocation
/// @brief UI component positioning and sizing parameters
///
/// @details Comprehensive structure for defining component placement
/// within LVGL screens. Supports both absolute positioning (x,y) and
/// relative alignment with offsets.
///
/// @positioning_modes:
/// - Absolute: Use x,y coordinates directly
/// - Relative: Use align with optional x_offset, y_offset
/// - Sizing: width,height or LV_SIZE_CONTENT for auto-sizing
///
/// @usage_examples:
/// - ComponentLocation(100, 50): Absolute position at (100,50)
/// - ComponentLocation(LV_ALIGN_CENTER): Centered with auto-size
/// - ComponentLocation(LV_ALIGN_LEFT_MID, 10, 0): Left-aligned with 10px offset
struct ComponentLocation
{
    lv_coord_t x = 0;                   ///< Absolute X coordinate
    lv_coord_t y = 0;                   ///< Absolute Y coordinate
    lv_align_t align = LV_ALIGN_CENTER; ///< LVGL alignment mode
    lv_coord_t x_offset = 0;            ///< X offset from alignment point
    lv_coord_t y_offset = 0;            ///< Y offset from alignment point
    int32_t rotation = 0;               ///< Rotation angle in degrees (optional)

    ComponentLocation() = default;

    /// Constructor for absolute positioning
    ComponentLocation(lv_coord_t x, lv_coord_t y) : x(x), y(y) {}

    /// Constructor for relative alignment with optional offsets
    ComponentLocation(lv_align_t align, lv_coord_t x_offset = 0, lv_coord_t y_offset = 0)
        : align(align), x_offset(x_offset), y_offset(y_offset) {}

    /// Constructor for rotation start point of scales
    ComponentLocation(int32_t rotation)
        : rotation(rotation) {}
};

/// @struct Configs
/// @brief Application configuration settings
///
/// @details Holds all persistent configuration values managed by
/// PreferenceManager. Automatically serialized to/from JSON in NVS.
///
/// @default_values All fields have sensible defaults for first-run
struct Configs
{
    // General settings
    std::string panelName = PanelNames::OIL; ///< Default panel on startup
    bool showSplash = true;                  ///< Show splash screen on startup
    int splashDuration = 2000;               ///< Splash screen duration in milliseconds
    
    // Display settings
    std::string theme = Themes::DAY;         ///< Theme preference (Day/Night)
    
    // Sensor settings
    int updateRate = 500;                    ///< Sensor update rate in milliseconds
    std::string pressureUnit = "PSI";        ///< Pressure unit (PSI, Bar, kPa)
    std::string tempUnit = "C";              ///< Temperature unit (C, F)
};

/// @brief UI state for processing decisions
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

/// @brief Trigger execution state enumeration
enum class TriggerExecutionState {
    INIT = 0,     ///< Initial state - no action required during system startup
    ACTIVE = 1,   ///< Active state - execute action function
    INACTIVE = 2  ///< Inactive state - execute restore function
};


/// @brief Trigger ID constants
constexpr const char *TRIGGER_KEY_PRESENT = "key_present";
constexpr const char *TRIGGER_KEY_NOT_PRESENT = "key_not_present";
constexpr const char *TRIGGER_LOCK_STATE = "lock_state";
constexpr const char *TRIGGER_LIGHTS_STATE = "lights_state";
constexpr const char *TRIGGER_ERROR_OCCURRED = "error_occurred";

/// @brief Consolidated GPIO state structure for single-read pattern
struct GpioState {
    bool keyPresent;
    bool keyNotPresent;
    bool lockState;
    bool lightsState;
};

/// @brief Direct trigger mapping structure (replaces trigger objects)
struct Trigger {
    const char *triggerId;
    int pin;
    TriggerActionType actionType;
    const char *actionTarget;
    const char *restoreTarget;
    TriggerPriority priority;
    TriggerExecutionState currentState = TriggerExecutionState::INIT;
};

/// @enum ErrorLevel
/// @brief Severity levels for application errors
enum class ErrorLevel { 
    WARNING,   ///< Non-critical issues that don't affect core functionality
    ERROR,     ///< Significant issues that may impact features
    CRITICAL   ///< Critical issues requiring immediate attention
};

/// @struct ErrorInfo
/// @brief Complete error information structure
struct ErrorInfo {
    ErrorLevel level;           ///< Severity level of the error
    const char* source;         ///< Component/manager that reported the error
    std::string message;        ///< Human-readable error description
    unsigned long timestamp;    ///< millis() timestamp when error occurred
    bool acknowledged;          ///< Whether user has acknowledged the error
};

// Theme color definitions
struct ThemeColors
{
    lv_color_t background;      // Background color
    lv_color_t text;            // Text color
    lv_color_t primary;         // Primary accent color
    lv_color_t gaugeNormal;    // Normal gauge color
    lv_color_t gaugeWarning;   // Warning gauge color
    lv_color_t gaugeDanger;    // Danger gauge color
    lv_color_t gaugeTicks;     // Gauge tick marks (soft off-white)
    lv_color_t needleNormal;   // Normal needle color (bright white)
    lv_color_t needleDanger;   // Danger needle color (bright red/orange)
    lv_color_t keyPresent;     // Normal key present clor (pure white)
    lv_color_t keyNotPresent; // Normal Key not present color (bright red)
};

/// @struct Action
/// @brief Simple action struct that holds a function to execute
struct Action {
    std::function<void()> execute;  ///< Function to execute
    
    // Constructor
    Action(std::function<void()> func = nullptr) : execute(func) {}
    
    // Check if action has a valid function
    bool IsValid() const { return execute != nullptr; }
};

