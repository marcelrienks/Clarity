#pragma once

#include <memory>
#include "interfaces/i_component_factory.h"

class IStyleService;
class ClarityComponent;
class OilPressureComponent;
class OilTemperatureComponent;
class ErrorComponent;
class KeyComponent;
class LockComponent;
class ConfigComponent;

class ComponentFactory : public IComponentFactory {
public:
    static ComponentFactory& Instance();
    
    // IComponentFactory implementation
    std::unique_ptr<ClarityComponent> CreateClarityComponent(IStyleService* style) override;
    std::unique_ptr<IComponent> CreateOilPressureComponent(IStyleService* style) override;
    std::unique_ptr<IComponent> CreateOilTemperatureComponent(IStyleService* style) override;
    std::unique_ptr<ErrorComponent> CreateErrorComponent(IStyleService* style) override;
    std::unique_ptr<KeyComponent> CreateKeyComponent(IStyleService* style) override;
    std::unique_ptr<LockComponent> CreateLockComponent(IStyleService* style) override;
    std::unique_ptr<ConfigComponent> CreateConfigComponent(IStyleService* style) override;
    
private:
    ComponentFactory() = default;
    ~ComponentFactory() = default;
    ComponentFactory(const ComponentFactory&) = delete;
    ComponentFactory& operator=(const ComponentFactory&) = delete;
};