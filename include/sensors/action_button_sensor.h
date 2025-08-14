#pragma once

#include "hardware/gpio_pins.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_sensor.h"
#include "utilities/types.h"

/**
 * @class ActionButtonSensor
 * @brief Action button sensor for single button navigation
 *
 * @details This sensor monitors the state of the action button connected to GPIO 32,
 * providing boolean readings for button press detection. It follows the same
 * pattern as other sensors in the system for consistent GPIO access.
 *
 * @model_role Provides button state data to ActionManager for timing logic
 * @data_type Boolean (true=button pressed, false=button not pressed)
 * @hardware_pin GPIO 32 connected to 3.3V through push button
 *
 * @gpio_configuration:
 * - Pin: GPIO 32 (general purpose I/O pin on ESP32)
 * - Mode: INPUT_PULLDOWN (internal pull-down resistor enabled)
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
class ActionButtonSensor : public ISensor
{
  public:
    // Constructors and Destructors
    ActionButtonSensor(IGpioProvider *gpioProvider);

    // ISensor Interface Implementation
    void Init() override;
    Reading GetReading() override;

    /// @brief Get current button state directly (for ActionManager)
    /// @return true if button is currently pressed, false otherwise
    bool IsButtonPressed();

  private:
    IGpioProvider *gpioProvider_;
};