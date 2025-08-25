#pragma once

#include "interfaces/i_sensor.h"
#include "sensors/base_sensor.h"
#include "interfaces/i_gpio_provider.h"
#include "utilities/types.h"
#include "utilities/constants.h"

#ifdef CLARITY_DEBUG
    #include "esp32-hal-log.h"
    #define LOG_TAG "ExampleInterruptSensor"
#else
    #define log_v(...)
    #define log_d(...)
    #define log_w(...)
    #define log_e(...)
#endif

/**
 * @class ExampleInterruptSensor
 * @brief Example sensor demonstrating the new coordinated interrupt system
 * 
 * @details This sensor shows how to integrate with the Phase 3 interrupt system:
 * 1. Inherits from both ISensor and BaseSensor
 * 2. Registers polled and queued interrupts during Init()
 * 3. Implements HasStateChanged() using DetectChange template
 * 4. Overrides OnInterruptTriggered() for custom behavior
 * 
 * @example_usage This sensor demonstrates:
 * - Polled interrupt for high-frequency state monitoring
 * - Queued interrupt for UI-dependent actions (panel loading)
 * - Proper change detection preventing corruption
 * - Custom interrupt action execution
 */
class ExampleInterruptSensor : public ISensor, public BaseSensor
{
public:
    ExampleInterruptSensor(IGpioProvider* gpioProvider);
    ~ExampleInterruptSensor() = default;
    
    // ISensor interface
    void Init() override;
    Reading GetReading() override;
    
    // BaseSensor change detection
    bool HasStateChanged() override;
    
protected:
    // Custom interrupt behavior
    void OnInterruptTriggered() override;
    
private:
    IGpioProvider* gpioProvider_;
    bool previousState_ = false;
    int32_t previousValue_ = -1;
    unsigned long lastPanelSwitchTime_ = 0;
    
    // Helper methods
    bool getCurrentState();
    int32_t getCurrentValue();
};