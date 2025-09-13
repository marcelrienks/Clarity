#pragma once // preventing duplicate definitions, alternative to the traditional include guards

/**
 * @file types.h
 * @brief Mutable types, structs, and runtime data containers for the Clarity automotive gauge system
 *
 * @details This header contains all mutable data structures and types that can change
 * during runtime. This includes:
 * - Variant types for sensor readings and dynamic data
 * - Configuration structures with user-modifiable settings
 * - State structures that track system and hardware status
 * - UI positioning and layout data with runtime calculations
 * - Error information with timestamps and acknowledgment states
 * - Function wrappers and action handlers
 *
 * @organization:
 * - Core Types: Fundamental data types like Reading variant
 * - Configuration Data: User settings and preferences (Configs)
 * - Runtime State: System and hardware state tracking (ErrorInfo)
 * - UI Data: Component positioning and layout (ComponentLocation)
 * - Behavior Types: Actions and triggers with mutable state
 *
 * @note For compile-time constants and immutable data, see constants.h
 * @note All structs in this file contain mutable data or constructors
 */

// System/Library Includes
#ifdef LVGL_MOCK
    #include "lvgl_mock.h"
#else
    #include <lvgl.h>
#endif
#include <cstring>
#include <functional>
#include <stdint.h>
#include <string>
#include <variant>
#include <vector>

// Project Includes
#include "utilities/constants.h"

//=============================================================================
// CORE TYPES
// Fundamental data types for sensor readings and dynamic data exchange
//=============================================================================

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
using Reading = std::variant<std::monostate, int32_t, double, std::string, bool>;

//=============================================================================
// UI DATA STRUCTURES
// Component positioning, layout data, and user interface calculations
//=============================================================================

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
    ComponentLocation(lv_coord_t x, lv_coord_t y) : x(x), y(y)
    {
    }

    /// Constructor for relative alignment with optional offsets
    ComponentLocation(lv_align_t align, lv_coord_t x_offset = 0, lv_coord_t y_offset = 0)
        : align(align), x_offset(x_offset), y_offset(y_offset)
    {
    }

    /// Constructor for rotation start point of scales
    ComponentLocation(int32_t rotation) : rotation(rotation)
    {
    }
};

//=============================================================================
// CONFIGURATION DATA
// User-modifiable settings and application preferences
//=============================================================================

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
    int splashDuration = 1500;               ///< Splash screen duration in milliseconds

    // Display settings
    std::string theme = Themes::DAY; ///< Theme preference (Day/Night)

    // Sensor settings
    int updateRate = 500;             ///< Sensor update rate in milliseconds
    std::string pressureUnit = "Bar"; ///< Pressure unit (PSI, Bar, kPa)
    std::string tempUnit = "C";       ///< Temperature unit (C, F)
    
    // Sensor calibration settings
    float pressureOffset = 0.0f;      ///< Pressure sensor offset correction
    float pressureScale = 1.0f;       ///< Pressure sensor scaling factor
    float tempOffset = 0.0f;          ///< Temperature sensor offset correction
    float tempScale = 1.0f;           ///< Temperature sensor scaling factor
};

//=============================================================================
// RUNTIME STATE STRUCTURES
// System status, hardware state, and error tracking data
//=============================================================================


/// @brief State-based Trigger for GPIO monitoring with dual functions
struct Trigger
{
    const char* id;                      ///< Static string identifier
    Priority priority;                   ///< Processing priority (CRITICAL > IMPORTANT > NORMAL)
    TriggerType type;                    ///< PANEL, STYLE, or FUNCTION
    void (*activateFunc)();              ///< Function called when trigger activates
    void (*deactivateFunc)();            ///< Function called when trigger deactivates
    class BaseSensor* sensor;            ///< Associated sensor for state monitoring
    bool isActive;                       ///< Current activation state
    
    // Execution methods as documented in interrupt-architecture.md
    void ExecuteActivate() {
        if (activateFunc) {
            activateFunc();
            isActive = true;  // Only activation sets active flag
        }
    }
    
    void ExecuteDeactivate() {
        if (deactivateFunc) {
            deactivateFunc();
            isActive = false;  // Clear active flag on deactivation
        }
    }
};

/// @struct ErrorInfo
/// @brief Complete error information structure
struct ErrorInfo
{
    static constexpr size_t MAX_MESSAGE_LENGTH = 128; ///< Fixed buffer size for embedded optimization

    ErrorLevel level;        ///< Severity level of the error
    const char *source;      ///< Component/manager that reported the error
    char message[MAX_MESSAGE_LENGTH]; ///< Fixed-size error message buffer (optimized for embedded)
    unsigned long timestamp; ///< millis() timestamp when error occurred
    bool acknowledged;       ///< Whether user has acknowledged the error

    // Constructor to maintain compatibility with std::string API
    ErrorInfo() : level(ErrorLevel::WARNING), source(nullptr), timestamp(0), acknowledged(false) {
        message[0] = '\0';
    }

    // Helper method to set message safely
    void SetMessage(const std::string& msg) {
        strncpy(message, msg.c_str(), MAX_MESSAGE_LENGTH - 1);
        message[MAX_MESSAGE_LENGTH - 1] = '\0'; // Ensure null termination
    }
};

//=============================================================================
// BEHAVIOR TYPES
// Action handlers, triggers, and function wrappers with mutable state
//=============================================================================

/// @struct Action  
/// @brief Event-based Action for button processing
struct Action
{
    const char* id;                      ///< Static string identifier
    void (*executeFunc)();               ///< Function to execute on button press
    bool hasTriggered;                   ///< Whether this action has been triggered
    ActionPress pressType;               ///< SHORT or LONG press type
    
    // Execution method as documented in interrupt-architecture.md
    void Execute() {
        if (executeFunc && hasTriggered) {
            executeFunc();
            hasTriggered = false;  // Clear trigger flag after execution
        }
    }
};

