#include "sensors/oil_pressure_sensor.h"
#include "managers/error_manager.h"
#include "config/config_types.h"
#include "utilities/logging.h"
#include <Arduino.h>
#include <esp32-hal-log.h>

// Constructors and Destructors

/**
 * @brief Constructs an oil pressure sensor with basic GPIO provider dependency
 * @param gpioProvider Pointer to GPIO provider interface for ADC reading
 * @param updateRateMs Update interval in milliseconds for sensor readings
 *
 * Initializes the sensor with GPIO provider for ADC operations and sets default
 * configuration values. Uses Bar as the default pressure unit. This constructor
 * is used when no preference service is available for dynamic configuration.
 */
OilPressureSensor::OilPressureSensor(IGpioProvider *gpioProvider, int updateRateMs)
    : gpioProvider_(gpioProvider), updateIntervalMs_(updateRateMs)
{
    log_v("OilPressureSensor() constructor called");
    // Set default unit to Bar
    targetUnit_ = "Bar";
    currentReading_ = 0;
}

/**
 * @brief Constructs an oil pressure sensor with preference service for dynamic configuration
 * @param gpioProvider Pointer to GPIO provider interface for ADC reading
 * @param preferenceService Pointer to preference service for configuration persistence
 * @param updateRateMs Default update interval in milliseconds for sensor readings
 *
 * Enhanced constructor that enables dynamic configuration through the preference service.
 * Allows for real-time calibration adjustments and unit changes. The preference service
 * enables live configuration updates and persistent storage of sensor settings.
 */
OilPressureSensor::OilPressureSensor(IGpioProvider *gpioProvider, IPreferenceService *preferenceService, int updateRateMs)
    : gpioProvider_(gpioProvider), preferenceService_(preferenceService), updateIntervalMs_(updateRateMs)
{
    log_v("OilPressureSensor() constructor with preference service called");
    // Set default unit to Bar
    targetUnit_ = "Bar";
    currentReading_ = 0;
}

// Core Functionality Methods

/**
 * @brief Initializes the oil pressure sensor hardware and configuration
 *
 * Configures the ADC for 12-bit resolution with 11dB attenuation for 0-3.3V range.
 * Loads configuration from preference service if available and registers for
 * dynamic configuration updates. Takes an initial reading to establish baseline
 * values. This is part of the ISensor interface implementation.
 */
void OilPressureSensor::Init()
{
    log_v("Init() called");
    // Configure GPIO pin for analog input

    // Configure ADC resolution and attenuation for direct 3.3V operation
    analogReadResolution(12);       // 12-bit resolution (0-4095)
    analogSetAttenuation(ADC_11db); // 0-3.3V range

    // Register with dynamic config system if available and load configuration
    if (preferenceService_) {
        LoadConfiguration();

        // Use unified preference service interface for dynamic configuration
        RegisterConfiguration();
        RegisterLiveUpdateCallbacks();
        log_i("OilPressureSensor registered with dynamic config system");
    }

    // Take initial reading to establish baseline
    GetReading();
}

/**
 * @brief Gets the list of supported pressure units
 * @return Vector of supported unit strings
 *
 * Returns the available pressure units that this sensor can convert to.
 * Supports automotive-standard pressure units including Bar (standard),
 * PSI (imperial), and kPa (metric). Used by the configuration system
 * for unit selection validation.
 */
std::vector<std::string> OilPressureSensor::GetSupportedUnits() const
{
    log_v("GetSupportedUnits() called");
    return {"Bar", "PSI", "kPa"};
}

/**
 * @brief Sets the target unit for pressure readings
 * @param unit Target pressure unit ("Bar", "PSI", or "kPa")
 *
 * Validates the requested unit against supported units and updates the target
 * unit for future readings. Falls back to Bar if an unsupported unit is requested
 * and reports a warning error. Used by both initialization and live configuration
 * updates to change the display unit.
 */
