#include "sensors/oil_temperature_sensor.h"
#include "utilities/unit_converter.h"
#include <Arduino.h>
#include <esp32-hal-log.h>

// Constructors and Destructors

/// @brief Constructor for OilTemperatureSensor
OilTemperatureSensor::OilTemperatureSensor(IGpioProvider* gpioProvider, int updateRateMs) : gpioProvider_(gpioProvider), updateIntervalMs_(updateRateMs)
{
    // Initialize with zero reading
    currentReading_ = 0;
    previousReading_ = -1; // Ensure first update is detected as changed
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
    
    // Take initial reading
    int32_t adcValue = gpioProvider_->AnalogRead(gpio_pins::OIL_TEMPERATURE);
    OilTemperatureSensor::GetReading(); // Read initial temperature value
}

/// @brief Get the current oil temperature reading with time-based sampling
/// @return Current temperature reading in Celsius
Reading OilTemperatureSensor::GetReading()
{
    unsigned long currentTime = millis();
    
    // Only read new value every updateIntervalMs_ milliseconds (from preferences)
    if (currentTime - lastUpdateTime_ >= updateIntervalMs_) {
        lastUpdateTime_ = currentTime;
        previousReading_ = currentReading_; // Store current before reading new
        
        // Read analog value from GPIO pin (0-4095 for 12-bit ADC)
        int32_t adcValue = gpioProvider_->AnalogRead(gpio_pins::OIL_TEMPERATURE);
        
        // Convert ADC value directly to configured temperature unit
        // For 22k potentiometer: Voltage = (ADC_value / 4095) * 3.3V
        // Temperature mapping based on configured unit preference
        int32_t newValue;
        if (preferenceService_) {
            const Configs& config = preferenceService_->GetConfig();
            if (config.tempUnit == "F") {
                // Map 0-4095 ADC to 32-248°F (0-120°C equivalent)
                // Formula: F = C * 9/5 + 32, so 0°C=32°F, 120°C=248°F
                int32_t celsiusValue = (adcValue * TEMPERATURE_MAX_CELSIUS) / ADC_MAX_VALUE;
                newValue = (celsiusValue * 9) / 5 + 32;
            } else {
                // Default Celsius mapping: 0-4095 ADC to 0-120°C
                newValue = (adcValue * TEMPERATURE_MAX_CELSIUS) / ADC_MAX_VALUE;
            }
        } else {
            // Fallback to Celsius if no preference service
            newValue = (adcValue * TEMPERATURE_MAX_CELSIUS) / ADC_MAX_VALUE;
        }
        
        // Only update if value actually changed (avoid redundant updates)
        if (newValue != currentReading_) {
            currentReading_ = newValue;
            // Log with appropriate unit
            const char* unit = "°C"; // Default
            if (preferenceService_) {
                const Configs& config = preferenceService_->GetConfig();
                unit = (config.tempUnit == "F") ? "°F" : "°C";
            }
            log_i("Temperature reading changed to %d%s (ADC: %d)", currentReading_, unit, adcValue);

        }
    }
    
    return currentReading_;
}


