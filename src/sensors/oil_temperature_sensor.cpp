#include "sensors/oil_temperature_sensor.h"

// Constructors and Destructors

/// @brief Constructor for OilTemperatureSensor
OilTemperatureSensor::OilTemperatureSensor()
{
    // Initialize with zero reading
    _current_reading = 0;
    _previous_reading = -1; // Ensure first update is detected as changed
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
    int32_t adc_value = analogRead(GpioPins::OIL_TEMPERATURE);
    OilTemperatureSensor::get_reading(); // Read initial temperature value
    log_i("Initial temperature reading: %d°C (ADC: %d)", _current_reading, adc_value);
}

/// @brief Get the current oil temperature reading with time-based sampling
/// @return Current temperature reading in Celsius
Reading OilTemperatureSensor::get_reading()
{
    unsigned long current_time = millis();
    
    // Only read new value every UPDATE_INTERVAL_MS milliseconds
    if (current_time - _last_update_time >= UPDATE_INTERVAL_MS) {
        _last_update_time = current_time;
        _previous_reading = _current_reading; // Store current before reading new
        
        // Read analog value from GPIO pin (0-4095 for 12-bit ADC)
        int32_t adc_value = analogRead(GpioPins::OIL_TEMPERATURE);
        
        // Convert ADC value to temperature using voltage divider calculation
        // For 22k potentiometer: Voltage = (ADC_value / 4095) * 3.3V
        // Temperature mapping: 0V = 0°C, 3.3V = 120°C
        int32_t new_value = (adc_value * TEMPERATURE_MAX_CELSIUS) / ADC_MAX_VALUE;
        
        log_v("ADC: %d, Temperature: %d°C", adc_value, new_value);
        
        // Only update if value actually changed (avoid redundant updates)
        if (new_value != _current_reading) {
            _current_reading = new_value;
            log_i("Temperature reading changed to %d°C", _current_reading);

        } else {
            log_d("Temperature reading unchanged: %d°C", new_value);
        }
    }
    
    return _current_reading;
}

/// @brief Check if the temperature value has changed since last reading
/// @return true if value changed, false otherwise
bool OilTemperatureSensor::has_value_changed()
{
    return _current_reading != _previous_reading;
}