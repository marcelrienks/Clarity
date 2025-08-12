#pragma once

// System/Library Includes
#include <string>
#include <vector>

// Project Includes
#include "interfaces/i_sensor.h"
#include "interfaces/i_gpio_provider.h"
#include "utilities/types.h"

/**
 * @class UnitAwareSensor
 * @brief Base class for sensors that support unit conversion
 * 
 * @details This abstract base class provides common functionality for sensors
 * that need to support multiple units of measure. It handles unit storage,
 * validation, and provides a framework for unit conversion.
 * 
 * @design_features:
 * - Stores target unit set by panel/component
 * - Provides unit validation against supported units
 * - Handles common sensor update logic (time-based sampling, delta detection)
 * - Separates ADC reading from unit conversion
 * 
 * @usage:
 * 1. Inherit from this class for unit-aware sensors
 * 2. Override GetSupportedUnits() to define valid units
 * 3. Override ConvertReading() to implement unit conversion
 * 4. Override ReadRawValue() to get ADC/hardware value
 * 
 * @benefits:
 * - Consistent unit handling across sensors
 * - Reusable update logic (time-based, delta)
 * - Clear separation of concerns
 * - Easy to add new unit-aware sensors
 */
class UnitAwareSensor : public ISensor
{
public:
    // Constructor
    UnitAwareSensor(IGpioProvider* gpioProvider, int updateRateMs);
    
    // ISensor interface implementation
    void SetTargetUnit(const std::string& unit) override;
    Reading GetReading() override;
    
protected:
    // Protected methods for derived classes
    
    /// @brief Read raw value from hardware (ADC, GPIO, etc)
    /// @return Raw sensor value before unit conversion
    virtual int32_t ReadRawValue() = 0;
    
    /// @brief Convert raw value to target unit
    /// @param rawValue The raw sensor reading
    /// @return Value converted to target unit
    virtual int32_t ConvertReading(int32_t rawValue) = 0;
    
    /// @brief Check if unit is supported by this sensor
    /// @param unit Unit to validate
    /// @return True if unit is supported
    bool IsUnitSupported(const std::string& unit) const;
    
    // Protected members accessible to derived classes
    IGpioProvider* gpioProvider_;
    std::string targetUnit_;
    int32_t currentReading_ = 0;
    int32_t previousReading_ = -1;
    unsigned long lastUpdateTime_ = 0;
    unsigned long updateIntervalMs_;
    
    // ADC constants
    static constexpr int32_t ADC_MAX_VALUE = 4095; // 12-bit ADC
    static constexpr float SUPPLY_VOLTAGE = 3.3f; // ESP32 3.3V supply
};