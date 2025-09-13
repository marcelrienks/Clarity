#pragma once

#include "hardware/gpio_pins.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_sensor.h"
#include "sensors/base_sensor.h"
#include "utilities/types.h"

/**
 * @struct GpioSensorConfig
 * @brief Configuration structure for generic GPIO sensors
 *
 * @details Defines all necessary parameters for configuring a GPIO sensor,
 * enabling the consolidation of multiple similar sensor classes into a
 * single generic implementation.
 */
struct GpioSensorConfig {
    uint8_t pin;                    ///< GPIO pin number
    uint8_t pinMode;                ///< Pin mode (INPUT, INPUT_PULLUP, INPUT_PULLDOWN)
    const char* name;               ///< Human-readable sensor name for logging
    bool activeHigh;                ///< true if HIGH means active, false if LOW means active
    bool debugOnly;                 ///< true if sensor is only available in debug builds

    /**
     * @brief Constructor with default values
     * @param pin GPIO pin number
     * @param pinMode Pin mode configuration
     * @param name Sensor name for logging
     * @param activeHigh true if HIGH is active state
     * @param debugOnly true if debug-only sensor
     */
    constexpr GpioSensorConfig(
        uint8_t pin,
        uint8_t pinMode,
        const char* name,
        bool activeHigh = true,
        bool debugOnly = false)
        : pin(pin)
        , pinMode(pinMode)
        , name(name)
        , activeHigh(activeHigh)
        , debugOnly(debugOnly) {}
};

/**
 * @class GpioSensor
 * @brief Generic GPIO sensor implementation for digital input monitoring
 *
 * @details This class consolidates the functionality of multiple specific GPIO
 * sensor classes (KeyPresentSensor, KeyNotPresentSensor, LockSensor, LightsSensor,
 * DebugErrorSensor) into a single configurable implementation. This reduces
 * code duplication and simplifies maintenance while preserving all functionality.
 *
 * @architecture_benefits:
 * - Eliminates 4+ individual sensor classes (~150+ lines of duplicate code)
 * - Configuration-driven approach enables easy addition of new sensors
 * - Consistent behavior across all GPIO sensors
 * - Simplified testing with fewer classes to mock
 * - Reduced memory footprint through code consolidation
 *
 * @usage_example:
 * ```cpp
 * // Define configuration
 * constexpr GpioSensorConfig KEY_PRESENT_CONFIG{
 *     gpio_pins::KEY_PRESENT, INPUT_PULLDOWN, "KeyPresent", true, false
 * };
 *
 * // Create sensor
 * GpioSensor keyPresentSensor(KEY_PRESENT_CONFIG, gpioProvider);
 * ```
 *
 * @change_detection Uses BaseSensor DetectChange template for consistency
 * @memory_safe Designed for ESP32 memory constraints with minimal overhead
 * @interrupt_support Compatible with existing interrupt architecture
 */
class GpioSensor : public BaseSensor
{
public:
    /**
     * @brief Constructor for GPIO sensor with configuration
     * @param config Sensor configuration structure
     * @param gpioProvider GPIO hardware abstraction provider
     */
    GpioSensor(const GpioSensorConfig& config, IGpioProvider* gpioProvider);

    /**
     * @brief Destructor with GPIO cleanup
     */
    ~GpioSensor();

    // ISensor interface implementation
    void Init() override;
    Reading GetReading() override;

    // BaseSensor interface implementation
    bool HasStateChanged() override;

    /**
     * @brief Get current GPIO state directly
     * @return true if sensor is active (based on activeHigh configuration), false otherwise
     */
    bool GetState();

    /**
     * @brief Get sensor name for logging and debugging
     * @return Human-readable sensor name
     */
    const char* GetName() const { return config_.name; }

    /**
     * @brief Get GPIO pin number
     * @return GPIO pin number for this sensor
     */
    uint8_t GetPin() const { return config_.pin; }

protected:
    /**
     * @brief Custom interrupt behavior (can be overridden for specific sensors)
     */
    void OnInterruptTriggered() override;

private:
    GpioSensorConfig config_;       ///< Sensor configuration
    IGpioProvider* gpioProvider_;   ///< GPIO hardware abstraction
    bool previousState_;            ///< Previous state for change detection

    /**
     * @brief Read raw GPIO pin state
     * @return Raw GPIO pin state (HIGH/LOW)
     */
    bool readRawPinState();

    /**
     * @brief Read logical sensor state based on configuration
     * @return Logical sensor state (active/inactive based on activeHigh setting)
     */
    bool readLogicalState();
};

// Predefined sensor configurations for common use cases
namespace sensor_configs {
    /// @brief Key present sensor configuration (GPIO 25)
    constexpr GpioSensorConfig KEY_PRESENT{
        gpio_pins::KEY_PRESENT, INPUT_PULLDOWN, "KeyPresent", true, false
    };

    /// @brief Key not present sensor configuration (GPIO 26)
    constexpr GpioSensorConfig KEY_NOT_PRESENT{
        gpio_pins::KEY_NOT_PRESENT, INPUT_PULLDOWN, "KeyNotPresent", true, false
    };

    /// @brief Lock sensor configuration (GPIO 27)
    constexpr GpioSensorConfig LOCK{
        gpio_pins::LOCK, INPUT_PULLDOWN, "Lock", true, false
    };

    /// @brief Lights sensor configuration (GPIO 33)
    constexpr GpioSensorConfig LIGHTS{
        gpio_pins::LIGHTS, INPUT_PULLDOWN, "Lights", true, false
    };

    /// @brief Debug error sensor configuration (GPIO 34, debug builds only)
    constexpr GpioSensorConfig DEBUG_ERROR{
        gpio_pins::DEBUG_ERROR, INPUT, "DebugError", true, true
    };
}