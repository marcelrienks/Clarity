#pragma once

#include <memory>
#include <map>
#include <vector>
#include <string>
#include "interfaces/i_sensor_factory.h"

namespace ArchitecturalTestHelpers {

class MockSensorFactory : public ISensorFactory {
public:
    void registerSensor(const std::string& name, SensorFactoryFunction factory) override {
        sensorFactories_[name] = factory;
        registrationCount_++;
    }

    std::unique_ptr<ISensor> createSensor(const std::string& name) override {
        auto it = sensorFactories_.find(name);
        if (it != sensorFactories_.end()) {
            creationCount_++;
            return it->second();
        }
        return nullptr;
    }

    bool hasSensorRegistration(const std::string& name) const override {
        return sensorFactories_.find(name) != sensorFactories_.end();
    }

    void clear() override {
        sensorFactories_.clear();
        clearCalled_ = true;
    }

    // Test Helper Methods
    int getRegistrationCount() const { return registrationCount_; }
    int getCreationCount() const { return creationCount_; }
    bool wasClearCalled() const { return clearCalled_; }
    
    void reset() {
        sensorFactories_.clear();
        registrationCount_ = 0;
        creationCount_ = 0;
        clearCalled_ = false;
    }

private:
    std::map<std::string, SensorFactoryFunction> sensorFactories_;
    int registrationCount_ = 0;
    int creationCount_ = 0;
    bool clearCalled_ = false;
};

} // namespace ArchitecturalTestHelpers
