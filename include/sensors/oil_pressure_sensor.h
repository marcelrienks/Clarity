#pragma once // preventing duplicate definitions, alternative to the traditional include guards

// System/Library Includes
#include <LovyanGFX.hpp>
#include <lvgl.h>

// Project Includes
#include "interfaces/i_sensor.h"
#include "utilities/types.h"
#include "hardware/gpio_pins.h"

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
 * @data_type int32_t (Bar - metric pressure unit)
 * @range 0-10 Bar typical automotive range
 * @update_frequency 1 Hz (every 1000ms)
 * 
 * @performance_features:
 * - Delta-based updates: Only reports when value changes
 * - Cached readings: Avoids redundant sensor polling
 * - Time-based sampling: Controlled update intervals
 * - Previous value tracking: Enables change detection
 * 
 * @hardware_interface 3.3V analog pressure sensor input via GPIO pin
 * @calibration 22k potentiometer: 0V = 0 Bar, 3.3V = 10 Bar
 * 
 * @critical_thresholds:
 * - Normal: 2-10 Bar
 * - Warning: 0-2 Bar
 * 
 * @context This sensor feeds the left-side oil pressure gauge.
 * It provides smart caching and delta updates for smooth performance.
 * Reads 3.3V analog input directly with linear voltage-to-pressure mapping.
 */
class OilPressureSensor : public ISensor
{
public:
    // Constructors and Destructors
    OilPressureSensor();

    // Core Functionality Methods
    void init() override;
    Reading get_reading() override;

private:
    // Private Data Members
    int32_t currentReading_ = 0;
    int32_t previousReading_ = -1;
    unsigned long _last_update_time = 0;
    static constexpr unsigned long UPDATE_INTERVAL_MS = 1000; // Update every 1000ms (1Hz)
    
    // ADC and potentiometer calibration constants
    static constexpr int32_t ADC_MAX_VALUE = 4095; // 12-bit ADC
    static constexpr int32_t PRESSURE_MAX_BAR = 10; // Maximum pressure reading in Bar
    static constexpr int32_t POTENTIOMETER_RESISTANCE = 22000; // 22k ohm potentiometer
    static constexpr float SUPPLY_VOLTAGE = 3.3; // ESP32 3.3V supply
};