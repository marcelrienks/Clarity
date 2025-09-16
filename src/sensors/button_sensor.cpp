#include "sensors/button_sensor.h"
#include "utilities/logging.h"
#include <Arduino.h>

#include "esp32-hal-log.h"

// Constructors and Destructors

/**
 * @brief Constructs a ButtonSensor instance with GPIO provider dependency
 * @param gpioProvider Pointer to GPIO provider interface for hardware abstraction
 *
 * Initializes the button sensor with the provided GPIO provider. The sensor
 * will use this provider to read button state from the configured GPIO pin.
 * This follows the dependency injection pattern for hardware abstraction.
 */
ButtonSensor::ButtonSensor(IGpioProvider *gpioProvider) : gpioProvider_(gpioProvider)
{
    log_v("ButtonSensor() constructor called");
}

// ISensor Interface Implementation

/**
 * @brief Initializes the button sensor hardware configuration
 *
 * Configures the GPIO pin for button input with pulldown resistor enabled.
 * Reads and logs the initial button state to verify proper hardware connection.
 * This is part of the ISensor interface implementation for sensor lifecycle management.
 */
void ButtonSensor::Init()
{
    log_v("Init() called");
    gpioProvider_->PinMode(gpio_pins::INPUT_BUTTON, INPUT_PULLDOWN);

    bool initialState = gpioProvider_->DigitalRead(gpio_pins::INPUT_BUTTON);
    log_i("GPIO %d initial state: %s", gpio_pins::INPUT_BUTTON,
          initialState ? "HIGH (pressed)" : "LOW (released)");

    log_i("ButtonSensor initialization completed on GPIO %d", gpio_pins::INPUT_BUTTON);
}

/**
 * @brief Get the current button state as a sensor reading
 * @return Reading containing the current button state (bool)
 */
Reading ButtonSensor::GetReading()
{
    // Process button state to detect actions
    ProcessButtonState();
    
    // Return the current button state as boolean for ActionHandler compatibility
    bool state = ReadButtonState();
    log_v("ButtonSensor::GetReading() returning %s", state ? "true" : "false");
    return state;
}

/**
 * @brief Get the current button action
 * @return ButtonAction indicating the type of press detected
 */
ButtonAction ButtonSensor::GetButtonAction()
{
    ProcessButtonState();
    return detectedAction_;
}

/**
 * @brief Check if a button action is ready
 * @return true if a valid button action has been detected
 */
bool ButtonSensor::HasButtonAction() const
{
    return actionReady_;
}

/**
 * @brief Clear the current button action (after processing)
 */
void ButtonSensor::ClearButtonAction()
{
    detectedAction_ = ButtonAction::NONE;
    actionReady_ = false;
}

/**
 * @brief Check if the action button is currently pressed
 * @return true if button is pressed (GPIO HIGH), false otherwise
 */
bool ButtonSensor::IsButtonPressed()
{
    // GPIO 32 reads HIGH when button is pressed (connected to 3.3V)
    return gpioProvider_->DigitalRead(gpio_pins::INPUT_BUTTON);
}

/**
 * @brief Reads the raw button state from GPIO with safety checks
 * @return true if button is pressed (GPIO HIGH), false otherwise
 *
 * Performs a direct GPIO read with null pointer safety check. Includes
 * periodic debug logging to verify continuous GPIO functionality.
 * The button reads HIGH when pressed due to pulldown resistor configuration.
 */
bool ButtonSensor::ReadButtonState()
{
    if (!gpioProvider_) {
        log_e("ButtonSensor::ReadButtonState() - gpioProvider_ is null!");
        return false;
    }

    bool state = gpioProvider_->DigitalRead(gpio_pins::INPUT_BUTTON);

    static unsigned long lastDebugTime = 0;
    unsigned long currentTime = millis();

    // Log every 2 seconds to verify GPIO is being read
    if (currentTime - lastDebugTime > 2000) {
        log_i("ButtonSensor GPIO %d continuous read = %s", gpio_pins::INPUT_BUTTON, state ? "HIGH" : "LOW");
        lastDebugTime = currentTime;
    }

    log_v("ButtonSensor::ReadButtonState() GPIO %d = %s", gpio_pins::INPUT_BUTTON, state ? "HIGH" : "LOW");
    return state;
}

/**
 * @brief Checks if button state has changed and action is ready
 * @return true if a button action is ready for processing
 *
 * Processes current button state and determines if any interrupt should be triggered.
 * Clears the trigger interrupt ID when no action is pending. This method is part
 * of the ISensor interface for state change detection in the interrupt system.
 */
