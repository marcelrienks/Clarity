#include "sensors/example_interrupt_sensor.h"
#include "managers/error_manager.h"
#include "hardware/gpio_pins.h"
#include <Arduino.h>

#ifdef CLARITY_DEBUG
    #include "esp32-hal-log.h"
    #define LOG_TAG "ExampleInterruptSensor"
#else
    #define log_v(...)
    #define log_d(...)
    #define log_w(...)
    #define log_i(...)
    #define log_e(...)
#endif

ExampleInterruptSensor::ExampleInterruptSensor(IGpioProvider* gpioProvider) 
    : gpioProvider_(gpioProvider)
{
    log_v("ExampleInterruptSensor() constructor called");
}

void ExampleInterruptSensor::Init()
{
    log_v("Init() called");
    
    if (!gpioProvider_)
    {
        log_e("GPIO provider is null");
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "ExampleInterruptSensor", 
                                           "Cannot initialize - GPIO provider is null");
        return;
    }
    
    // Configure GPIO for sensor input
    gpioProvider_->PinMode(gpio_pins::KEY_PRESENT, INPUT_PULLDOWN);
    
    // Register example polled interrupt - checks state every 200ms
    bool polledRegistered = RegisterPolledInterrupt(
        "example_sensor_state",           // Unique ID
        Priority::NORMAL,                 // Normal priority
        InterruptEffect::BUTTON_ACTION,   // Custom action effect
        200                              // 200ms polling interval
    );
    
    if (polledRegistered)
    {
        log_d("Registered polled interrupt for state monitoring");
    }
    else
    {
        log_w("Failed to register polled interrupt");
    }
    
    // Register example queued interrupt - for panel switching (UI-dependent)
    bool queuedRegistered = RegisterQueuedInterrupt(
        "example_panel_switch",           // Unique ID
        Priority::IMPORTANT,              // Important priority
        InterruptEffect::LOAD_PANEL,     // Panel loading effect
        (void*)"ConfigPanel"             // Panel name to load
    );
    
    if (queuedRegistered)
    {
        log_d("Registered queued interrupt for panel switching");
    }
    else
    {
        log_w("Failed to register queued interrupt");
    }
    
    log_i("ExampleInterruptSensor initialized with coordinated interrupts");
}

Reading ExampleInterruptSensor::GetReading()
{
    log_v("GetReading() called");
    
    int32_t currentValue = getCurrentValue();
    log_d("Sensor reading: %d", currentValue);
    
    return currentValue;
}

bool ExampleInterruptSensor::HasStateChanged()
{
    log_v("HasStateChanged() called");
    
    // Get current state and use BaseSensor change detection
    bool currentState = getCurrentState();
    bool stateChanged = DetectChange(currentState, previousState_);
    
    // Also check for value changes
    int32_t currentValue = getCurrentValue();
    bool valueChanged = DetectChange(currentValue, previousValue_);
    
    bool changed = stateChanged || valueChanged;
    
    if (changed)
    {
        log_d("State changed - state: %s, value: %d", 
              currentState ? "true" : "false", currentValue);
    }
    
    return changed;
}

void ExampleInterruptSensor::OnInterruptTriggered()
{
    log_v("OnInterruptTriggered() called");
    
    unsigned long currentTime = millis();
    
    // Example custom behavior: prevent rapid panel switches
    if (currentTime - lastPanelSwitchTime_ < 2000) // 2 second cooldown
    {
        log_d("Panel switch on cooldown - ignoring interrupt");
        return;
    }
    
    log_i("Example sensor interrupt triggered - executing custom action");
    
    // Example: Update cooldown time
    lastPanelSwitchTime_ = currentTime;
    
    // Example: Report sensor activity
    bool currentState = getCurrentState();
    int32_t currentValue = getCurrentValue();
    
    log_i("Sensor interrupt action - state: %s, value: %d, time: %lu",
          currentState ? "active" : "inactive", currentValue, currentTime);
    
    // In a real implementation, this might:
    // - Trigger specific panel loading
    // - Update system preferences  
    // - Send notifications
    // - Update UI state
}

// Private helper methods
bool ExampleInterruptSensor::getCurrentState()
{
    if (!gpioProvider_)
    {
        return false;
    }
    
    // Example: Read GPIO state
    return gpioProvider_->DigitalRead(gpio_pins::KEY_PRESENT);
}

int32_t ExampleInterruptSensor::getCurrentValue()
{
    // Example: Convert boolean state to numeric value
    bool state = getCurrentState();
    return state ? 100 : 0; // Example mapping
}