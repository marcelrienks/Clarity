#pragma once

#include "hardware/gpio_pins.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_sensor.h"
#include "sensors/base_sensor.h"
#include "utilities/types.h"

/**
 * @class KeyNotPresentSensor
 * @brief Independent sensor for key not present detection (GPIO 26)
 *
 * @details This sensor monitors only the key not present state (GPIO 26) as part of
 * the split sensor architecture. Separating key present and key not present 
 * sensors prevents initialization race conditions and enables proper resource
 * ownership by handlers.
 *
 * @architecture_requirement Part of split sensor design to prevent race conditions
 * @gpio_pin GPIO 26 with pull-down resistor
 * @ownership Created and owned by TriggerHandler
 * @change_detection Uses BaseSensor DetectChange template for consistency
 *
 * @usage_context:
 * - Automatic panel restoration when key is removed
 * - Vehicle security state monitoring
 * - Power management triggers
 *
 * @split_sensor_design This sensor handles only key not present detection.
 * KeyPresentSensor handles the complementary key present state.
 * This separation prevents GPIO resource conflicts and initialization races.
 */
class KeyNotPresentSensor : public BaseSensor
{
public:
    // Constructors and Destructors
    KeyNotPresentSensor(IGpioProvider* gpioProvider);
    ~KeyNotPresentSensor();

    // Core Functionality Methods
    void Init() override;
    Reading GetReading() override;

    /// @brief Get current key not present state directly
    /// @return true if key is not present (GPIO 26 HIGH), false otherwise
    bool GetKeyNotPresentState();
    
    // BaseSensor interface
    bool HasStateChanged() override;

protected:
    // Custom interrupt behavior
    void OnInterruptTriggered() override;

private:
    IGpioProvider* gpioProvider_;
    bool previousState_ = false;
    
    /// @brief Read GPIO 26 pin state
    /// @return true if key not present pin is HIGH, false if LOW
    bool readKeyNotPresentState();
};