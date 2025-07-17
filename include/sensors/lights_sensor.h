#pragma once

// System/Library Includes
#include <esp32-hal-log.h>

// Project Includes
#include "interfaces/i_sensor.h"
#include "hardware/gpio_pins.h"
#include "utilities/types.h"

/**
 * @class LightsSensor
 * @brief Sensor for monitoring lights switch state on GPIO 32
 * 
 * @details This sensor monitors the lights switch (DIP switch 4) to determine
 * whether the vehicle lights are on/off and trigger theme changes accordingly.
 * The sensor reads GPIO 32 which is connected to DIP switch 4 with pull-down configuration.
 * 
 * @gpio_logic:
 * - GPIO 32 LOW (switch OFF): Lights off - Day theme
 * - GPIO 32 HIGH (switch ON): Lights on - Night theme
 * 
 * @hardware_connection:
 * - GPIO 32 connected to DIP switch 4a (signal)
 * - DIP switch 4b connected to 3V3 (power)
 * - INPUT_PULLDOWN configuration ensures LOW when switch is OFF
 * 
 * @return_values:
 * - true: Lights on - Night theme requested (GPIO HIGH)
 * - false: Lights off - Day theme requested (GPIO LOW)
 * 
 * @use_cases:
 * - Automatic theme switching based on vehicle lights state
 * - Integration with LightsTrigger for system-wide theme changes
 * - Manual theme control via physical switch
 * 
 * @context: This sensor provides the input for the LightsTrigger which
 * manages system-wide theme switching via the StyleManager.
 */
class LightsSensor : public ISensor
{
public:
    // Constructors and Destructors
    LightsSensor();

    // Core Functionality Methods
    void init() override;
    Reading get_reading() override;

private:
    // Private Data Members
    static constexpr const char* SENSOR_TAG = "LightsSensor"; ///< Logging tag
};