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

    static bool IsUnitSupported(const std::string &unit, const std::vector<std::string> &supportedUnits)
    {
        return std::find(supportedUnits.begin(), supportedUnits.end(), unit) != supportedUnits.end();
    }

    static bool IsValidAdcReading(int32_t rawValue)
    {
        return rawValue >= 0 && rawValue <= ADC_MAX_VALUE;
    }
};