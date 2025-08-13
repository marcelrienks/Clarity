#pragma once

// System/Library Includes
#include <Arduino.h>
#include <algorithm>
#include <string>
#include <vector>

// Project Includes
#include "utilities/types.h"

/**
 * @class SensorHelper
 * @brief Helper class for common sensor operations
 *
 * @details This class provides utility functions for sensors including:
 * - Time-based update rate limiting
 * - Unit validation
 * - ADC constants and conversions
 *
 * @usage: Sensors can use these static methods to avoid code duplication
 */
class SensorHelper
{
  public:
    // ADC constants
    static constexpr int32_t ADC_MAX_VALUE = 4095; // 12-bit ADC
    static constexpr float SUPPLY_VOLTAGE = 3.3f;  // ESP32 3.3V supply

    /**
     * @brief Check if enough time has passed for next sensor update
     * @param lastUpdateTime Last update timestamp in milliseconds
     * @param updateIntervalMs Update interval in milliseconds
     * @return True if update should occur
     */
    static bool ShouldUpdate(unsigned long &lastUpdateTime, unsigned long updateIntervalMs)
    {
        unsigned long currentTime = millis();
        if (currentTime - lastUpdateTime >= updateIntervalMs)
        {
            lastUpdateTime = currentTime;
            return true;
        }
        return false;
    }

    /**
     * @brief Validate if a unit is in the supported units list
     * @param unit Unit to validate
     * @param supportedUnits List of supported units
     * @return True if unit is supported
     */
    static bool IsUnitSupported(const std::string &unit, const std::vector<std::string> &supportedUnits)
    {
        return std::find(supportedUnits.begin(), supportedUnits.end(), unit) != supportedUnits.end();
    }

    /**
     * @brief Check if ADC reading is valid
     * @param rawValue ADC reading to validate
     * @return True if reading is within valid ADC range
     */
    static bool IsValidAdcReading(int32_t rawValue)
    {
        return rawValue >= 0 && rawValue <= ADC_MAX_VALUE;
    }
};