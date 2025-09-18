#include "sensors/oil_temperature_sensor.h"
#include "managers/error_manager.h"
#include "config/config_types.h"
#include "utilities/logging.h"
#include <Arduino.h>
#include <esp32-hal-log.h>

// Constructors and Destructors

/**
 * @brief Constructs an oil temperature sensor with basic GPIO provider dependency
 * @param gpioProvider Pointer to GPIO provider interface for ADC reading
 * @param updateRateMs Update interval in milliseconds for sensor readings
 *
 * Initializes the sensor with GPIO provider for ADC operations and sets default
 * configuration values. Uses Celsius as the default temperature unit. This constructor
 * is used when no preference service is available for dynamic configuration.
 */
OilTemperatureSensor::OilTemperatureSensor(IGpioProvider *gpioProvider, int updateRateMs)
    : gpioProvider_(gpioProvider), updateIntervalMs_(updateRateMs)
{
    log_v("OilTemperatureSensor() constructor called");
    // Set default unit to Celsius
    targetUnit_ = "C";
    currentReading_ = 0;
}

/**
 * @brief Constructs an oil temperature sensor with preference service for dynamic configuration
 * @param gpioProvider Pointer to GPIO provider interface for ADC reading
 * @param preferenceService Pointer to preference service for configuration persistence
 * @param updateRateMs Default update interval in milliseconds for sensor readings
 *
 * Enhanced constructor that enables dynamic configuration through the preference service.
 * Allows for real-time calibration adjustments and unit changes. The preference service
 * enables live configuration updates and persistent storage of sensor settings.
 */
OilTemperatureSensor::OilTemperatureSensor(IGpioProvider *gpioProvider, IPreferenceService *preferenceService, int updateRateMs)
    : gpioProvider_(gpioProvider), preferenceService_(preferenceService), updateIntervalMs_(updateRateMs)
{
    log_v("OilTemperatureSensor() constructor with preference service called");
    // Set default unit to Celsius
    targetUnit_ = "C";
    currentReading_ = 0;
}

// Core Functionality Methods

/**
 * @brief Initialize the oil temperature sensor hardware and configuration
 *
 * Configures ESP32 ADC for 12-bit resolution with 11dB attenuation for 0-3.3V range.
 * Registers with dynamic configuration system if enabled and loads persisted settings.
 * Sets up live configuration callbacks for runtime updates.
 * Takes initial sensor reading to establish baseline for change detection.
 */
void OilTemperatureSensor::Init()
{
    log_v("Init() called");
    // Configure GPIO pin for analog input

    // Configure ADC resolution and attenuation for direct 3.3V operation
    analogReadResolution(12);       // 12-bit resolution (0-4095)
    analogSetAttenuation(ADC_11db); // 0-3.3V range

    // Register with dynamic config system if available and load configuration
    if (preferenceService_) {
        LoadConfiguration();

        // Register this sensor's configuration section with the preference service
        // This enables self-registration per docs/plans/dynamic-config-implementation.md
        RegisterConfiguration();

        // Set up live callbacks to respond to configuration changes
        RegisterLiveUpdateCallbacks();
        log_i("OilTemperatureSensor registered with dynamic config system");
    }

    // Take initial reading to establish baseline
    GetReading();
}

/**
 * @brief Gets the list of supported temperature units
 * @return Vector of supported unit strings
 *
 * Returns the available temperature units that this sensor can convert to.
 * Supports both Celsius and Fahrenheit for international compatibility.
 * Used by the configuration system for unit selection validation.
 */
std::vector<std::string> OilTemperatureSensor::GetSupportedUnits() const
{
    log_v("GetSupportedUnits() called");
    return {"C", "F"};
}

/**
 * @brief Sets the target unit for temperature readings
 * @param unit Target temperature unit ("C" or "F")
 *
 * Validates the requested unit against supported units and updates the target
 * unit for future readings. Falls back to Celsius if an unsupported unit is
 * requested and reports a warning error. Used by both initialization and live
 * configuration updates to change the display unit.
 */
void OilTemperatureSensor::SetTargetUnit(const std::string &unit)
{
    log_v("SetTargetUnit() called");
    // Validate unit is supported
    auto supportedUnits = GetSupportedUnits();
    if (!SensorHelper::IsUnitSupported(unit, supportedUnits))
    {
        // Use static string to avoid allocation in error path
        static const char* errorMsg = "Unsupported temperature unit requested. Using default C.";
        ErrorManager::Instance().ReportError(ErrorLevel::WARNING, "OilTemperatureSensor", errorMsg);
        targetUnit_ = "C";
    }
    else
    {
        targetUnit_ = unit;
    }
}

/**
 * @brief Gets the current temperature reading in the configured unit
 * @return Reading containing temperature value or invalid reading on error
 *
 * Performs rate-limited ADC sampling, validates the raw reading, and converts
 * to the target temperature unit. Includes change detection and error handling
 * for out-of-range ADC values. Updates the cached reading only when the value
 * changes to minimize unnecessary processing. This is the main interface method
 * for obtaining temperature measurements.
 */
