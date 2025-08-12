#include "sensors/debug_error_sensor.h"
#include "managers/error_manager.h"
#include <Arduino.h>

// Constructors and Destructors
DebugErrorSensor::DebugErrorSensor(IGpioProvider *gpioProvider)
    : gpioProvider_(gpioProvider), previousState_(false), initialized_(false), startupTime_(0)
{
    log_d("Creating DebugErrorSensor for GPIO %d", gpio_pins::DEBUG_ERROR);
}

// Core Functionality Methods
void DebugErrorSensor::Init()
{
    log_d("Initializing DebugErrorSensor on GPIO %d", gpio_pins::DEBUG_ERROR);
    
    if (!gpioProvider_) {
        log_e("DebugErrorSensor requires GPIO provider");
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
    startupTime_ = millis();  // Record startup time for grace period
    initialized_ = true;
    
    log_d("DebugErrorSensor initialized on GPIO %d, initial state: %s", 
          gpio_pins::DEBUG_ERROR, previousState_ ? "HIGH" : "LOW");
}

Reading DebugErrorSensor::GetReading()
{
    if (!initialized_) {
        log_w("DebugErrorSensor not initialized");
        return false;
    }
    
    // Ignore input during startup grace period (reduce to 1 second for faster testing)
    // This prevents false triggers from GPIO 34 floating during initialization
    const unsigned long STARTUP_GRACE_PERIOD_MS = 1000;
    if (millis() - startupTime_ < STARTUP_GRACE_PERIOD_MS) {
        log_v("DebugErrorSensor: Still in grace period (%lu ms remaining)", 
              STARTUP_GRACE_PERIOD_MS - (millis() - startupTime_));
        return previousState_;  // Return last known stable state
    }
    
    // Read current pin state with debouncing
    bool currentState = gpioProvider_->DigitalRead(gpio_pins::DEBUG_ERROR);
    
    // Simple debouncing: read twice with small delay to ensure stable signal
    delay(5);
    bool confirmedState = gpioProvider_->DigitalRead(gpio_pins::DEBUG_ERROR);
    
    // Log pin readings for debugging
    static unsigned long lastLogTime = 0;
    if (millis() - lastLogTime > 5000 || currentState != previousState_) {  // Log every 5 seconds or on change
        log_d("DebugErrorSensor GPIO %d: first=%s, confirmed=%s, previous=%s", 
              gpio_pins::DEBUG_ERROR,
              currentState ? "HIGH" : "LOW", 
              confirmedState ? "HIGH" : "LOW",
              previousState_ ? "HIGH" : "LOW");
        lastLogTime = millis();
    }
    
    // Only proceed if both readings match
    if (currentState != confirmedState) {
        log_d("DebugErrorSensor: Unstable reading detected, ignoring");
        return previousState_;  // Return previous stable state
    }
    
    // Detect rising edge (LOW to HIGH transition) with confirmed stable reading
    if (!previousState_ && confirmedState) {
        log_i("Debug error trigger activated - rising edge detected on GPIO %d", gpio_pins::DEBUG_ERROR);
        
        // Generate three test errors for error panel testing
        ErrorManager::Instance().ReportWarning("DebugTest", 
            "Test warning for debugging error panel functionality");
        
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "DebugTest", 
            "Test error for debugging error panel navigation");
        
        ErrorManager::Instance().ReportCriticalError("DebugTest", 
            "Test critical error for debugging error display");
        
        log_i("Debug errors generated: 1 WARNING, 1 ERROR, 1 CRITICAL");
    }
    
    // Update previous state for next edge detection
    previousState_ = confirmedState;
    
    // Return current confirmed state
    return confirmedState;
}