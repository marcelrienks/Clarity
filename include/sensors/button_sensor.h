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
class ButtonSensor : public ISensor, public BaseSensor
{
  public:
    // Button timing constants
    static constexpr uint32_t DEBOUNCE_MS = 50;
    static constexpr uint32_t SHORT_PRESS_MAX_MS = 2000;
    static constexpr uint32_t LONG_PRESS_MAX_MS = 5000;

    // Constructors and Destructors
    ButtonSensor(IGpioProvider *gpioProvider);

    // ISensor Interface Implementation
    void Init() override;
    Reading GetReading() override;

    /// @brief Get the current button action
    /// @return ButtonAction indicating the type of press detected
    ButtonAction GetButtonAction();
    
    /// @brief Check if a button action is ready
    /// @return true if a valid button action has been detected
    bool HasButtonAction() const;
    
    /// @brief Clear the current button action (after processing)
    void ClearButtonAction();
    
    /// @brief Get current raw button state
    /// @return true if button is currently pressed, false otherwise
    bool IsButtonPressed();
    
    // BaseSensor interface
    bool HasStateChanged() override;

  protected:
    // Custom interrupt behavior
    void OnInterruptTriggered() override;

  private:
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
    
    /// @brief Read GPIO pin and determine button state
    /// @return Button state based on GPIO pin reading
    bool readButtonState();
    
    /// @brief Process button state changes and detect actions
    void processButtonState();
    
    /// @brief Determine button action based on press duration
    /// @param duration Press duration in milliseconds
    /// @return ButtonAction type based on duration
    ButtonAction determineAction(unsigned long duration);
};