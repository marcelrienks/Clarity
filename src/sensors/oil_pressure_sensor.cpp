#include "sensors/oil_pressure_sensor.h"

// Constructors and Destructors

/// @brief Constructor for OilPressureSensor
OilPressureSensor::OilPressureSensor()
{
    // Initialize with zero reading
    currentReading_ = 0;
    previousReading_ = -1; // Ensure first update is detected as changed
}

// Core Functionality Methods

/// @brief Initialize the oil pressure sensor hardware
void OilPressureSensor::init()
{
    // Configure GPIO pin for analog input
    log_d("...");
    
    // Configure ADC resolution and attenuation for direct 3.3V operation
    analogReadResolution(12); // 12-bit resolution (0-4095)
    analogSetAttenuation(ADC_11db); // 0-3.3V range
    
    // Take initial reading
    int32_t adcValue = analogRead(gpio_pins::OIL_PRESSURE);
    OilPressureSensor::get_reading(); // Read initial pressure value

    log_i("Initial pressure reading: %d Bar (ADC: %d)", currentReading_, adcValue);
}

/// @brief Get the current oil pressure reading with time-based sampling
/// @return Current pressure reading in Bar
Reading OilPressureSensor::get_reading()
{
    unsigned long current_time = millis();
    
    // Only read new value every UPDATE_INTERVAL_MS milliseconds
    if (current_time - _last_update_time >= UPDATE_INTERVAL_MS) {
        _last_update_time = current_time;
        previousReading_ = currentReading_; // Store current before reading new
        
        // Read analog value from GPIO pin (0-4095 for 12-bit ADC)
        int32_t adcValue = analogRead(gpio_pins::OIL_PRESSURE);
        
        // Convert ADC value to pressure using voltage divider calculation
        // For 22k potentiometer: Voltage = (ADC_value / 4095) * 3.3V
        // Pressure mapping: 0V = 0 Bar, 3.3V = 10 Bar
        int32_t newValue = (adcValue * PRESSURE_MAX_BAR) / ADC_MAX_VALUE;
        
        log_v("ADC: %d, Pressure: %d Bar", adcValue, newValue);
        
        // Only update if value actually changed (avoid redundant updates)
        if (newValue != currentReading_) {
            currentReading_ = newValue;
            log_i("Pressure reading changed to %d Bar", currentReading_);

        } else {
            log_d("Pressure reading unchanged: %d Bar", newValue);
        }
    }
    
    return currentReading_;
}

