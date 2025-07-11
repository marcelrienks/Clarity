#pragma once // preventing duplicate definitions, alternative to the traditional include guards

// System/Library Includes
#include <LovyanGFX.hpp>
#include <lvgl.h>

// Project Includes
#include "interfaces/i_sensor.h"
#include "utilities/types.h"
#include "hardware/gpio_pins.h"

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
 * @data_type int32_t (Degrees Celsius)
 * @range 0-120°C typical automotive range
 * @update_frequency 0.67 Hz (every 1500ms) - slower than pressure
 * 
 * @performance_features:
 * - Delta-based updates: Only reports when value changes
 * - Cached readings: Avoids redundant sensor polling
 * - Time-based sampling: Controlled update intervals
 * - Previous value tracking: Enables change detection
 * - Slower sampling: Temperature changes more gradually than pressure
 * 
 * @hardware_interface 3.3V analog temperature sensor input via GPIO pin
 * @calibration Linear mapping: 0V = 0°C, 3.3V = 120°C
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
 * Reads 3.3V analog input directly with linear voltage-to-temperature mapping.
 */
class OilTemperatureSensor : public ISensor
{
public:
    // Constructors and Destructors
    OilTemperatureSensor();

    // Core Functionality Methods
    void init() override;
    Reading get_reading() override;
    
    // Delta-based update support
    bool has_value_changed();

private:
    // Private Data Members
    int32_t _current_reading = 0;
    int32_t _previous_reading = -1;
    unsigned long _last_update_time = 0;
    static constexpr unsigned long UPDATE_INTERVAL_MS = 1000; // Update every 1000ms (1Hz)
    
    // ADC and sensor calibration constants
    static constexpr int32_t ADC_MAX_VALUE = 4095; // 12-bit ADC
    static constexpr int32_t TEMPERATURE_MAX_CELSIUS = 120; // Maximum temperature reading in Celsius
};