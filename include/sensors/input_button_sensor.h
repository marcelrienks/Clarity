#pragma once

#include "interfaces/i_sensor.h"
#include "interfaces/i_gpio_provider.h"
#include "utilities/types.h"
#include "hardware/gpio_pins.h"

/**
 * @class InputButtonSensor
 * @brief Input button sensor for single button navigation
 * 
 * @details This sensor monitors the state of the input button connected to GPIO 34,
 * providing boolean readings for button press detection. It follows the same
 * pattern as other sensors in the system for consistent GPIO access.
 * 
 * @model_role Provides button state data to InputManager for timing logic
 * @data_type Boolean (true=button pressed, false=button not pressed)
 * @hardware_pin GPIO 34 connected to 3.3V through push button
 * 
 * @gpio_configuration:
 * - Pin: GPIO 34 (input-only pin on ESP32)
 * - Mode: INPUT (no internal pull-up/down needed, button provides 3.3V)
 * - Logic: HIGH when pressed, LOW when not pressed
 * 
 * @usage_context:
 * - Single button navigation system
 * - Menu navigation and selection
 * - Panel-specific input handling
 * 
 * @consistency Follows same pattern as KeySensor, LockSensor, etc.
 * @dependency_injection Uses IGpioProvider for hardware abstraction
 */
class InputButtonSensor : public ISensor
{
public:
    // Constructors and Destructors
    InputButtonSensor(IGpioProvider* gpioProvider);

    // ISensor Interface Implementation
    void Init() override;
    Reading GetReading() override;
    
    /// @brief Get current button state directly (for InputManager)
    /// @return true if button is currently pressed, false otherwise
    bool IsButtonPressed();

private:
    IGpioProvider* gpioProvider_;
};