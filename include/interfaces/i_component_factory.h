#pragma once
#include <memory>

class IStyleService;
class IComponent;
class ClarityComponent;
class OemOilPressureComponent;
class OemOilTemperatureComponent;
class ErrorComponent;
class KeyComponent;
class LockComponent;
class ConfigComponent;

class IComponentFactory {
public:
    virtual ~IComponentFactory() = default;
    virtual std::unique_ptr<ClarityComponent> CreateClarityComponent(IStyleService* style) = 0;
    virtual std::unique_ptr<IComponent> CreateOilPressureComponent(IStyleService* style) = 0;
    virtual std::unique_ptr<IComponent> CreateOilTemperatureComponent(IStyleService* style) = 0;
    virtual std::unique_ptr<ErrorComponent> CreateErrorComponent(IStyleService* style) = 0;
    virtual std::unique_ptr<KeyComponent> CreateKeyComponent(IStyleService* style) = 0;
    virtual std::unique_ptr<LockComponent> CreateLockComponent(IStyleService* style) = 0;
    virtual std::unique_ptr<ConfigComponent> CreateConfigComponent(IStyleService* style) = 0;
};