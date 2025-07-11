#include "sensors/oil_pressure_sensor.h"

// Constructors and Destructors

/// @brief Constructor for OilPressureSensor
OilPressureSensor::OilPressureSensor()
{
    // Initialize with zero reading
    _current_reading = 0;
    _previous_reading = -1; // Ensure first update is detected as changed
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
    int32_t adc_value = analogRead(GpioPins::OIL_PRESSURE);
    OilPressureSensor::get_reading(); // Read initial pressure value

    log_i("Initial pressure reading: %d Bar (ADC: %d)", _current_reading, adc_value);
}

/// @brief Get the current oil pressure reading with time-based sampling
/// @return Current pressure reading in Bar
Reading OilPressureSensor::get_reading()
{
    unsigned long current_time = millis();
    
    // Only read new value every UPDATE_INTERVAL_MS milliseconds
    if (current_time - _last_update_time >= UPDATE_INTERVAL_MS) {
        _last_update_time = current_time;
        _previous_reading = _current_reading; // Store current before reading new
        
        // Read analog value from GPIO pin (0-4095 for 12-bit ADC)
        int32_t adc_value = analogRead(GpioPins::OIL_PRESSURE);
        
        // Convert ADC value to pressure (1-10 Bar range)
        int32_t new_value = 1 + (adc_value * (PRESSURE_MAX_BAR - 1)) / ADC_MAX_VALUE;
        
        log_v("ADC: %d, Pressure: %d Bar", adc_value, new_value);
        
        // Only update if value actually changed (avoid redundant updates)
        if (new_value != _current_reading) {
            _current_reading = new_value;
            log_i("Pressure reading changed to %d Bar", _current_reading);

        } else {
            log_d("Pressure reading unchanged: %d Bar", new_value);
        }
    }
    
    return _current_reading;
}

/// @brief Check if the pressure value has changed since last reading
/// @return true if value changed, false otherwise
bool OilPressureSensor::has_value_changed()
{
    return _current_reading != _previous_reading;
}