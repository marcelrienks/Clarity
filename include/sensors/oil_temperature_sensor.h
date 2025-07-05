#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_sensor.h"
#include "utilities/types.h"

#include <lvgl.h>
#include <LovyanGFX.hpp>
#include <esp_random.h>

/**
 * @class OilTemperatureSensor
 * @brief Oil temperature monitoring sensor with delta-based updates
 * 
 * @details This sensor monitors engine oil temperature levels and provides
 * intelligent caching and delta-based updates for performance optimization.
 * It implements time-based sampling and change detection to minimize
 * unnecessary UI updates. Updates at a slower rate than pressure for stability.
 * 
 * @model_role Provides oil temperature data to OemOilTemperatureComponent
 * @data_type int32_t (Degrees Fahrenheit or Celsius)
 * @range 80-200°F typical automotive range
 * @update_frequency 0.67 Hz (every 1500ms) - slower than pressure
 * 
 * @performance_features:
 * - Delta-based updates: Only reports when value changes
 * - Cached readings: Avoids redundant sensor polling
 * - Time-based sampling: Controlled update intervals
 * - Previous value tracking: Enables change detection
 * - Slower sampling: Temperature changes more gradually than pressure
 * 
 * @simulation_mode Currently uses ESP32 random number generator for testing
 * @hardware_interface Designed for thermistor or thermocouple input
 * @calibration Future: Temperature curve calibration for actual sensors
 * 
 * @critical_thresholds:
 * - Normal: 0-120°C
 * - Warning: 100-120°C
 * 
 * @special_considerations:
 * - Temperature mapping: Values are mapped for display in component
 * - Thermal lag: Temperature changes slowly, requiring fewer updates
 * - Overheating protection: Critical for engine safety
 * 
 * @context This sensor feeds the right-side oil temperature gauge.
 * It provides smart caching and delta updates with slower sampling rate.
 * Currently simulated but designed for real temperature sensor integration.
 */
class OilTemperatureSensor : public ISensor
{
public:
    OilTemperatureSensor();

    void init() override;
    Reading get_reading() override;
    
    // Delta-based update support
    bool has_value_changed() override;
    Reading get_cached_reading() override;

    // TODO: TEMP for testing
private:
    int32_t _current_reading = 0;
    int32_t _previous_reading = -1;
    unsigned long _last_update_time = 0;
    static constexpr unsigned long UPDATE_INTERVAL_MS = 1500; // Update every 1500ms (0.67Hz)
};