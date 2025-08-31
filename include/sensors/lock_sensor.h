#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "hardware/gpio_pins.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_sensor.h"
#include "sensors/base_sensor.h"
#include "utilities/types.h"

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
 * @hardware_interface GPIO 27 with pull-down resistor
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
class LockSensor : public ISensor, public BaseSensor
{
  public:
    // Constructors and Destructors
    LockSensor(IGpioProvider *gpioProvider);

    // Core Functionality Methods
    void Init() override;
    Reading GetReading() override;
    
    // BaseSensor interface
    bool HasStateChanged() override;
    
    // Simplified interrupt system
    const char* GetTriggerInterruptId() const;

  protected:
    // Custom interrupt behavior
    void OnInterruptTriggered() override;

  private:
    IGpioProvider *gpioProvider_;
    bool previousLockState_ = false;
    const char* triggerInterruptId_ = nullptr;  // ID of interrupt to trigger on state change
    
    /// @brief Read GPIO pin and determine lock state
    /// @return Lock state based on GPIO pin reading
    bool readLockState();
};