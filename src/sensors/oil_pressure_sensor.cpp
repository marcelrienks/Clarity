#include "sensors/oil_pressure_sensor.h"
#include <Arduino.h>
#include <esp32-hal-log.h>

// Constructors and Destructors

/// @brief Constructor for OilPressureSensor
OilPressureSensor::OilPressureSensor(IGpioProvider* gpioProvider, int updateRateMs) 
    : UnitAwareSensor(gpioProvider, updateRateMs)
{
    // Set default unit to Bar
    targetUnit_ = "Bar";
}

// Core Functionality Methods

/// @brief Initialize the oil pressure sensor hardware
void OilPressureSensor::Init()
{
    // Configure GPIO pin for analog input
    log_d("Initializing oil pressure sensor ADC configuration");
    
    // Configure ADC resolution and attenuation for direct 3.3V operation
    analogReadResolution(12); // 12-bit resolution (0-4095)
    analogSetAttenuation(ADC_11db); // 0-3.3V range
    
    // Take initial reading to establish baseline
    GetReading();
}

/// @brief Get supported pressure units
std::vector<std::string> OilPressureSensor::GetSupportedUnits() const
{
    return {"Bar", "PSI", "kPa"};
}

// Protected UnitAwareSensor implementation

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
    
    if (targetUnit_ == "PSI") {
        // Map 0-4095 ADC to 0-145 PSI (0-10 Bar equivalent)
        return (rawValue * PRESSURE_MAX_PSI) / ADC_MAX_VALUE;
    } else if (targetUnit_ == "kPa") {
        // Map 0-4095 ADC to 0-1000 kPa (0-10 Bar equivalent)
        return (rawValue * PRESSURE_MAX_KPA) / ADC_MAX_VALUE;
    } else {
        // Default Bar mapping: 0-4095 ADC to 0-10 Bar
        return (rawValue * PRESSURE_MAX_BAR) / ADC_MAX_VALUE;
    }
}