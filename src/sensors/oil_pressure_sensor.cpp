#include "sensors/oil_pressure_sensor.h"

OilPressureSensor::OilPressureSensor()
{
    // Generate initial value so needles render on first load
    _current_reading = esp_random() % 61;
    _previous_reading = -1; // Ensure first update is detected as changed
}

void OilPressureSensor::init()
{
    // Not needed but required to satisfy interface
}

// TODO: TEMP code for testing  
Reading OilPressureSensor::get_reading()
{
    unsigned long current_time = millis();
    
    // Only generate new value every UPDATE_INTERVAL_MS milliseconds
    if (current_time - _last_update_time >= UPDATE_INTERVAL_MS) {
        _last_update_time = current_time;
        _previous_reading = _current_reading; // Store current before generating new
        
        // Use lightweight ESP32 random number generator (0-60 PSI range)
        int32_t new_value = esp_random() % 61;
        
        // Only update if value actually changed (avoid redundant updates)
        if (new_value != _current_reading) {
            _current_reading = new_value;
            log_i("current_reading changed to %d", _current_reading);
        } else {
            log_d("Generated same value %d, keeping previous", new_value);
        }
    }
    
    return _current_reading;
}

bool OilPressureSensor::has_value_changed()
{
    return _current_reading != _previous_reading;
}