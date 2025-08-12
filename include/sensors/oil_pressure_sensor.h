#pragma once // preventing duplicate definitions, alternative to the traditional include guards

// System/Library Includes

// Project Includes
#include "sensors/unit_aware_sensor.h"
#include "hardware/gpio_pins.h"

/**
 * @class OilPressureSensor
 * @brief Oil pressure monitoring sensor with unit-aware conversions
 * 
 * @details This sensor monitors engine oil pressure levels and provides
 * readings in the unit requested by the consuming panel/component.
 * It inherits delta-based updates and time-based sampling from UnitAwareSensor.
 * 
 * @model_role Provides oil pressure data to OemOilPressureComponent
 * @supported_units: Bar, PSI, kPa
 * @range 0-10 Bar (0-145 PSI, 0-1000 kPa) typical automotive range
 * @update_frequency Configurable via constructor
 * 
 * @unit_conversion:
 * - Bar: Direct mapping 0-10 Bar
 * - PSI: 0-145 PSI (14.5 PSI per Bar)
 * - kPa: 0-1000 kPa (100 kPa per Bar)
 * 
 * @hardware_interface 3.3V analog pressure sensor input via GPIO pin
 * @calibration 22k potentiometer: 0V = 0 Bar, 3.3V = 10 Bar
 * 
 * @context This sensor provides pressure readings in the unit requested
 * by the panel, removing unit conversion logic from display components.
 */
class OilPressureSensor : public UnitAwareSensor
{
public:
    // Constructors and Destructors
    OilPressureSensor(IGpioProvider *gpioProvider, int updateRateMs = 500);

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
    // Pressure calibration constants
    static constexpr int32_t PRESSURE_MAX_BAR = 10; // Maximum pressure reading in Bar
    static constexpr int32_t PRESSURE_MAX_PSI = 145; // Maximum pressure reading in PSI (10 Bar * 14.5)
    static constexpr int32_t PRESSURE_MAX_KPA = 1000; // Maximum pressure reading in kPa (10 Bar * 100)
};