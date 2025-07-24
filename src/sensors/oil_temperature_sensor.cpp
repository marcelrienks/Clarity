#include "sensors/oil_temperature_sensor.h"

// Constructors and Destructors

/// @brief Constructor for OilTemperatureSensor
OilTemperatureSensor::OilTemperatureSensor()
{
    // Initialize with zero reading
    currentReading_ = 0;
    previousReading_ = -1; // Ensure first update is detected as changed
}

// Core Functionality Methods

/// @brief Initialize the oil temperature sensor hardware
void OilTemperatureSensor::init()
{
    // Configure GPIO pin for analog input
    log_d("...");
    
    // Configure ADC resolution and attenuation for direct 3.3V operation
    analogReadResolution(12); // 12-bit resolution (0-4095)
    analogSetAttenuation(ADC_11db); // 0-3.3V range
    
    // Take initial reading
    int32_t adcValue = analogRead(gpio_pins::OIL_TEMPERATURE);
    OilTemperatureSensor::GetReading(); // Read initial temperature value
    log_i("Initial temperature reading: %d°C (ADC: %d)", currentReading_, adcValue);
}

/// @brief Get the current oil temperature reading with time-based sampling
/// @return Current temperature reading in Celsius
Reading OilTemperatureSensor::GetReading()
{
    unsigned long currentTime = millis();
    
    // Only read new value every UPDATE_INTERVAL_MS milliseconds
    if (currentTime - lastUpdateTime_ >= UPDATE_INTERVAL_MS) {
        lastUpdateTime_ = currentTime;
        previousReading_ = currentReading_; // Store current before reading new
        
        // Read analog value from GPIO pin (0-4095 for 12-bit ADC)
        int32_t adcValue = analogRead(gpio_pins::OIL_TEMPERATURE);
        
        // Convert ADC value to temperature using voltage divider calculation
        // For 22k potentiometer: Voltage = (ADC_value / 4095) * 3.3V
        // Temperature mapping: 0V = 0°C, 3.3V = 120°C
        int32_t newValue = (adcValue * TEMPERATURE_MAX_CELSIUS) / ADC_MAX_VALUE;
        
        log_v("ADC: %d, Temperature: %d°C", adcValue, newValue);
        
        // Only update if value actually changed (avoid redundant updates)
        if (newValue != currentReading_) {
            currentReading_ = newValue;
            log_i("Temperature reading changed to %d°C", currentReading_);

        } else {
            log_d("Temperature reading unchanged: %d°C", newValue);
        }
    }
    
    return currentReading_;
}

