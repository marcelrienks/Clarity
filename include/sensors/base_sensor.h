#pragma once

#include "utilities/types.h"
#include "utilities/constants.h"
#include <functional>

#include "esp32-hal-log.h"

/**
 * @class BaseSensor
 * @brief Base class providing consistent change detection and interrupt registration for all sensors
 *
 * @details This base class implements the change detection pattern required by
 * the coordinated interrupt system. All sensors must inherit from BaseSensor
 * to ensure consistent change detection behavior and prevent corruption from
 * multiple evaluations per cycle. Phase 3 adds interrupt registration capabilities.
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
class BaseSensor
{
protected:
    bool initialized_ = false; ///< Initialization flag for first-read handling

    /**
     * @brief Template method for consistent change detection across all sensor types
     * @tparam T The data type being compared (bool, int32_t, double, etc.)
     * @param currentValue The current sensor reading
     * @param previousValue Reference to stored previous value (updated by this method)
     * @return true if value has changed since last call, false otherwise
     *
     * @details This template method provides consistent change detection logic:
     * - First call (initialization): Sets previousValue, returns false (no change)
     * - Subsequent calls: Compares values, updates previousValue, returns change status
     * - Atomic operation: Value comparison and update happen together
     * - Thread-safe: Designed for ESP32 single-threaded interrupt processing
     *
     * @change_detection_rules:
     * 1. First read never reports a change (initialization)
     * 2. Previous state is updated atomically with comparison
     * 3. Each sensor maintains its own change detection state
     * 4. Template supports any comparable type (bool, int32_t, double, etc.)
     *
     * @corruption_prevention This method must be called exactly once per evaluation
     * cycle to prevent change detection corruption. Multiple calls in the same
     * cycle will corrupt the previousValue state.
     */
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

public:
    virtual ~BaseSensor() = default;
    
    
    /**
     * @brief Check if this sensor has state changes (must be implemented by derived classes)
     * @return true if sensor state has changed since last evaluation
     * @note This method should use DetectChange template for consistency
     */
    virtual bool HasStateChanged() { return false; }

protected:
    
    /**
     * @brief Virtual method for sensor-specific interrupt execution (override in derived classes)
     * @details This method is called when the interrupt condition is met
     * Default implementation does nothing - sensors should override for specific behavior
     */
    virtual void OnInterruptTriggered() { 
        log_d("BaseSensor interrupt triggered for sensor - no specific action defined"); 
    }
};