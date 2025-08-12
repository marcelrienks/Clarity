#include "sensors/oil_pressure_sensor.h"
#include "managers/error_manager.h"
#include "utilities/unit_converter.h"
#include <Arduino.h>
#include <esp32-hal-log.h>

// Constructors and Destructors

/// @brief Constructor for OilPressureSensor
OilPressureSensor::OilPressureSensor(IGpioProvider* gpioProvider, int updateRateMs) : gpioProvider_(gpioProvider), updateIntervalMs_(updateRateMs)
{
    // Initialize with zero reading
    currentReading_ = 0;
    previousReading_ = -1; // Ensure first update is detected as changed
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
    
    // Take initial reading
    int32_t adcValue = gpioProvider_->AnalogRead(gpio_pins::OIL_PRESSURE);
    OilPressureSensor::GetReading(); // Read initial pressure value
}

/// @brief Get the current oil pressure reading with time-based sampling
/// @return Current pressure reading in Bar
Reading OilPressureSensor::GetReading()
{
    unsigned long currentTime = millis();
    
    // Only read new value every updateIntervalMs_ milliseconds (from preferences)
    if (currentTime - lastUpdateTime_ >= updateIntervalMs_) {
        lastUpdateTime_ = currentTime;
        previousReading_ = currentReading_; // Store current before reading new
        
        // Read analog value from GPIO pin (0-4095 for 12-bit ADC)
        int32_t adcValue = gpioProvider_->AnalogRead(gpio_pins::OIL_PRESSURE);
        
        // Check for invalid ADC readings
        if (adcValue < 0 || adcValue > ADC_MAX_VALUE) {
            ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "OilPressureSensor", 
                "ADC reading out of range: " + std::to_string(adcValue));
            return std::monostate{}; // Return invalid reading
        }
        
        // Convert ADC value directly to configured pressure unit
        // For 22k potentiometer: Voltage = (ADC_value / 4095) * 3.3V
        // Pressure mapping based on configured unit preference
        int32_t newValue;
        if (preferenceService_) {
            const Configs& config = preferenceService_->GetConfig();
            if (config.pressureUnit == "PSI") {
                // Map 0-4095 ADC to 0-145 PSI (0-10 Bar equivalent)
                newValue = (adcValue * 145) / ADC_MAX_VALUE;
            } else if (config.pressureUnit == "kPa") {
                // Map 0-4095 ADC to 0-1000 kPa (0-10 Bar equivalent)
                newValue = (adcValue * 1000) / ADC_MAX_VALUE;
            } else {
                // Default Bar mapping: 0-4095 ADC to 0-10 Bar
                newValue = (adcValue * PRESSURE_MAX_BAR) / ADC_MAX_VALUE;
            }
        } else {
            // Fallback to Bar if no preference service
            newValue = (adcValue * PRESSURE_MAX_BAR) / ADC_MAX_VALUE;
        }
        
        // Only update if value actually changed (avoid redundant updates)
        if (newValue != currentReading_) {
            currentReading_ = newValue;
            // Log with appropriate unit
            const char* unit = "Bar"; // Default
            if (preferenceService_) {
                const Configs& config = preferenceService_->GetConfig();
                unit = config.pressureUnit.c_str();
            }
            log_i("Pressure reading changed to %d %s (ADC: %d)", currentReading_, unit, adcValue);

        }
    }
    
    return currentReading_;
}


