#pragma once

#include <memory>
#include <string>
#include <functional>

#include "interfaces/i_sensor.h"

using SensorFactoryFunction = std::function<std::unique_ptr<ISensor>()>;

/**
 * @interface ISensorFactory
 * @brief Interface for sensor creation and management
 */
class ISensorFactory {
public:
    virtual ~ISensorFactory() = default;

    virtual void registerSensor(const std::string& name, SensorFactoryFunction factory) = 0;
    virtual std::unique_ptr<ISensor> createSensor(const std::string& name) = 0;
    virtual bool hasSensorRegistration(const std::string& name) const = 0;
    virtual void clear() = 0;
};
