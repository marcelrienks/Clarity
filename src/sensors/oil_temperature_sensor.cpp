#include "sensors/oil_temperature_sensor.h"
#include <Arduino.h>
#include <esp32-hal-log.h>

// Constructors and Destructors

/// @brief Constructor for OilTemperatureSensor
OilTemperatureSensor::OilTemperatureSensor(IGpioProvider* gpioProvider, int updateRateMs) 
    : UnitAwareSensor(gpioProvider, updateRateMs)
{
    // Set default unit to Celsius
    targetUnit_ = "C";
}

// Core Functionality Methods

/// @brief Initialize the oil temperature sensor hardware
void OilTemperatureSensor::Init()
{
    // Configure GPIO pin for analog input
    log_d("Initializing oil temperature sensor ADC configuration");
    
    // Configure ADC resolution and attenuation for direct 3.3V operation
    analogReadResolution(12); // 12-bit resolution (0-4095)
    analogSetAttenuation(ADC_11db); // 0-3.3V range
    
    // Take initial reading to establish baseline
    GetReading();
}

/// @brief Get supported temperature units
std::vector<std::string> OilTemperatureSensor::GetSupportedUnits() const
{
    return {"C", "F"};
}

// Protected UnitAwareSensor implementation

/// @brief Read raw ADC value from temperature sensor
int32_t OilTemperatureSensor::ReadRawValue()
{
    // Read analog value from GPIO pin (0-4095 for 12-bit ADC)
    return gpioProvider_->AnalogRead(gpio_pins::OIL_TEMPERATURE);
}

/// @brief Convert raw ADC value to requested temperature unit
int32_t OilTemperatureSensor::ConvertReading(int32_t rawValue)
{
    // Convert ADC value to requested temperature unit
    // Base calibration: 0-4095 ADC = 0-120°C
    
    // First convert to Celsius
    int32_t celsiusValue = (rawValue * TEMPERATURE_MAX_CELSIUS) / ADC_MAX_VALUE;
    
    if (targetUnit_ == "F") {
        // Convert Celsius to Fahrenheit: F = C * 9/5 + 32
        return (celsiusValue * 9) / 5 + 32;
    } else {
        // Default to Celsius
        return celsiusValue;
    }
}