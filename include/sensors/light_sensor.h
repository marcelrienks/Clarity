#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_sensor.h"
#include "interfaces/i_gpio_provider.h"
#include "utilities/types.h"
#include "hardware/gpio_pins.h"

#include <random>

/**
 * @class LightSensor
 * @brief Vehicle lights status sensor for automotive monitoring
 * 
 * @details This sensor monitors the lights status of the vehicle,
 * providing boolean readings for light-related states. It can detect
 * headlights, running lights, or other automotive lighting systems.
 * 
 * @model_role Provides lights status data to trigger system and related components
 * @data_type Boolean (true=lights on, false=lights off)
 * @update_strategy Event-driven or polled based on implementation
 * 
 * @hardware_interface GPIO 33 with pull-down resistor
 * @debouncing Built-in debouncing for stable readings
 * 
 * @usage_context:
 * - Automatic theme switching (day/night mode)
 * - Vehicle lighting monitoring
 * - Power management optimization
 * - User interface brightness control
 * 
 * @context This sensor provides lights status information for the trigger system.
 * It enables automatic theme switching based on vehicle lighting state - when
 * lights are on (night driving), switch to night theme for better visibility.
 */
class LightSensor : public ISensor
{
public:
    // Constructors and Destructors
    LightSensor(IGpioProvider *gpioProvider);

    // Core Functionality Methods
    void Init() override;
    Reading GetReading() override;
    
    /// @brief Get current lights state directly (for triggers)
    /// @return true if lights are on, false if lights are off
    bool GetLightsState();

private:
    IGpioProvider *gpioProvider_;
};