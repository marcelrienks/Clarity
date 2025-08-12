#pragma once // preventing duplicate definitions, alternative to the traditional include guards

// System/Library Includes

// Project Includes
#include "sensors/unit_aware_sensor.h"
#include "hardware/gpio_pins.h"

/**
 * @class OilTemperatureSensor
 * @brief Oil temperature monitoring sensor with unit-aware conversions
 * 
 * @details This sensor monitors engine oil temperature levels and provides
 * readings in the unit requested by the consuming panel/component.
 * It inherits delta-based updates and time-based sampling from UnitAwareSensor.
 * 
 * @model_role Provides oil temperature data to OemOilTemperatureComponent
 * @supported_units: C (Celsius), F (Fahrenheit)
 * @range 0-120°C (32-248°F) typical automotive range
 * @update_frequency Configurable via constructor
 * 
 * @unit_conversion:
 * - C: Direct mapping 0-120°C
 * - F: 32-248°F (C * 9/5 + 32)
 * 
 * @hardware_interface 3.3V analog temperature sensor input via GPIO pin
 * @calibration 22k potentiometer: 0V = 0°C, 3.3V = 120°C
 * 
 * @context This sensor provides temperature readings in the unit requested
 * by the panel, removing unit conversion logic from display components.
 */
class OilTemperatureSensor : public UnitAwareSensor
{
public:
    // Constructors and Destructors
    OilTemperatureSensor(IGpioProvider *gpioProvider, int updateRateMs = 500);

    // Core Functionality Methods
    void Init() override;
    std::vector<std::string> GetSupportedUnits() const override;
    
    /// @brief Set the update rate in milliseconds
    /// @param updateRateMs Update interval in milliseconds
    void SetUpdateRate(int updateRateMs) { updateIntervalMs_ = updateRateMs; }

protected:
    // UnitAwareSensor implementation
    int32_t ReadRawValue() override;
    int32_t ConvertReading(int32_t rawValue) override;
    
private:
    // Temperature calibration constants
    static constexpr int32_t TEMPERATURE_MAX_CELSIUS = 120; // Maximum temperature reading in Celsius
    static constexpr int32_t TEMPERATURE_MIN_FAHRENHEIT = 32; // 0°C in Fahrenheit
    static constexpr int32_t TEMPERATURE_MAX_FAHRENHEIT = 248; // 120°C in Fahrenheit
};