void OilPressureSensor::SetTargetUnit(const std::string &unit)
{
    log_v("SetTargetUnit() called");
    // Validate unit is supported
    auto supportedUnits = GetSupportedUnits();
    if (!SensorHelper::IsUnitSupported(unit, supportedUnits))
    {
        // Use static string to avoid allocation in error path
        static const char* errorMsg = "Unsupported pressure unit requested. Using default Bar.";
        ErrorManager::Instance().ReportError(ErrorLevel::WARNING, "OilPressureSensor", errorMsg);
        targetUnit_ = "Bar";
    }
    else
    {
        targetUnit_ = unit;
    }
}

/**
 * @brief Gets the current pressure reading in the configured unit
 * @return Reading containing pressure value or invalid reading on error
 *
 * Performs rate-limited ADC sampling, validates the raw reading, and converts
 * to the target pressure unit. Includes change detection and error handling
 * for out-of-range ADC values. Updates the cached reading only when the value
 * changes to minimize unnecessary processing. This is the main interface method
 * for obtaining pressure measurements.
 */
Reading OilPressureSensor::GetReading()
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
            ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "OilPressureSensor", errorBuffer);
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
            log_t("Pressure reading: %d %s", currentReading_, targetUnit_.c_str());
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
void OilPressureSensor::SetUpdateRate(int updateRateMs)
{
    updateIntervalMs_ = updateRateMs;
}

// Internal methods

/**
 * @brief Reads raw ADC value from the pressure sensor
 * @return 12-bit ADC value (0-4095) representing analog voltage
 *
 * Performs a direct ADC read from the oil pressure sensor GPIO pin.
 * The reading represents the analog voltage from the pressure transducer
 * before any calibration or unit conversion is applied.
 */
int32_t OilPressureSensor::ReadRawValue()
{
    // Read analog value from GPIO pin (0-4095 for 12-bit ADC)
    return gpioProvider_->AnalogRead(gpio_pins::OIL_PRESSURE);
}

/**
 * @brief Converts raw ADC value to requested pressure unit with calibration
 * @param rawValue Raw 12-bit ADC value (0-4095)
 * @return Calibrated pressure value in the target unit
 *
 * Applies calibration scale and offset to the raw ADC reading, then converts
 * to the requested pressure unit. The base calibration maps 0-4095 ADC to
 * 0-10 Bar. Supports conversion to PSI (0-145), kPa (0-1000), and Bar (0-10)
 * pressure ranges for automotive applications.
 */
int32_t OilPressureSensor::ConvertReading(int32_t rawValue)
{
    // Apply calibration
    float calibratedValue = static_cast<float>(rawValue);
    calibratedValue = (calibratedValue * calibrationScale_) + calibrationOffset_;

    // Convert calibrated ADC value to requested pressure unit
    // Base calibration: 0-4095 ADC = 0-10 Bar
    int32_t calibratedRaw = static_cast<int32_t>(calibratedValue);

    if (targetUnit_ == "PSI")
    {
        // Map 0-4095 ADC to 0-145 PSI (0-10 Bar equivalent)
        return (calibratedRaw * SensorConstants::PRESSURE_MAX_PSI) / SensorHelper::ADC_MAX_VALUE;
    }
    else if (targetUnit_ == "kPa")
    {
        // Map 0-4095 ADC to 0-1000 kPa (0-10 Bar equivalent)
        return (calibratedRaw * SensorConstants::PRESSURE_MAX_KPA) / SensorHelper::ADC_MAX_VALUE;
    }
    else
    {
        // Default Bar mapping: 0-4095 ADC to 0-10 Bar
        return (calibratedRaw * SensorConstants::PRESSURE_MAX_BAR) / SensorHelper::ADC_MAX_VALUE;
    }
}

/**
 * @brief Checks if sensor state has changed since last evaluation
 * @return true if pressure reading has changed significantly
 *
 * Uses the cached reading to avoid double ADC reads, as this method is called
 * during interrupt evaluation while GetReading() is called during the main loop.
 * Implements change detection for the interrupt system to trigger appropriate
 * responses to pressure changes.
 */
