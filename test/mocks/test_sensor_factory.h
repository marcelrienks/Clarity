#pragma once

#include <memory>
#include <map>
#include "interfaces/i_sensor_factory.h"

namespace ArchitecturalTestHelpers {

class TestSensorFactory : public ISensorFactory {
public:
    void registerSensor(const std::string& name, SensorFactoryFunction factory) override {
        sensorFactories_[name] = factory;
    }

    std::unique_ptr<ISensor> createSensor(const std::string& name) override {
        auto it = sensorFactories_.find(name);
        if (it != sensorFactories_.end()) {
            return it->second();
        }
        return nullptr;
    }

    bool hasSensorRegistration(const std::string& name) const override {
        return sensorFactories_.find(name) != sensorFactories_.end();
    }

    void clear() override {
        sensorFactories_.clear();
    }

private:
    std::map<std::string, SensorFactoryFunction> sensorFactories_;
};

} // namespace ArchitecturalTestHelpers
