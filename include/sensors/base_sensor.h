#pragma once

#include "interfaces/i_sensor.h"
#include "definitions/types.h"
#include "definitions/constants.h"
#include <functional>
#include <Arduino.h>

#include "esp32-hal-log.h"

/**
 * @class BaseSensor
 * @brief Base class providing consistent change detection and interrupt registration for all sensors
 *
 * @details This base class implements the change detection pattern required by
 * the coordinated interrupt system. All sensors must inherit from BaseSensor
 * to ensure consistent change detection behavior and prevent corruption from
 * multiple evaluations per cycle.
 *
 * @design_pattern Template method pattern for change detection
 * @change_detection Prevents corruption through single evaluation per cycle
 * @memory_safe Designed for ESP32 memory constraints with minimal overhead
 * @interrupt_support Provides helper methods for registering sensor interrupts
 *
 * @architecture_requirement All sensors in the system must inherit from BaseSensor
 * to provide consistent change detection patterns for the interrupt system.
 *
 * @usage_pattern:
 * 1. Sensor inherits from both ISensor and BaseSensor
 * 2. Sensor implements HasStateChanged() method using DetectChange template
 * 3. Sensor optionally registers interrupts using RegisterInterrupt methods
 * 4. Interrupt evaluation calls HasStateChanged() exactly once per cycle
 * 5. Change detection prevents corruption from multiple calls
 *
 * @prevention Eliminates change detection corruption that occurs when
 * HasStateChanged() is called multiple times per evaluation cycle.
 *
 * @example:
 * ```cpp
 * class KeyPresentSensor : public ISensor, public BaseSensor {
 * public:
 *     void Init() override {
 *         RegisterTrigger("key_present", Priority::IMPORTANT, 
 *                                InterruptEffect::LOAD_PANEL, 200);
 *     }
 *     bool HasStateChanged() {
 *         bool currentState = std::get<bool>(GetReading());
 *         return DetectChange(currentState, previousState_);
 *     }
 * private:
 *     bool previousState_ = false;
 * };
 * ```
 */
class BaseSensor : public ISensor
{
public:
    // ========== Constructors and Destructor ==========
    virtual ~BaseSensor() = default;
    
    // ========== Public Interface Methods ==========
    // ISensor interface methods (must be implemented by concrete sensors)
    virtual void Init() = 0;
    virtual Reading GetReading() = 0;
    
    virtual bool HasStateChanged() = 0;

    virtual void OnInterruptTriggered() {
    }

protected:
    // ========== Protected Methods ==========
    template<typename T>
    bool DetectChange(T currentValue, T& previousValue)
    {
        if (!initialized_) {
            previousValue = currentValue;
            initialized_ = true;
            return false; // No change on first read
        }

        bool changed = (currentValue != previousValue);
        previousValue = currentValue;
        return changed;
    }

    bool ShouldUpdate(unsigned long &lastUpdateTime, unsigned long updateIntervalMs)
    {
        unsigned long currentTime = millis();
        if (currentTime - lastUpdateTime >= updateIntervalMs)
        {
            lastUpdateTime = currentTime;
            return true;
        }
        return false;
    }

    bool IsValidAdcReading(int32_t rawValue)
    {
        return rawValue >= 0 && rawValue <= ADC_MAX_VALUE;
    }

    // ========== Protected Constants ==========
    static constexpr int32_t ADC_MAX_VALUE = 4095; // 12-bit ADC
    static constexpr float SUPPLY_VOLTAGE = 3.3f;  // ESP32 3.3V supply

    // ========== Protected Data Members ==========
    bool initialized_ = false; ///< Initialization flag for first-read handling
};