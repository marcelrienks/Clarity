#pragma once

#include "hardware/gpio_pins.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_sensor.h"
#include "sensors/base_sensor.h"
#include "utilities/types.h"

/**
 * @class KeyPresentSensor
 * @brief Independent sensor for key present detection (GPIO 25)
 *
 * @details This sensor monitors only the key present state (GPIO 25) as part of
 * the split sensor architecture. Separating key present and key not present 
 * sensors prevents initialization race conditions and enables proper resource
 * ownership by handlers.
 *
 * @architecture_requirement Part of split sensor design to prevent race conditions
 * @gpio_pin GPIO 25 with pull-down resistor
 * @ownership Created and owned by TriggerHandler
 * @change_detection Uses BaseSensor DetectChange template for consistency
 *
 * @usage_context:
 * - Automatic key panel loading when key is detected
 * - Vehicle security monitoring
 * - Power state management triggers
 *
 * @split_sensor_design This sensor handles only key present detection.
 * KeyNotPresentSensor handles the complementary key not present state.
 * This separation prevents GPIO resource conflicts and initialization races.
 */
class KeyPresentSensor : public ISensor, public BaseSensor
{
public:
    // Constructors and Destructors
    KeyPresentSensor(IGpioProvider* gpioProvider);
    ~KeyPresentSensor();

    // Core Functionality Methods
    void Init() override;
    Reading GetReading() override;

    /// @brief Get current key present state directly
    /// @return true if key is present (GPIO 25 HIGH), false otherwise
    bool GetKeyPresentState();
    
    // BaseSensor interface
    bool HasStateChanged() override;

protected:
    // Custom interrupt behavior
    void OnInterruptTriggered() override;

private:
    IGpioProvider* gpioProvider_;
    bool previousState_ = false;
    
    /// @brief Read GPIO 25 pin state
    /// @return true if key present pin is HIGH, false if LOW
    bool readKeyPresentState();
};