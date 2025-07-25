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
#include <lvgl.h>
#include <string>
#include <variant>
#include <vector>
#include <stdint.h>
#include <esp_timer.h>

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
    static constexpr const char *PANEL_NAME = "panel_name"; ///< Default panel setting
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
    std::string panelName = PanelNames::OIL; ///< Default panel on startup
};

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


/**
 * @brief Structure for shared trigger state
 * 
 * @details This structure represents the state of a trigger,
 * including its action, target, priority, and active status.
 */
struct TriggerState
{
    std::string action;           ///< Action to perform
    std::string target;           ///< Target of the action
    TriggerPriority priority;     ///< Priority level
    uint64_t timestamp;           ///< When trigger was created (for FIFO within same priority)
    bool active;                  ///< Whether trigger is currently active (GPIO HIGH)
    
    TriggerState() = default;
    TriggerState(const char* action, const char* target, TriggerPriority priority, uint64_t timestamp)
        : action(action), target(target), priority(priority), timestamp(timestamp), active(true) {}
};

/// @brief Configuration constants
constexpr int PANEL_STATE_MUTEX_TIMEOUT = 100;
constexpr int THEME_STATE_MUTEX_TIMEOUT = 100;


/// @brief Action constants
constexpr const char *ACTION_LOAD_PANEL = "LoadPanel";
constexpr const char *ACTION_RESTORE_PREVIOUS_PANEL = "RestorePreviousPanel";
constexpr const char *ACTION_CHANGE_THEME = "ChangeTheme";

/// @brief Theme name constants
constexpr const char *THEME_DAY = "Day";
constexpr const char *THEME_NIGHT = "Night";

/// @brief Trigger ID constants
constexpr const char *TRIGGER_MONITOR_TASK = "TriggerMonitorTask";
constexpr const char *TRIGGER_KEY_PRESENT = "key_present";
constexpr const char *TRIGGER_KEY_NOT_PRESENT = "key_not_present";
constexpr const char *TRIGGER_LOCK_STATE = "lock_state";
constexpr const char *TRIGGER_THEME_SWITCH = "theme_switch";

/// @brief ISR Event types for safe interrupt handling
enum class ISREventType {
    KEY_PRESENT,
    KEY_NOT_PRESENT,
    LOCK_STATE_CHANGE,
    THEME_SWITCH
};

/// @brief ISR Event structure for ISR-to-Task communication
struct ISREvent {
    ISREventType eventType;
    bool pinState;
    uint32_t timestamp;
    
    ISREvent() : eventType(ISREventType::KEY_PRESENT), pinState(false), timestamp(0) {}
    ISREvent(ISREventType type, bool state) : eventType(type), pinState(state), timestamp(esp_timer_get_time()) {}
};