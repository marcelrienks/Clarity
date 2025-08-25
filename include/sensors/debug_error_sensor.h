#pragma once

#include "hardware/gpio_pins.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_sensor.h"
#include "sensors/base_sensor.h"
#include "utilities/types.h"

/**
 * @class DebugErrorSensor
 * @brief Debug-only sensor for triggering test errors during development
 *
 * @details This sensor monitors GPIO pin 34 and triggers test errors when
 * the pin transitions from LOW to HIGH. It generates three test errors
 * (WARNING, ERROR, CRITICAL) for debugging the error handling system.
 * This sensor only responds to rising edges (LOW->HIGH) and ignores
 * falling edges (HIGH->LOW).
 *
 * @model_role Provides debug error triggering capability for development
 * @data_type Boolean (true=trigger errors, false=no action)
 * @update_strategy Edge-triggered on rising edge only
 *
 * @debug_only This sensor is for development/debugging purposes only
 * @hardware_interface GPIO pin 34 (input-only, no internal pull resistors)
 * @edge_detection Rising edge only (LOW to HIGH transition)
 *
 * @usage_context:
 * - Development testing of error handling system
 * - Manual triggering of error panel for UI testing
 * - Debugging error workflow and navigation
 *
 * @context This sensor is used exclusively for debugging. It allows
 * developers to manually trigger errors using DIP switch 8 to test
 * the error handling system without actual system errors.
 */
class DebugErrorSensor : public ISensor, public BaseSensor
{
  public:
    // Constructors and Destructors
    DebugErrorSensor(IGpioProvider *gpioProvider);

    // Core Functionality Methods
    void Init() override;
    Reading GetReading() override;
    
    // BaseSensor interface
    bool HasStateChanged() override;

  protected:
    // Custom interrupt behavior
    void OnInterruptTriggered() override;

  private:
    IGpioProvider *gpioProvider_;
    bool previousState_;        // Track previous pin state for edge detection
    bool initialized_;          // Flag to track initialization
    unsigned long startupTime_; // Time when sensor was initialized (for grace period)
    
    /// @brief Read GPIO pin state
    /// @return Current pin state
    bool readPinState();
};