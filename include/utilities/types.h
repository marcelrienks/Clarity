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

/// @enum Themes
/// @brief Visual theme options for the application
///
/// @details Defines available visual themes with different color schemes.
/// Managed by StyleManager for consistent application-wide theming.
enum class Themes
{
    Night, ///< Dark theme with red accents (default)
    Day    ///< Light theme with white accents
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
/// @brief String constants for panel identification
///
/// @details Provides consistent string identifiers for panel types
/// used in the PanelManager factory system and configuration.
struct PanelNames
{
    static constexpr const char *Splash = "SplashPanel"; ///< Startup splash screen
    static constexpr const char *Oil = "OemOilPanel";    ///< Oil monitoring dashboard
    static constexpr const char *Key = "KeyPanel";       ///< Key status panel
    static constexpr const char *Lock = "LockPanel";     ///< Lock status panel
};

/// @struct TriggerNames
/// @brief String constants for trigger identification
///
/// @details Provides consistent string identifiers for trigger types
/// used in the InterruptManager registration system.
struct TriggerNames
{
    static constexpr const char *KeyPresent = "key_present_trigger";        ///< Key present detection trigger
    static constexpr const char *KeyNotPresent = "key_not_present_trigger"; ///< Key not present detection trigger
    static constexpr const char *Lock = "lock_trigger";                     ///< Lock detection trigger
};

/// @struct JsonDocNames
/// @brief JSON field names for configuration serialization
///
/// @details Defines consistent field names used in JSON configuration
/// documents for PreferenceManager serialization.
struct JsonDocNames
{
    static constexpr const char *panel_name = "panel_name"; ///< Default panel setting
};

/// @struct WidgetLocation
/// @brief UI widget positioning and sizing parameters
///
/// @details Comprehensive structure for defining widget placement
/// within LVGL screens. Supports both absolute positioning (x,y) and
/// relative alignment with offsets.
///
/// @positioning_modes:
/// - Absolute: Use x,y coordinates directly
/// - Relative: Use align with optional x_offset, y_offset
/// - Sizing: width,height or LV_SIZE_CONTENT for auto-sizing
///
/// @usage_examples:
/// - WidgetLocation(100, 50): Absolute position at (100,50)
/// - WidgetLocation(LV_ALIGN_CENTER): Centered with auto-size
/// - WidgetLocation(LV_ALIGN_LEFT_MID, 10, 0): Left-aligned with 10px offset
struct WidgetLocation
{
    lv_coord_t x = 0;                   ///< Absolute X coordinate
    lv_coord_t y = 0;                   ///< Absolute Y coordinate
    lv_align_t align = LV_ALIGN_CENTER; ///< LVGL alignment mode
    lv_coord_t x_offset = 0;            ///< X offset from alignment point
    lv_coord_t y_offset = 0;            ///< Y offset from alignment point
    int32_t rotation = 0;               ///< Rotation angle in degrees (optional)

    WidgetLocation() = default;

    /// Constructor for absolute positioning
    WidgetLocation(lv_coord_t x, lv_coord_t y) : x(x), y(y) {}

    /// Constructor for relative alignment with optional offsets
    WidgetLocation(lv_align_t align, lv_coord_t x_offset = 0, lv_coord_t y_offset = 0)
        : align(align), x_offset(x_offset), y_offset(y_offset) {}

    /// Constructor for rotation start point of scales
    WidgetLocation(int32_t rotation)
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
    std::string panel_name = PanelNames::Oil; ///< Default panel on startup
};