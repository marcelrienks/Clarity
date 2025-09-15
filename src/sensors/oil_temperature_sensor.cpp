#include "sensors/oil_temperature_sensor.h"
#include "managers/error_manager.h"
#include "config/config_types.h"
#include "utilities/logging.h"
#include <Arduino.h>
#include <esp32-hal-log.h>

// Constructors and Destructors

/// @brief Constructor for OilTemperatureSensor
OilTemperatureSensor::OilTemperatureSensor(IGpioProvider *gpioProvider, int updateRateMs)
    : gpioProvider_(gpioProvider), updateIntervalMs_(updateRateMs)
{
    log_v("OilTemperatureSensor() constructor called");
    // Set default unit to Celsius
    targetUnit_ = "C";
    currentReading_ = 0;
}

/// @brief Constructor for OilTemperatureSensor with preference service for calibration
OilTemperatureSensor::OilTemperatureSensor(IGpioProvider *gpioProvider, IPreferenceService *preferenceService, int updateRateMs)
    : gpioProvider_(gpioProvider), preferenceService_(preferenceService), updateIntervalMs_(updateRateMs)
{
    log_v("OilTemperatureSensor() constructor with preference service called");
    // Set default unit to Celsius
    targetUnit_ = "C";
    currentReading_ = 0;
}

// Core Functionality Methods

/// @brief Initialize the oil temperature sensor hardware
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

        // Check if dynamic config is enabled
        std::string dynamicEnabled = preferenceService_->GetPreference("dynamic_ui_enabled");
        if (dynamicEnabled == "true" || dynamicEnabled.empty()) {
            // Access dynamic config capabilities via static_cast (DynamicPreferenceManager implements both interfaces)
            dynamicConfigService_ = static_cast<IDynamicConfigService*>(preferenceService_);
            if (dynamicConfigService_) {
                RegisterConfiguration();
                RegisterLiveUpdateCallbacks();
                log_d("OilTemperatureSensor registered with dynamic config system");
            }
        }
    }

    // Take initial reading to establish baseline
    GetReading();
}

/// @brief Get supported temperature units
std::vector<std::string> OilTemperatureSensor::GetSupportedUnits() const
{
    log_v("GetSupportedUnits() called");
    return {"C", "F"};
}

/// @brief Set the target unit for temperature readings
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

/// @brief Get the current temperature reading
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

/// @brief Set the update rate for sensor readings
void OilTemperatureSensor::SetUpdateRate(int updateRateMs)
{
    updateIntervalMs_ = updateRateMs;
}

// Internal methods

/// @brief Read raw ADC value from temperature sensor
int32_t OilTemperatureSensor::ReadRawValue()
{
    // Read analog value from GPIO pin (0-4095 for 12-bit ADC)
    return gpioProvider_->AnalogRead(gpio_pins::OIL_TEMPERATURE);
}

/// @brief Convert raw ADC value to requested temperature unit
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

/// @brief Check if sensor state has changed since last evaluation
bool OilTemperatureSensor::HasStateChanged()
{
    // Use cached reading to avoid double ADC reads - GetReading() updates currentReading_
    int32_t cachedReading = static_cast<int32_t>(std::get<int32_t>(GetReading()));
    return DetectChange(cachedReading, previousChangeReading_);
}

/// @brief Load configuration from preference system
void OilTemperatureSensor::LoadConfiguration()
{
    if (!preferenceService_) return;

    // Load from legacy config
    const Configs& config = preferenceService_->GetConfig();
    targetUnit_ = config.tempUnit;
    updateIntervalMs_ = config.updateRate;
    calibrationOffset_ = config.tempOffset;
    calibrationScale_ = config.tempScale;

    log_d("Loaded oil temperature sensor configuration: unit=%s, rate=%lu, offset=%.2f, scale=%.2f",
          targetUnit_.c_str(), updateIntervalMs_, calibrationOffset_, calibrationScale_);
}

/// @brief Register configuration with dynamic config system
void OilTemperatureSensor::RegisterConfiguration()
{
    if (!dynamicConfigService_) return;

    using namespace Config;

    ConfigSection section("OilTemperatureSensor", "oil_temperature", "Oil Temperature Sensor");
    section.displayOrder = 3;

    // Temperature unit selection
    section.AddItem(ConfigItem("unit", "Temperature Unit", ConfigValueType::Enum,
        std::string("C"), ConfigMetadata("C,F")));

    // Update rate
    section.AddItem(ConfigItem("update_rate", "Update Rate (ms)", ConfigValueType::Enum,
        500, ConfigMetadata("250,500,1000,2000")));

    // Calibration offset
    section.AddItem(ConfigItem("offset", "Calibration Offset", ConfigValueType::Float,
        0.0f, ConfigMetadata("-5.0,5.0")));

    // Calibration scale
    section.AddItem(ConfigItem("scale", "Calibration Scale", ConfigValueType::Float,
        1.0f, ConfigMetadata("0.9,1.1")));

    dynamicConfigService_->RegisterConfigSection(section);
    log_d("Registered oil temperature sensor configuration");
}

void OilTemperatureSensor::RegisterLiveUpdateCallbacks() {
    if (!dynamicConfigService_) return;

    // Register callback for our section changes
    auto callback = [this](const std::string& fullKey,
                          const std::optional<Config::ConfigValue>& oldValue,
                          const Config::ConfigValue& newValue) {
        log_d("OilTemperatureSensor: Config change detected for %s", fullKey.c_str());

        // Handle unit change
        if (fullKey == "oil_temperature.unit") {
            if (auto newUnit = Config::ConfigValueHelper::GetValue<std::string>(newValue)) {
                SetTargetUnit(*newUnit);
                log_i("Oil temperature unit changed to: %s", newUnit->c_str());
            }
        }

        // Handle update rate change
        else if (fullKey == "oil_temperature.update_rate") {
            if (auto newRate = Config::ConfigValueHelper::GetValue<int>(newValue)) {
                SetUpdateRate(*newRate);
                log_i("Oil temperature update rate changed to: %d ms", *newRate);
            }
        }

        // Handle calibration offset change
        else if (fullKey == "oil_temperature.offset") {
            if (auto newOffset = Config::ConfigValueHelper::GetValue<float>(newValue)) {
                calibrationOffset_ = *newOffset;
                log_i("Oil temperature calibration offset changed to: %.2f", *newOffset);
            }
        }

        // Handle calibration scale change
        else if (fullKey == "oil_temperature.scale") {
            if (auto newScale = Config::ConfigValueHelper::GetValue<float>(newValue)) {
                calibrationScale_ = *newScale;
                log_i("Oil temperature calibration scale changed to: %.2f", *newScale);
            }
        }
    };

    // Register for all oil_temperature section changes
    configCallbackId_ = dynamicConfigService_->RegisterChangeCallback("oil_temperature", callback);
    log_d("Registered live update callback with ID: %u", configCallbackId_);
}