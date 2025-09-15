#include "sensors/oil_pressure_sensor.h"
#include "managers/error_manager.h"
#include "config/config_types.h"
#include "utilities/logging.h"
#include <Arduino.h>
#include <esp32-hal-log.h>

// Constructors and Destructors

/// @brief Constructor for OilPressureSensor
OilPressureSensor::OilPressureSensor(IGpioProvider *gpioProvider, int updateRateMs)
    : gpioProvider_(gpioProvider), updateIntervalMs_(updateRateMs)
{
    log_v("OilPressureSensor() constructor called");
    // Set default unit to Bar
    targetUnit_ = "Bar";
    currentReading_ = 0;
}

/// @brief Constructor for OilPressureSensor with preference service for calibration
OilPressureSensor::OilPressureSensor(IGpioProvider *gpioProvider, IPreferenceService *preferenceService, int updateRateMs)
    : gpioProvider_(gpioProvider), preferenceService_(preferenceService), updateIntervalMs_(updateRateMs)
{
    log_v("OilPressureSensor() constructor with preference service called");
    // Set default unit to Bar
    targetUnit_ = "Bar";
    currentReading_ = 0;
}

// Core Functionality Methods

/// @brief Initialize the oil pressure sensor hardware
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

        // Check if we should register our configuration
        std::string dynamicEnabled = preferenceService_->GetPreference("dynamic_ui_enabled");
        if (dynamicEnabled == "true" || dynamicEnabled.empty()) {
            // Use unified preference service interface for dynamic configuration
            RegisterConfiguration();
            RegisterLiveUpdateCallbacks();
            log_i("OilPressureSensor registered with dynamic config system");
        }
    }

    // Take initial reading to establish baseline
    GetReading();
}

/// @brief Get supported pressure units
std::vector<std::string> OilPressureSensor::GetSupportedUnits() const
{
    log_v("GetSupportedUnits() called");
    return {"Bar", "PSI", "kPa"};
}

/// @brief Set the target unit for pressure readings
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

/// @brief Get the current pressure reading
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

/// @brief Set the update rate for sensor readings
void OilPressureSensor::SetUpdateRate(int updateRateMs)
{
    updateIntervalMs_ = updateRateMs;
}

// Internal methods

/// @brief Read raw ADC value from pressure sensor
int32_t OilPressureSensor::ReadRawValue()
{
    // Read analog value from GPIO pin (0-4095 for 12-bit ADC)
    return gpioProvider_->AnalogRead(gpio_pins::OIL_PRESSURE);
}

/// @brief Convert raw ADC value to requested pressure unit
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

/// @brief Check if sensor state has changed since last evaluation
bool OilPressureSensor::HasStateChanged()
{
    // Use cached reading to avoid double ADC reads - GetReading() updates currentReading_
    // This method is called during interrupt evaluation, GetReading() during main loop
    int32_t cachedReading = static_cast<int32_t>(std::get<int32_t>(GetReading()));
    return DetectChange(cachedReading, previousChangeReading_);
}

/// @brief Load configuration from preference system
void OilPressureSensor::LoadConfiguration()
{
    if (!preferenceService_) return;

    // Load from dynamic config system
    targetUnit_ = preferenceService_->GetPreference("oil_pressure.unit");
    if (targetUnit_.empty()) targetUnit_ = "Bar";

    std::string updateRateStr = preferenceService_->GetPreference("oil_pressure.update_rate");
    if (!updateRateStr.empty()) {
        updateIntervalMs_ = std::stoi(updateRateStr);
    } else {
        updateIntervalMs_ = 500;
    }

    std::string offsetStr = preferenceService_->GetPreference("oil_pressure.offset");
    if (!offsetStr.empty()) {
        calibrationOffset_ = std::stof(offsetStr);
    } else {
        calibrationOffset_ = 0.0f;
    }

    std::string scaleStr = preferenceService_->GetPreference("oil_pressure.scale");
    if (!scaleStr.empty()) {
        calibrationScale_ = std::stof(scaleStr);
    } else {
        calibrationScale_ = 1.0f;
    }

    log_i("Loaded oil pressure sensor configuration: unit=%s, rate=%lu, offset=%.2f, scale=%.2f",
          targetUnit_.c_str(), updateIntervalMs_, calibrationOffset_, calibrationScale_);
}

/// @brief Register configuration with dynamic config system
void OilPressureSensor::RegisterConfiguration()
{
    if (!preferenceService_) return;

    using namespace Config;

    ConfigSection section("OilPressureSensor", "oil_pressure", "Oil Pressure Sensor");
    section.displayOrder = 2;

    // Pressure unit selection
    section.AddItem(ConfigItem("unit", "Pressure Unit", ConfigValueType::Enum,
        std::string("Bar"), ConfigMetadata("PSI,Bar,kPa")));

    // Update rate
    section.AddItem(ConfigItem("update_rate", "Update Rate (ms)", ConfigValueType::Enum,
        500, ConfigMetadata("250,500,1000,2000")));

    // Calibration offset
    section.AddItem(ConfigItem("offset", "Calibration Offset", ConfigValueType::Float,
        0.0f, ConfigMetadata("-1.0,1.0")));

    // Calibration scale
    section.AddItem(ConfigItem("scale", "Calibration Scale", ConfigValueType::Float,
        1.0f, ConfigMetadata("0.9,1.1")));

    preferenceService_->RegisterConfigSection(section);
    log_i("Registered oil pressure sensor configuration");
}

void OilPressureSensor::RegisterLiveUpdateCallbacks() {
    if (!preferenceService_) return;

    // Register callback for our section changes
    auto callback = [this](const std::string& fullKey,
                          const std::optional<Config::ConfigValue>& oldValue,
                          const Config::ConfigValue& newValue) {

        // Handle unit change
        if (fullKey == "oil_pressure.unit") {
            if (auto newUnit = Config::ConfigValueHelper::GetValue<std::string>(newValue)) {
                SetTargetUnit(*newUnit);
                log_i("Oil pressure unit changed to: %s", newUnit->c_str());
            }
        }

        // Handle update rate change
        else if (fullKey == "oil_pressure.update_rate") {
            if (auto newRate = Config::ConfigValueHelper::GetValue<int>(newValue)) {
                SetUpdateRate(*newRate);
                log_i("Oil pressure update rate changed to: %d ms", *newRate);
            }
        }

        // Handle calibration offset change
        else if (fullKey == "oil_pressure.offset") {
            if (auto newOffset = Config::ConfigValueHelper::GetValue<float>(newValue)) {
                calibrationOffset_ = *newOffset;
                log_i("Oil pressure calibration offset changed to: %.2f", *newOffset);
            }
        }

        // Handle calibration scale change
        else if (fullKey == "oil_pressure.scale") {
            if (auto newScale = Config::ConfigValueHelper::GetValue<float>(newValue)) {
                calibrationScale_ = *newScale;
                log_i("Oil pressure calibration scale changed to: %.2f", *newScale);
            }
        }
    };

    // Register for all oil_pressure section changes
    configCallbackId_ = preferenceService_->RegisterChangeCallback("oil_pressure", callback);
}