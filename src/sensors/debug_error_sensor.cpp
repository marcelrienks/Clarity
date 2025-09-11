// Only compile debug error sensor in debug builds
#ifdef CLARITY_DEBUG

#include "sensors/debug_error_sensor.h"
#include "managers/error_manager.h"
#include <Arduino.h>
#include <esp32-hal-log.h>

// Constructors and Destructors
DebugErrorSensor::DebugErrorSensor(IGpioProvider *gpioProvider)
    : gpioProvider_(gpioProvider), previousState_(false), initialized_(false), startupTime_(0)
{
    log_v("DebugErrorSensor() constructor called");
}

// Core Functionality Methods
void DebugErrorSensor::Init()
{
    log_v("Init() called");

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
    
    // Interrupt registration is now handled centrally in ManagerFactory

    // Debug error sensor initialized
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
        return previousState_; // Return previous stable state
    }

    // Generate errors whenever button is pressed (GPIO HIGH) and error queue is empty
    if (confirmedState)
    {
        log_i("Debug error button pressed - GPIO %d is HIGH", gpio_pins::DEBUG_ERROR);
        
        // Only generate errors if error queue is currently empty
        if (!ErrorManager::Instance().HasPendingErrors())
        {
            log_i("Error queue is empty - generating test errors");
            
            // Generate three test errors for error panel testing
            ErrorManager::Instance().ReportWarning("DebugTest", "Test warning for debugging error panel functionality");

            ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "DebugTest",
                                                 "Test error for debugging error panel navigation");

            ErrorManager::Instance().ReportCriticalError("DebugTest", "Test critical error for debugging error display");

            log_i("Debug errors generated: 1 WARNING, 1 ERROR, 1 CRITICAL");
        }
        else
        {
            log_i("Error queue not empty - ignoring debug button press");
        }
    }

    // For push button debug sensor, always return false after generating errors
    // This prevents the trigger system from keeping the trigger active
    return false;
}

bool DebugErrorSensor::readPinState()
{
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
    
    log_v("readPinState: Readings don't match, returning previous state=%s", 
          previousState_ ? "HIGH" : "LOW");
    // Return previous stable state if readings don't match
    return previousState_;
}

bool DebugErrorSensor::HasStateChanged()
{
    log_v("HasStateChanged() called");
    
    if (!initialized_) {
        log_v("HasStateChanged: Not initialized, returning false");
        return false;
    }
    
    // Ignore input during startup grace period
    const unsigned long STARTUP_GRACE_PERIOD_MS = 1000;
    unsigned long currentTime = millis();
    if (currentTime - startupTime_ < STARTUP_GRACE_PERIOD_MS)
    {
        log_v("HasStateChanged: Still in grace period (%lu ms remaining)", 
              STARTUP_GRACE_PERIOD_MS - (currentTime - startupTime_));
        return false;
    }
    
    bool currentState = readPinState();
    
    // Simply return true whenever button is pressed
    // This will trigger GetReading() which handles error generation
    if (currentState)
    {
    }
    
    return currentState;
}

void DebugErrorSensor::OnInterruptTriggered()
{
    log_v("OnInterruptTriggered() called");
    
    log_i("Debug error trigger activated via interrupt - generating test errors");
    
    // Generate three test errors for error panel testing
    ErrorManager::Instance().ReportWarning("DebugTest", 
                                           "Test warning from debug error sensor");
    
    ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "DebugTest",
                                         "Test error from debug error sensor");
    
    ErrorManager::Instance().ReportCriticalError("DebugTest", 
                                                 "Test critical error from debug error sensor");
    
    log_i("Debug errors generated: 1 WARNING, 1 ERROR, 1 CRITICAL");
}

#endif // CLARITY_DEBUG