bool OilPressureSensor::HasStateChanged()
{
    // Use cached reading to avoid double ADC reads - GetReading() updates currentReading_
    // This method is called during interrupt evaluation, GetReading() during main loop
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
void OilPressureSensor::LoadConfiguration()
{
    if (!preferenceService_) return;

    // Load using type-safe config system with static constants
    if (auto unitValue = preferenceService_->QueryConfig<std::string>(CONFIG_UNIT)) {
        targetUnit_ = *unitValue;
    } else {
        targetUnit_ = "Bar";
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

    log_i("Loaded oil pressure sensor configuration: unit=%s, rate=%lu, offset=%.2f, scale=%.2f",
          targetUnit_.c_str(), updateIntervalMs_, calibrationOffset_, calibrationScale_);
}

/**
 * @brief Registers configuration schema with the dynamic config system
 *
 * Defines the configuration options available for the oil pressure sensor
 * including unit selection, update rate, and calibration parameters.
 * Creates a configuration section that appears in the dynamic UI for
 * real-time sensor adjustment and calibration.
 */
void OilPressureSensor::RegisterConfiguration()
{
    if (!preferenceService_) return;

    using namespace Config;

    ConfigSection section("OilPressureSensor", CONFIG_SECTION, "Oil Pressure Sensor");
    section.displayOrder = 2;

    // Pressure unit selection
    section.AddItem(ConfigItem("unit", "Pressure Unit", std::string("Bar"),
        ConfigMetadata("PSI,Bar,kPa", ConfigItemType::Selection)));

    // Update rate
    section.AddItem(ConfigItem("update_rate", "Update Rate (ms)", 500,
        ConfigMetadata("250,500,1000,2000", ConfigItemType::Selection)));

    // Calibration offset
    section.AddItem(ConfigItem("offset", "Calibration Offset", 0.0f,
        ConfigMetadata("-1.0,-0.5,-0.2,-0.1,0.0,0.1,0.2,0.5,1.0", ConfigItemType::Selection)));

    // Calibration scale
    section.AddItem(ConfigItem("scale", "Calibration Scale", 1.0f,
        ConfigMetadata("0.9,0.95,1.0,1.05,1.1", ConfigItemType::Selection)));

    preferenceService_->RegisterConfigSection(section);
    log_i("Registered oil pressure sensor configuration");
}

/**
 * @brief Registers live update callbacks for real-time configuration changes
 *
 * Sets up callback handlers that respond immediately to configuration changes
 * in the dynamic UI. Enables real-time tuning of sensor parameters including
 * unit changes, update rate adjustments, and calibration modifications without
 * requiring a system restart. This provides a seamless user experience for
 * sensor calibration and adjustment.
 */
void OilPressureSensor::RegisterLiveUpdateCallbacks() {
    if (!preferenceService_) return;

    // Register callback for our section changes
    auto callback = [this](const std::string& fullKey,
                          const std::optional<Config::ConfigValue>& oldValue,
                          const Config::ConfigValue& newValue) {

        // Handle unit change
        if (fullKey == CONFIG_UNIT) {
            if (auto newUnit = Config::ConfigValueHelper::GetValue<std::string>(newValue)) {
                SetTargetUnit(*newUnit);
                log_i("Oil pressure unit changed to: %s", newUnit->c_str());
            }
        }

        // Handle update rate change
        else if (fullKey == CONFIG_UPDATE_RATE) {
            if (auto newRate = Config::ConfigValueHelper::GetValue<int>(newValue)) {
                SetUpdateRate(*newRate);
                log_i("Oil pressure update rate changed to: %d ms", *newRate);
            }
        }

        // Handle calibration offset change
        else if (fullKey == CONFIG_OFFSET) {
            if (auto newOffset = Config::ConfigValueHelper::GetValue<float>(newValue)) {
                calibrationOffset_ = *newOffset;
                log_i("Oil pressure calibration offset changed to: %.2f", *newOffset);
            }
        }

        // Handle calibration scale change
        else if (fullKey == CONFIG_SCALE) {
            if (auto newScale = Config::ConfigValueHelper::GetValue<float>(newValue)) {
                calibrationScale_ = *newScale;
                log_i("Oil pressure calibration scale changed to: %.2f", *newScale);
            }
        }
    };

    // Register for all oil_pressure section changes
    configCallbackId_ = preferenceService_->RegisterChangeCallback("oil_pressure", callback);
}