#include "sensors/oil_temperature_sensor.h"
#include "managers/error_manager.h"
#include <Arduino.h>
#include <esp32-hal-log.h>

// Constructors and Destructors

/// @brief Constructor for OilTemperatureSensor
OilTemperatureSensor::OilTemperatureSensor(IGpioProvider *gpioProvider, int updateRateMs)
    : gpioProvider_(gpioProvider), updateIntervalMs_(updateRateMs)
{
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
        ErrorManager::Instance().ReportError(ErrorLevel::WARNING, "OilTemperatureSensor",
                                             "Unsupported unit requested: " + unit + ". Using default C.");
        targetUnit_ = "C";
    }
    else
    {
        targetUnit_ = unit;
        log_d("Temperature unit set to: %s", targetUnit_.c_str());
    }
}

/// @brief Get the current temperature reading
Reading OilTemperatureSensor::GetReading()
{
    log_v("GetReading() called");
    // Check if enough time has passed for update
    if (SensorHelper::ShouldUpdate(lastUpdateTime_, updateIntervalMs_))
    {
        // Read raw value from ADC
        int32_t rawValue = ReadRawValue();

        // Validate ADC reading
        if (!SensorHelper::IsValidAdcReading(rawValue))
        {
            ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "OilTemperatureSensor",
                                                 "Raw reading out of range: " + std::to_string(rawValue));
            return std::monostate{}; // Return invalid reading
        }

        // Convert to target unit
        int32_t newValue = ConvertReading(rawValue);

        // Only update if value actually changed
        if (newValue != currentReading_)
        {
            currentReading_ = newValue;
            log_i("Temperature reading changed to %d %s (raw: %d)", currentReading_, targetUnit_.c_str(), rawValue);
        }
    }

    return currentReading_;
}

// Internal methods

/// @brief Read raw ADC value from temperature sensor
int32_t OilTemperatureSensor::ReadRawValue()
{
    log_v("ReadRawValue() called");
    // Read analog value from GPIO pin (0-4095 for 12-bit ADC)
    return gpioProvider_->AnalogRead(gpio_pins::OIL_TEMPERATURE);
}

/// @brief Convert raw ADC value to requested temperature unit
int32_t OilTemperatureSensor::ConvertReading(int32_t rawValue)
{
    log_v("ConvertReading() called");
    // Convert ADC value directly to requested temperature unit
    // Base calibration: 0-4095 ADC = 0-120°C

    if (targetUnit_ == "F")
    {
        // Direct conversion to Fahrenheit
        // ADC 0-4095 maps to 32-248°F (0-120°C converted)
        // Formula: F = (rawValue * (248-32) / 4095) + 32
        return (rawValue *
                (SensorConstants::TEMPERATURE_MAX_FAHRENHEIT - SensorConstants::TEMPERATURE_MIN_FAHRENHEIT)) /
                   SensorHelper::ADC_MAX_VALUE +
               SensorConstants::TEMPERATURE_MIN_FAHRENHEIT;
    }
    else
    {
        // Direct conversion to Celsius (default)
        // ADC 0-4095 maps to 0-120°C
        return (rawValue * SensorConstants::TEMPERATURE_MAX_CELSIUS) / SensorHelper::ADC_MAX_VALUE;
    }
}