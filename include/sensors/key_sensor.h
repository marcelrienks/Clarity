#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_sensor.h"
#include "interfaces/i_gpio_provider.h"
#include "utilities/types.h"
#include "hardware/gpio_pins.h"

#include <random>

/**
 * @class KeySensor
 * @brief Key/ignition status sensor for vehicle monitoring
 * 
 * @details This sensor monitors the key or ignition status of the vehicle,
 * providing boolean readings for key-related states. It simulates key
 * detection for testing purposes.
 * 
 * @model_role Provides key status data to KeyWidget and related systems
 * @data_type Boolean (true=key present/on, false=key absent/off)
 * @update_strategy Event-driven or polled based on implementation
 * 
 * @simulation_mode Currently uses simulated data for testing
 * @hardware_interface Designed for digital input pin monitoring
 * @debouncing Built-in debouncing for stable readings
 * 
 * @usage_context:
 * - Vehicle security monitoring
 * - Power state management
 * - User interface status indication
 * 
 * @context This sensor provides key status information. It's part of
 * the vehicle monitoring system and feeds data to KeyWidget for display.
 * Currently implemented with simulated data for testing.
 */
class KeySensor : public ISensor
{
public:
    // Constructors and Destructors
    KeySensor(IGpioProvider *gpioProvider);

    // Core Functionality Methods
    void init() override;
    Reading getReading() override;
    
    /// @brief Get current key state directly (for panels)
    /// @return Current KeyState based on GPIO readings
    KeyState getKeyState();

private:
    IGpioProvider *gpioProvider_;
    
    /// @brief Read GPIO pins and determine key state
    /// @return KeyState based on GPIO pin readings
    KeyState readKeyState();
};