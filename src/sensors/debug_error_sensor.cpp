#include "sensors/debug_error_sensor.h"
#include "managers/error_manager.h"
#include <Arduino.h>

// Constructors and Destructors
DebugErrorSensor::DebugErrorSensor(IGpioProvider *gpioProvider)
    : gpioProvider_(gpioProvider), previousState_(false), initialized_(false)
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
    
    // Configure pin as input with pull-down
    gpioProvider_->PinMode(gpio_pins::DEBUG_ERROR, INPUT_PULLDOWN);
    
    // Read initial state
    previousState_ = gpioProvider_->DigitalRead(gpio_pins::DEBUG_ERROR);
    initialized_ = true;
    
    log_d("DebugErrorSensor initialized, initial state: %s", previousState_ ? "HIGH" : "LOW");
}

Reading DebugErrorSensor::GetReading()
{
    if (!initialized_) {
        log_w("DebugErrorSensor not initialized");
        return false;
    }
    
    // Read current pin state
    bool currentState = gpioProvider_->DigitalRead(gpio_pins::DEBUG_ERROR);
    
    // Detect rising edge (LOW to HIGH transition)
    if (!previousState_ && currentState) {
        log_d("Debug error trigger detected - rising edge on GPIO %d", gpio_pins::DEBUG_ERROR);
        
        // Generate three test errors
        ErrorManager::Instance().ReportWarning("DebugTest", 
            "Test warning for debugging error panel functionality");
        
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "DebugTest", 
            "Test error for debugging error panel navigation");
        
        ErrorManager::Instance().ReportCriticalError("DebugTest", 
            "Test critical error for debugging error display");
        
        log_i("Debug errors generated: 1 WARNING, 1 ERROR, 1 CRITICAL");
    }
    
    // Update previous state for next edge detection
    previousState_ = currentState;
    
    // Return current state (true when HIGH)
    return currentState;
}