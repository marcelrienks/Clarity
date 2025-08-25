#include "sensors/debug_error_sensor.h"
#include "managers/error_manager.h"
#include <Arduino.h>
#include <esp32-hal-log.h>

// Constructors and Destructors
DebugErrorSensor::DebugErrorSensor(IGpioProvider *gpioProvider)
    : gpioProvider_(gpioProvider), previousState_(false), initialized_(false), startupTime_(0)
{
    log_v("DebugErrorSensor() constructor called");
    log_d("Creating DebugErrorSensor for GPIO %d", gpio_pins::DEBUG_ERROR);
}

// Core Functionality Methods
void DebugErrorSensor::Init()
{
    log_v("Init() called");
    log_d("Initializing DebugErrorSensor on GPIO %d", gpio_pins::DEBUG_ERROR);

    if (!gpioProvider_)
    {
        log_e("DebugErrorSensor requires GPIO provider");
        ErrorManager::Instance().ReportCriticalError("DebugErrorSensor", "Cannot initialize - GPIO provider is null");
        return;
    }

    // Configure pin as input with pull-down resistor
    // Note: GPIO 34 is input-only and lacks internal pull resistors on ESP32
    // We'll use INPUT_PULLDOWN which may not work on GPIO 34, but try anyway
    gpioProvider_->PinMode(gpio_pins::DEBUG_ERROR, INPUT_PULLDOWN);

    // Allow time for pin to stabilize after configuration
    delay(10);

    // Read initial stable state after configuration
    previousState_ = gpioProvider_->DigitalRead(gpio_pins::DEBUG_ERROR);
    startupTime_ = millis(); // Record startup time for grace period
    initialized_ = true;
    
    // Register coordinated interrupts for debug error detection
    bool polledRegistered = RegisterPolledInterrupt(
        "debug_error_monitor",        // Unique ID
        Priority::CRITICAL,           // Critical priority for error triggers
        InterruptEffect::LOAD_PANEL,  // Errors trigger error panel
        300                          // 300ms polling for debug error detection
    );
    
    if (polledRegistered)
    {
        log_d("Registered polled interrupt for debug error monitoring");
    }
    else
    {
        log_w("Failed to register polled interrupt for debug error sensor");
    }

    log_d("DebugErrorSensor initialized on GPIO %d with coordinated interrupts, initial state: %s", 
          gpio_pins::DEBUG_ERROR, previousState_ ? "HIGH" : "LOW");
}

Reading DebugErrorSensor::GetReading()
{
    log_v("GetReading() called");
    if (!initialized_)
    {
        log_w("DebugErrorSensor not initialized");
        return false;
    }

    // Ignore input during startup grace period (reduce to 1 second for faster testing)
    // This prevents false triggers from GPIO 34 floating during initialization
    const unsigned long STARTUP_GRACE_PERIOD_MS = 1000;
    if (millis() - startupTime_ < STARTUP_GRACE_PERIOD_MS)
    {
        // Grace period - suppress logging to reduce noise
        return previousState_; // Return last known stable state
    }

    // Read current pin state with debouncing
    bool currentState = gpioProvider_->DigitalRead(gpio_pins::DEBUG_ERROR);

    // Simple debouncing: read twice with small delay to ensure stable signal
    delay(5);
    bool confirmedState = gpioProvider_->DigitalRead(gpio_pins::DEBUG_ERROR);


    // Only proceed if both readings match
    if (currentState != confirmedState)
    {
        log_d("DebugErrorSensor: Unstable reading detected, ignoring");
        return previousState_; // Return previous stable state
    }

    // Detect rising edge (LOW to HIGH transition) with confirmed stable reading
    if (!previousState_ && confirmedState)
    {
        log_i("Debug error trigger activated - rising edge detected on GPIO %d", gpio_pins::DEBUG_ERROR);

        // Generate three test errors for error panel testing
        ErrorManager::Instance().ReportWarning("DebugTest", "Test warning for debugging error panel functionality");

        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "DebugTest",
                                             "Test error for debugging error panel navigation");

        ErrorManager::Instance().ReportCriticalError("DebugTest", "Test critical error for debugging error display");

        log_i("Debug errors generated: 1 WARNING, 1 ERROR, 1 CRITICAL");
    }

    // Update previous state for next edge detection
    previousState_ = confirmedState;

    // Return current confirmed state
    return confirmedState;
}

bool DebugErrorSensor::readPinState()
{
    log_v("readPinState() called");
    if (!initialized_) return false;
    
    // Use the same debouncing logic as GetReading
    bool currentState = gpioProvider_->DigitalRead(gpio_pins::DEBUG_ERROR);
    delay(5);
    bool confirmedState = gpioProvider_->DigitalRead(gpio_pins::DEBUG_ERROR);
    
    // Only return if both readings match
    if (currentState == confirmedState)
    {
        return confirmedState;
    }
    
    // Return previous stable state if readings don't match
    return previousState_;
}

bool DebugErrorSensor::HasStateChanged()
{
    log_v("HasStateChanged() called");
    
    if (!initialized_) return false;
    
    // Ignore input during startup grace period
    const unsigned long STARTUP_GRACE_PERIOD_MS = 1000;
    if (millis() - startupTime_ < STARTUP_GRACE_PERIOD_MS)
    {
        return false;
    }
    
    bool currentState = readPinState();
    
    // Only detect rising edges (LOW to HIGH transition)
    bool changed = !previousState_ && currentState;
    
    if (changed)
    {
        log_d("Debug error state changed - rising edge detected: LOW -> HIGH");
    }
    
    return changed;
}

void DebugErrorSensor::OnInterruptTriggered()
{
    log_v("OnInterruptTriggered() called");
    
    bool currentState = readPinState();
    log_i("Debug error sensor interrupt triggered - current state: %s", 
          currentState ? "HIGH" : "LOW");
    
    // Generate test errors on rising edge
    if (!previousState_ && currentState)
    {
        log_i("Debug error trigger activated via interrupt - generating test errors");
        
        // Generate three test errors for error panel testing
        ErrorManager::Instance().ReportWarning("DebugTestInterrupt", 
                                               "Test warning from interrupt-based debug error sensor");
        
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "DebugTestInterrupt",
                                             "Test error from interrupt-based debug error sensor");
        
        ErrorManager::Instance().ReportCriticalError("DebugTestInterrupt", 
                                                     "Test critical error from interrupt-based debug error sensor");
        
        log_i("Debug errors generated via interrupt: 1 WARNING, 1 ERROR, 1 CRITICAL");
        
        // Update previous state
        previousState_ = currentState;
    }
}