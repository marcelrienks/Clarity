#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_sensor.h"
#include "utilities/types.h"
#include "hardware/gpio_pins.h"

#include <lvgl.h>
#include <LovyanGFX.hpp>
#include <random>

/**
 * @class LockSensor
 * @brief Lock status sensor for vehicle monitoring
 * 
 * @details This sensor monitors the lock status of the vehicle,
 * providing boolean readings for lock-related states. It simulates lock
 * detection for testing purposes.
 * 
 * @model_role Provides lock status data to LockWidget and related systems
 * @data_type Boolean (true=lock engaged/on, false=lock disengaged/off)
 * @update_strategy Event-driven or polled based on implementation
 * 
 * @simulation_mode Currently uses simulated data for testing
 * @hardware_interface Designed for digital input pin monitoring
 * @debouncing Built-in debouncing for stable readings
 * 
 * @usage_context:
 * - Vehicle security monitoring
 * - Lock state management
 * - User interface status indication
 * 
 * @context This sensor provides lock status information. It's part of
 * the vehicle monitoring system and feeds data to LockWidget for display.
 * Currently implemented with simulated data for testing.
 */
class LockSensor : public ISensor
{
public:
    // Constructors and Destructors
    LockSensor();

    // Core Functionality Methods
    void init() override;
    Reading GetReading() override;
};