#pragma once // preventing duplicate definitions, alternative to the traditional include guards

// System/Library Includes
#include <string>
#include <vector>

// Project Includes
#include "hardware/gpio_pins.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_preference_service.h"
#include "sensors/base_sensor.h"
#include "utilities/sensor_helper.h"
#include "utilities/types.h"

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
class OilPressureSensor : public BaseSensor
{
  public:
    // Constructors and Destructors
    OilPressureSensor(IGpioProvider *gpioProvider, int updateRateMs = 500);
    OilPressureSensor(IGpioProvider *gpioProvider, IPreferenceService *preferenceService, int updateRateMs = 500);

    // BaseSensor interface implementation
    void Init() override;
    Reading GetReading() override;
    bool HasStateChanged() override;
    
    // ISensor optional methods
    void SetTargetUnit(const std::string &unit) override;
    std::vector<std::string> GetSupportedUnits() const override;

    /// @brief Set the update rate in milliseconds
    /// @param updateRateMs Update interval in milliseconds
    void SetUpdateRate(int updateRateMs);

  protected:
    // Internal methods
    int32_t ReadRawValue();
    int32_t ConvertReading(int32_t rawValue);

  private:
    // Instance members
    IGpioProvider *gpioProvider_;
    IPreferenceService *preferenceService_ = nullptr;
    std::string targetUnit_ = "Bar";
    int32_t currentReading_ = 0;
    int32_t previousReading_ = 0;  // For change detection
    unsigned long lastUpdateTime_ = 0;
    unsigned long updateIntervalMs_;
};