Reading OilTemperatureSensor::GetReading()
{
    // Check if enough time has passed for update
    if (SensorHelper::ShouldUpdate(lastUpdateTime_, updateIntervalMs_))
    {
        // Read raw value from ADC
        int32_t rawValue = ReadRawValue();

        // Validate ADC reading
        if (!SensorHelper::IsValidAdcReading(rawValue))
        {
            // Use char buffer to avoid std::to_string allocation in error path
            static char errorBuffer[64];
            snprintf(errorBuffer, sizeof(errorBuffer), "Raw reading out of range: %d", rawValue);
            ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "OilTemperatureSensor", errorBuffer);
            return std::monostate{}; // Return invalid reading
        }

        // Convert to target unit
        int32_t newValue = ConvertReading(rawValue);

        // Update change tracking
        DetectChange(newValue, previousReading_);

        // Only update current reading if value actually changed
        if (newValue != currentReading_)
        {
            currentReading_ = newValue;
            log_t("Temperature reading: %d %s", currentReading_, targetUnit_.c_str());
        }
    }

    return currentReading_;
}

/**
 * @brief Sets the update rate for sensor readings
 * @param updateRateMs Update interval in milliseconds
 *
 * Updates the sensor's reading interval to control the frequency of ADC sampling.
 * Lower values provide more responsive readings but increase CPU usage.
 * Used by both initialization and live configuration updates.
 */
void OilTemperatureSensor::SetUpdateRate(int updateRateMs)
{
    updateIntervalMs_ = updateRateMs;
}

// Internal methods

/**
 * @brief Reads raw ADC value from the temperature sensor
 * @return 12-bit ADC value (0-4095) representing analog voltage
 *
 * Performs a direct ADC read from the oil temperature sensor GPIO pin.
 * The reading represents the analog voltage from the temperature sensor
 * before any calibration or unit conversion is applied.
 */
int32_t OilTemperatureSensor::ReadRawValue()
{
    // Read analog value from GPIO pin (0-4095 for 12-bit ADC)
    return gpioProvider_->AnalogRead(gpio_pins::OIL_TEMPERATURE);
}

/**
 * @brief Converts raw ADC value to requested temperature unit with calibration
 * @param rawValue Raw 12-bit ADC value (0-4095)
 * @return Calibrated temperature value in the target unit
 *
 * Applies calibration scale and offset to the raw ADC reading, then converts
 * to the requested temperature unit. The base calibration maps 0-4095 ADC to
 * 0-120°C. Supports conversion to Fahrenheit (32-248°F) and Celsius (0-120°C)
 * temperature ranges for automotive applications.
 */
int32_t OilTemperatureSensor::ConvertReading(int32_t rawValue)
{
    // Apply calibration
    float calibratedValue = static_cast<float>(rawValue);
    calibratedValue = (calibratedValue * calibrationScale_) + calibrationOffset_;

    // Convert calibrated ADC value to requested temperature unit
    // Base calibration: 0-4095 ADC = 0-120°C
    int32_t calibratedRaw = static_cast<int32_t>(calibratedValue);

    if (targetUnit_ == "F")
    {
        // Direct conversion to Fahrenheit
        // ADC 0-4095 maps to 32-248°F (0-120°C converted)
        // Formula: F = (rawValue * (248-32) / 4095) + 32
        return (calibratedRaw *
                (SensorConstants::TEMPERATURE_MAX_FAHRENHEIT - SensorConstants::TEMPERATURE_MIN_FAHRENHEIT)) /
                   SensorHelper::ADC_MAX_VALUE +
               SensorConstants::TEMPERATURE_MIN_FAHRENHEIT;
    }
    else
    {
        // Direct conversion to Celsius (default)
        // ADC 0-4095 maps to 0-120°C
        return (calibratedRaw * SensorConstants::TEMPERATURE_MAX_CELSIUS) / SensorHelper::ADC_MAX_VALUE;
    }
}

/**
 * @brief Checks if sensor state has changed since last evaluation
 * @return true if temperature reading has changed significantly
 *
 * Uses the cached reading to avoid double ADC reads, as this method is called
 * during interrupt evaluation while GetReading() is called during the main loop.
 * Implements change detection for the interrupt system to trigger appropriate
 * responses to temperature changes.
 */
bool OilTemperatureSensor::HasStateChanged()
{
    // Use cached reading to avoid double ADC reads - GetReading() updates currentReading_
    int32_t cachedReading = static_cast<int32_t>(std::get<int32_t>(GetReading()));
    return DetectChange(cachedReading, previousChangeReading_);
}

/**
 * @brief Loads configuration from the preference system
 *
 * Retrieves saved configuration values from the preference service including
 * target unit, update rate, and calibration parameters. Provides sensible
 * defaults for any missing configuration values. This enables persistent
 * configuration storage and restoration across system restarts.
 */
