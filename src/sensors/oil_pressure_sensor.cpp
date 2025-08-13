#include "sensors/oil_pressure_sensor.h"
#include "managers/error_manager.h"
#include <Arduino.h>
#include <esp32-hal-log.h>

// Constructors and Destructors

/// @brief Constructor for OilPressureSensor
OilPressureSensor::OilPressureSensor(IGpioProvider *gpioProvider, int updateRateMs)
    : gpioProvider_(gpioProvider), updateIntervalMs_(updateRateMs)
{
    // Set default unit to Bar
    targetUnit_ = "Bar";
    currentReading_ = 0;
}

// Core Functionality Methods

/// @brief Initialize the oil pressure sensor hardware
void OilPressureSensor::Init()
{
    // Configure GPIO pin for analog input
    log_d("Initializing oil pressure sensor ADC configuration");

    // Configure ADC resolution and attenuation for direct 3.3V operation
    analogReadResolution(12);       // 12-bit resolution (0-4095)
    analogSetAttenuation(ADC_11db); // 0-3.3V range

    // Take initial reading to establish baseline
    GetReading();
}

/// @brief Get supported pressure units
std::vector<std::string> OilPressureSensor::GetSupportedUnits() const
{
    return {"Bar", "PSI", "kPa"};
}

/// @brief Set the target unit for pressure readings
void OilPressureSensor::SetTargetUnit(const std::string &unit)
{
    // Validate unit is supported
    auto supportedUnits = GetSupportedUnits();
    if (!SensorHelper::IsUnitSupported(unit, supportedUnits))
    {
        ErrorManager::Instance().ReportError(ErrorLevel::WARNING, "OilPressureSensor",
                                             "Unsupported unit requested: " + unit + ". Using default Bar.");
        targetUnit_ = "Bar";
    }
    else
    {
        targetUnit_ = unit;
        log_d("Pressure unit set to: %s", targetUnit_.c_str());
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
            ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "OilPressureSensor",
                                                 "Raw reading out of range: " + std::to_string(rawValue));
            return std::monostate{}; // Return invalid reading
        }

        // Convert to target unit
        int32_t newValue = ConvertReading(rawValue);

        // Only update if value actually changed
        if (newValue != currentReading_)
        {
            currentReading_ = newValue;
            log_i("Pressure reading changed to %d %s (raw: %d)", currentReading_, targetUnit_.c_str(), rawValue);
        }
    }

    return currentReading_;
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
    // Convert ADC value to requested pressure unit
    // Base calibration: 0-4095 ADC = 0-10 Bar

    if (targetUnit_ == "PSI")
    {
        // Map 0-4095 ADC to 0-145 PSI (0-10 Bar equivalent)
        return (rawValue * SensorConstants::PRESSURE_MAX_PSI) / SensorHelper::ADC_MAX_VALUE;
    }
    else if (targetUnit_ == "kPa")
    {
        // Map 0-4095 ADC to 0-1000 kPa (0-10 Bar equivalent)
        return (rawValue * SensorConstants::PRESSURE_MAX_KPA) / SensorHelper::ADC_MAX_VALUE;
    }
    else
    {
        // Default Bar mapping: 0-4095 ADC to 0-10 Bar
        return (rawValue * SensorConstants::PRESSURE_MAX_BAR) / SensorHelper::ADC_MAX_VALUE;
    }
}