bool ButtonSensor::HasStateChanged()
{
    ProcessButtonState();

    // If no action is ready, clear the interrupt ID
    if (!actionReady_)
    {
        triggerInterruptId_ = nullptr;
    }

    return actionReady_;
}

/**
 * @brief Gets the interrupt ID that should be triggered for current button action
 * @return Pointer to interrupt ID string, or nullptr if no action pending
 *
 * Returns the appropriate interrupt identifier ("short_press" or "long_press")
 * based on the detected button action. Used by the interrupt manager to
 * trigger the correct system response for button events.
 */
const char* ButtonSensor::GetTriggerInterruptId() const
{
    return triggerInterruptId_;
}

/**
 * @brief Processes button state changes and detects press actions
 *
 * Implements a state machine that tracks button press/release events and
 * determines the type of action based on press duration. Handles press start
 * detection, press end with duration calculation, and timeout protection.
 * Sets appropriate interrupt IDs for the interrupt system to process.
 * Includes debug logging for timing analysis and state verification.
 */
void ButtonSensor::ProcessButtonState()
{
    // Get the logical button state (ReadButtonState handles the GPIO inversion)
    bool currentState = ReadButtonState();
    unsigned long currentTime = millis();

    static unsigned long lastDebugTime = 0;
    static bool lastLoggedState = false;

    // Debug every 3 seconds or on state changes
    if ((currentTime - lastDebugTime > 3000) || (currentState != lastLoggedState)) {
        // Button state processing
        lastDebugTime = currentTime;
        lastLoggedState = currentState;
    }

    if (currentState && !currentButtonState_)
    {
        buttonPressStartTime_ = currentTime;
        currentButtonState_ = true;
        log_t("ButtonSensor: PRESS STARTED at %lu ms (GPIO HIGH detected)", buttonPressStartTime_);
    }
    else if (!currentState && currentButtonState_)
    {
        buttonPressDuration_ = currentTime - buttonPressStartTime_;
        currentButtonState_ = false;

        log_t("ButtonSensor: PRESS ENDED after %lu ms (GPIO LOW detected)", buttonPressDuration_);

        ButtonAction action = DetermineAction(buttonPressDuration_);
        if (action != ButtonAction::NONE)
        {
            detectedAction_ = action;
            actionReady_ = true;
            log_t("ButtonSensor: ACTION DETECTED: %s (actionReady=true)",
                  action == ButtonAction::SHORT_PRESS ? "SHORT_PRESS" : "LONG_PRESS");

            // In simplified system, determine which interrupt should be triggered
            if (action == ButtonAction::SHORT_PRESS)
            {
                log_t("ButtonSensor: Should trigger 'short_press' action");
                triggerInterruptId_ = "short_press";
            }
            else if (action == ButtonAction::LONG_PRESS)
            {
                log_t("ButtonSensor: Should trigger 'long_press' action");
                triggerInterruptId_ = "long_press";
            }
        }
        else
        {
            log_w("ButtonSensor: Press duration %lu ms resulted in NO ACTION", buttonPressDuration_);
        }
    }
    else if (currentState && currentButtonState_)
    {
        unsigned long heldDuration = currentTime - buttonPressStartTime_;
        if (heldDuration > LONG_PRESS_MAX_MS)
        {
            log_w("ButtonSensor: PRESS TIMEOUT after %lu ms - resetting state", heldDuration);
            currentButtonState_ = false;
            buttonPressStartTime_ = 0;
        }
    }
}

/**
 * @brief Determine button action based on press duration
 * @param duration Press duration in milliseconds
 * @return ButtonAction type based on duration
 */
ButtonAction ButtonSensor::DetermineAction(unsigned long duration)
{
    if (duration < DEBOUNCE_MS)
    {
        log_v("Press duration %lu ms - ignored (debounce)", duration);
        return ButtonAction::NONE;
    }
    else if (duration <= SHORT_PRESS_MAX_MS)
    {
        return ButtonAction::SHORT_PRESS;
    }
    else if (duration <= LONG_PRESS_MAX_MS)
    {
        return ButtonAction::LONG_PRESS;
    }
    else
    {
        log_v("Press duration %lu ms - ignored (timeout)", duration);
        return ButtonAction::NONE;
    }
}

/**
 * @brief Handles interrupt-driven button state processing
 *
 * Called when a hardware interrupt is triggered on the button GPIO pin.
 * Immediately processes the current button state to detect any actions.
 * This enables responsive button handling in interrupt-driven systems.
 */
void ButtonSensor::OnInterruptTriggered()
{
    ProcessButtonState();
}