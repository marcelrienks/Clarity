#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_sensor.h"
#include "utilities/types.h"

#include <lvgl.h>
#include <LovyanGFX.hpp>
#include <esp_random.h>

/**
 * @class OilPressureSensor
 * @brief Oil pressure monitoring sensor with delta-based updates
 * 
 * @details This sensor monitors engine oil pressure levels and provides
 * intelligent caching and delta-based updates for performance optimization.
 * It implements time-based sampling and change detection to minimize
 * unnecessary UI updates.
 * 
 * @model_role Provides oil pressure data to OemOilPressureComponent
 * @data_type int32_t (PSI - Pounds per Square Inch)
 * @range 0-60 PSI typical automotive range
 * @update_frequency 1 Hz (every 1000ms)
 * 
 * @performance_features:
 * - Delta-based updates: Only reports when value changes
 * - Cached readings: Avoids redundant sensor polling
 * - Time-based sampling: Controlled update intervals
 * - Previous value tracking: Enables change detection
 * 
 * @simulation_mode Currently uses ESP32 random number generator for testing
 * @hardware_interface Designed for analog pressure sensor input
 * @calibration Future: ADC calibration for actual pressure sensors
 * 
 * @critical_thresholds:
 * - Normal: 5-60 PSI
 * - Warning: 0-5 PSI
 * 
 * @context This sensor feeds the left-side oil pressure gauge.
 * It provides smart caching and delta updates for smooth performance.
 * Currently simulated but designed for real pressure sensor integration.
 */
class OilPressureSensor : public ISensor
{
public:
    OilPressureSensor();

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
    static constexpr unsigned long UPDATE_INTERVAL_MS = 1000; // Update every 1000ms (1Hz)
};