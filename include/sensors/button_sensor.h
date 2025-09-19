#pragma once

#include "hardware/gpio_pins.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_sensor.h"
#include "sensors/base_sensor.h"
#include "utilities/types.h"

/**
 * @class ButtonSensor
 * @brief Button sensor with integrated timing logic
 *
 * @details This sensor monitors the button (GPIO 32) and provides
 * timing-based button action detection. It internally handles debouncing
 * and differentiates between short and long presses.
 *
 * @timing_logic:
 * - Debounce: < 50ms (ignored)
 * - Short press: 50ms - 2000ms
 * - Long press: 2000ms - 5000ms
 * - Timeout: > 5000ms (ignored)
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
class ButtonSensor : public BaseSensor
{
  public:
    // ========== Constructors and Destructor ==========
    ButtonSensor(IGpioProvider *gpioProvider);

    // ========== Public Interface Methods ==========
    // ISensor Interface Implementation
    void Init() override;
    Reading GetReading() override;

    // BaseSensor interface
    bool HasStateChanged() override;

    // Action handling
    ButtonAction GetAndConsumeAction();

    // ========== Public Data Members ==========
    // Button timing constants
    static constexpr uint32_t DEBOUNCE_MS = 50;
    static constexpr uint32_t SHORT_PRESS_MAX_MS = 2000;
    static constexpr uint32_t LONG_PRESS_MAX_MS = 5000;

  protected:
    // ========== Protected Methods ==========
    void OnInterruptTriggered() override;

  private:
    // ========== Private Methods ==========
    bool ReadButtonState();
    void ProcessButtonState();
    ButtonAction DetermineAction(unsigned long duration);
    
    // ========== Private Data Members ==========
    IGpioProvider *gpioProvider_;
    
    // Button state tracking
    bool currentButtonState_ = false;
    bool previousButtonState_ = false;
    
    // Timing tracking
    unsigned long buttonPressStartTime_ = 0;
    unsigned long buttonPressDuration_ = 0;
    
    // Action detection
    ButtonAction detectedAction_ = ButtonAction::NONE;
    bool actionReady_ = false;
    bool longPressTriggeredDuringHold_ = false;
};