void OilTemperatureSensor::LoadConfiguration()
{
    if (!preferenceService_) return;

    // Load using type-safe config system with static constants
    if (auto unitValue = preferenceService_->QueryConfig<std::string>(CONFIG_UNIT)) {
        targetUnit_ = *unitValue;
    } else {
        targetUnit_ = "C";
    }

    if (auto rateValue = preferenceService_->QueryConfig<int>(CONFIG_UPDATE_RATE)) {
        updateIntervalMs_ = *rateValue;
    } else {
        updateIntervalMs_ = 500;
    }

    if (auto offsetValue = preferenceService_->QueryConfig<float>(CONFIG_OFFSET)) {
        calibrationOffset_ = *offsetValue;
    } else {
        calibrationOffset_ = 0.0f;
    }

    if (auto scaleValue = preferenceService_->QueryConfig<float>(CONFIG_SCALE)) {
        calibrationScale_ = *scaleValue;
    } else {
        calibrationScale_ = 1.0f;
    }

    log_i("Loaded oil temperature sensor configuration: unit=%s, rate=%lu, offset=%.2f, scale=%.2f",
          targetUnit_.c_str(), updateIntervalMs_, calibrationOffset_, calibrationScale_);
}

/**
 * @brief Register configuration with dynamic config system
 * @details Implements component self-registration pattern from dynamic-config-implementation.md
 * This allows the sensor to define its own configuration requirements including:
 * - Temperature unit selection (C/F)
 */
/// - Update rate options
/// - Calibration parameters with validation ranges
void OilTemperatureSensor::RegisterConfiguration()
{
    if (!preferenceService_) return;

    using namespace Config;

    // Create configuration section for this sensor component
    ConfigSection section("OilTemperatureSensor", CONFIG_SECTION, "Oil Temperature Sensor");
    section.displayOrder = 3; // Controls UI ordering in config menus

    // Temperature unit selection - enum with C/F options
    section.AddItem(ConfigItem("unit", "Temperature Unit", ConfigValueType::Enum,
        std::string("C"), ConfigMetadata("C,F")));

    // Update rate - predefined options for sensor reading frequency
    section.AddItem(ConfigItem("update_rate", "Update Rate (ms)", ConfigValueType::Enum,
        500, ConfigMetadata("250,500,1000,2000")));

    // Calibration offset - float with range validation (-5.0 to +5.0)
    section.AddItem(ConfigItem("offset", "Calibration Offset", ConfigValueType::Float,
        0.0f, ConfigMetadata("-5.0,5.0")));

    // Calibration scale - float with range validation (0.9 to 1.1)
    section.AddItem(ConfigItem("scale", "Calibration Scale", ConfigValueType::Float,
        1.0f, ConfigMetadata("0.9,1.1")));

    // Register with preference service for persistence and UI generation
    preferenceService_->RegisterConfigSection(section);
    log_i("Registered oil temperature sensor configuration");
}

/**
 * @brief Register callbacks for live configuration updates
 * @details Sets up real-time response to configuration changes without requiring restart
 * Implements live update system from dynamic-config-implementation.md Phase 4
 */
void OilTemperatureSensor::RegisterLiveUpdateCallbacks() {
    if (!preferenceService_) return;

    // Register callback to watch for changes to oil_temperature section
    // Lambda captures 'this' to allow access to sensor methods
    auto callback = [this](const std::string& fullKey,
                          const std::optional<Config::ConfigValue>& oldValue,
                          const Config::ConfigValue& newValue) {

        // Handle temperature unit change (C/F) - immediate effect on readings
        if (fullKey == CONFIG_UNIT) {
            if (auto newUnit = Config::ConfigValueHelper::GetValue<std::string>(newValue)) {
                SetTargetUnit(*newUnit);
                log_i("Oil temperature unit changed to: %s", newUnit->c_str());
            }
        }

        // Handle update rate change - controls sensor reading frequency
        else if (fullKey == CONFIG_UPDATE_RATE) {
            if (auto newRate = Config::ConfigValueHelper::GetValue<int>(newValue)) {
                SetUpdateRate(*newRate);
                log_i("Oil temperature update rate changed to: %d ms", *newRate);
            }
        }

        // Handle calibration offset change
        else if (fullKey == CONFIG_OFFSET) {
            if (auto newOffset = Config::ConfigValueHelper::GetValue<float>(newValue)) {
                calibrationOffset_ = *newOffset;
                log_i("Oil temperature calibration offset changed to: %.2f", *newOffset);
            }
        }

        // Handle calibration scale change
        else if (fullKey == CONFIG_SCALE) {
            if (auto newScale = Config::ConfigValueHelper::GetValue<float>(newValue)) {
                calibrationScale_ = *newScale;
                log_i("Oil temperature calibration scale changed to: %.2f", *newScale);
            }
        }
    };

    // Register for all oil_temperature section changes
    configCallbackId_ = preferenceService_->RegisterChangeCallback("oil_temperature", callback);
}