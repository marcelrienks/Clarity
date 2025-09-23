#pragma once

// System/Library Includes
#include <string>
#include <vector>

// Project Includes
#include "hardware/gpio_pins.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_configuration_manager.h"
#include "interfaces/i_config.h"
#include "sensors/base_sensor.h"
#include "definitions/types.h"
#include "definitions/constants.h"
#include "definitions/configs.h"

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
class OilTemperatureSensor : public BaseSensor, public IConfig
{
  public:
    // ========== Constructors and Destructor ==========
    OilTemperatureSensor(IGpioProvider *gpioProvider, int updateRateMs = 500);
    OilTemperatureSensor(IGpioProvider *gpioProvider, IConfigurationManager *preferenceService, int updateRateMs = 500);

    // ========== Public Interface Methods ==========
    // BaseSensor interface implementation
    void Init() override;
    Reading GetReading() override;
    bool HasStateChanged() override;
    
    // ISensor optional methods
    void SetTargetUnit(const std::string &unit) override;
    std::vector<std::string> GetSupportedUnits() const override;

    void SetUpdateRate(int updateRateMs);

    void LoadConfiguration();

    // IConfig implementation (instance method for backward compatibility)
    void RegisterConfig(IConfigurationManager* preferenceService) override;

    // Static schema registration for self-registering pattern
    static void RegisterConfigSchema(IConfigurationManager* preferenceService);

    void RegisterLiveUpdateCallbacks();

    // ========== Configuration Constants ==========
    static constexpr const char* CONFIG_SECTION = ConfigConstants::Sections::OIL_TEMPERATURE_SENSOR;
    static constexpr const char* CONFIG_UNIT = ConfigConstants::Keys::OIL_TEMPERATURE_UNIT;
    static constexpr const char* CONFIG_UPDATE_RATE = ConfigConstants::Keys::OIL_TEMPERATURE_UPDATE_RATE;
    static constexpr const char* CONFIG_OFFSET = ConfigConstants::Keys::OIL_TEMPERATURE_OFFSET;
    static constexpr const char* CONFIG_SCALE = ConfigConstants::Keys::OIL_TEMPERATURE_SCALE;

  protected:
    // ========== Protected Methods ==========
    int32_t ReadRawValue();
    int32_t ConvertReading(int32_t rawValue);

  private:
    // ========== Configuration Items (inline definitions) ==========
    inline static Config::ConfigItem unitConfig_{ConfigConstants::Items::UNIT, UIStrings::ConfigLabels::TEMPERATURE_UNIT,
                                                  std::string(ConfigConstants::Defaults::DEFAULT_TEMPERATURE_UNIT),
                                                  Config::ConfigMetadata("C,F", Config::ConfigItemType::Selection)};
    inline static Config::ConfigItem updateRateConfig_{ConfigConstants::Items::UPDATE_RATE, UIStrings::ConfigLabels::UPDATE_RATE_MS,
                                                        ConfigConstants::Defaults::DEFAULT_UPDATE_RATE,
                                                        Config::ConfigMetadata("250,500,1000,2000", Config::ConfigItemType::Selection)};
    inline static Config::ConfigItem offsetConfig_{ConfigConstants::Items::OFFSET, UIStrings::ConfigLabels::CALIBRATION_OFFSET,
                                                    ConfigConstants::Defaults::DEFAULT_CALIBRATION_OFFSET,
                                                    Config::ConfigMetadata("-5.0,-2.0,-1.0,-0.5,0.0,0.5,1.0,2.0,5.0", Config::ConfigItemType::Selection)};
    inline static Config::ConfigItem scaleConfig_{ConfigConstants::Items::SCALE, UIStrings::ConfigLabels::CALIBRATION_SCALE,
                                                   ConfigConstants::Defaults::DEFAULT_CALIBRATION_SCALE,
                                                   Config::ConfigMetadata("0.9,0.95,1.0,1.05,1.1", Config::ConfigItemType::Selection)};

    // ========== Private Data Members ==========
    IGpioProvider *gpioProvider_;
    IConfigurationManager *preferenceService_ = nullptr;
    float calibrationOffset_ = 0.0f;
    float calibrationScale_ = 1.0f;
    std::string targetUnit_ = "C";
    int32_t currentReading_ = 0;
    int32_t previousReading_ = 0;  // For GetReading() change tracking
    int32_t previousChangeReading_ = 0;  // For HasStateChanged() separate tracking
    unsigned long lastUpdateTime_ = 0;
    unsigned long updateIntervalMs_;
    uint32_t configCallbackId_ = 0;  // For live update callback management
};