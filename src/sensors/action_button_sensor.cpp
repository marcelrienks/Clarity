#include "sensors/action_button_sensor.h"
#include <Arduino.h>

#ifdef CLARITY_DEBUG
    #include "esp32-hal-log.h"
    #ifndef LOG_TAG
        #define LOG_TAG "ActionButtonSensor"
    #endif
#else
    #define log_d(...)
#endif

// Constructors and Destructors

ActionButtonSensor::ActionButtonSensor(IGpioProvider *gpioProvider) : gpioProvider_(gpioProvider)
{
    log_v("ActionButtonSensor() constructor called");
}

// ISensor Interface Implementation

/// @brief Initialize the action button sensor on GPIO 32
/// @details Configures GPIO with pull-down resistor for button input
void ActionButtonSensor::Init()
{
    log_v("Init() called");
    // Configure GPIO 32 as input with pull-down resistor
    // This ensures LOW state when button is not pressed (Normally Open button to 3.3V)
    gpioProvider_->PinMode(gpio_pins::INPUT_BUTTON, INPUT_PULLDOWN);

    log_d("ActionButtonSensor initialized on GPIO %d with pull-down", gpio_pins::INPUT_BUTTON);

    // Log initial state after proper configuration
    bool initialState = gpioProvider_->DigitalRead(gpio_pins::INPUT_BUTTON);
    log_i("GPIO %d initial state after configuration: %s", gpio_pins::INPUT_BUTTON,
          initialState ? "HIGH (pressed)" : "LOW (released)");
    
    // Register coordinated interrupts for button press detection
    bool polledRegistered = RegisterPolledInterrupt(
        "button_state_monitor",       // Unique ID
        Priority::IMPORTANT,          // Important priority for user input
        InterruptEffect::BUTTON_ACTION, // Button actions
        50                           // 50ms polling for responsive button detection
    );
    
    if (polledRegistered)
    {
        log_d("Registered polled interrupt for button state monitoring");
    }
    else
    {
        log_w("Failed to register polled interrupt for action button sensor");
    }
    
    log_i("ActionButtonSensor initialization completed on GPIO %d with coordinated interrupts", gpio_pins::INPUT_BUTTON);
}

/// @brief Get the current button state as a sensor reading
/// @return Reading containing 1 if pressed, 0 if released
Reading ActionButtonSensor::GetReading()
{
    log_v("GetReading() called");
    // Return button state as boolean (0 = not pressed, 1 = pressed)
    bool pressed = IsButtonPressed();
    return static_cast<int32_t>(pressed ? 1 : 0);
}

/// @brief Check if the action button is currently pressed
/// @return true if button is pressed (GPIO HIGH), false otherwise
bool ActionButtonSensor::IsButtonPressed()
{
    log_v("IsButtonPressed() called");
    // GPIO 32 reads HIGH when button is pressed (connected to 3.3V)
    return gpioProvider_->DigitalRead(gpio_pins::INPUT_BUTTON);
}

bool ActionButtonSensor::readButtonState()
{
    log_v("readButtonState() called");
    return gpioProvider_->DigitalRead(gpio_pins::INPUT_BUTTON);
}

bool ActionButtonSensor::HasStateChanged()
{
    log_v("HasStateChanged() called");
    
    bool currentState = readButtonState();
    bool changed = DetectChange(currentState, previousButtonState_);
    
    if (changed)
    {
        log_d("Button state changed from %s to %s", 
              previousButtonState_ ? "PRESSED" : "RELEASED",
              currentState ? "PRESSED" : "RELEASED");
    }
    
    return changed;
}

void ActionButtonSensor::OnInterruptTriggered()
{
    log_v("OnInterruptTriggered() called");
    
    bool currentState = readButtonState();
    log_i("Button sensor interrupt triggered - current state: %s", 
          currentState ? "PRESSED" : "RELEASED");
    
    // Notify ActionManager about button state change
    // This will be handled by the ActionManager's interrupt callback
}