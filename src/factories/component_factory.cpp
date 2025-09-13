#include "factories/component_factory.h"
#include "components/clarity_component.h"
#include "components/oem/oem_oil_pressure_component.h"
#include "components/oem/oem_oil_temperature_component.h"
#include "components/error_component.h"
#include "components/key_component.h"
#include "components/lock_component.h"
#include "components/config_component.h"
#include "interfaces/i_style_service.h"

#include "esp32-hal-log.h"

ComponentFactory& ComponentFactory::Instance()
{
    static ComponentFactory instance;
    return instance;
}

std::unique_ptr<ClarityComponent> ComponentFactory::CreateClarityComponent(IStyleService* style)
{
    auto component = std::make_unique<ClarityComponent>(style);
    if (!component) {
        log_e("Failed to create ClarityComponent - allocation failed");
        return nullptr;
    }
    return component;
}

std::unique_ptr<IComponent> ComponentFactory::CreateOilPressureComponent(IStyleService* style)
{
    auto component = std::make_unique<OemOilPressureComponent>(style);
    if (!component) {
        log_e("Failed to create OemOilPressureComponent - allocation failed");
        return nullptr;
    }
    return component;
}

std::unique_ptr<IComponent> ComponentFactory::CreateOilTemperatureComponent(IStyleService* style)
{
    auto component = std::make_unique<OemOilTemperatureComponent>(style);
    if (!component) {
        log_e("Failed to create OemOilTemperatureComponent - allocation failed");
        return nullptr;
    }
    return component;
}

std::unique_ptr<ErrorComponent> ComponentFactory::CreateErrorComponent(IStyleService* style)
{
    auto component = std::make_unique<ErrorComponent>(style);
    if (!component) {
        log_e("Failed to create ErrorComponent - allocation failed");
        return nullptr;
    }
    return component;
}

std::unique_ptr<KeyComponent> ComponentFactory::CreateKeyComponent(IStyleService* style)
{
    auto component = std::make_unique<KeyComponent>(style);
    if (!component) {
        log_e("Failed to create KeyComponent - allocation failed");
        return nullptr;
    }
    return component;
}

std::unique_ptr<LockComponent> ComponentFactory::CreateLockComponent(IStyleService* style)
{
    auto component = std::make_unique<LockComponent>(style);
    if (!component) {
        log_e("Failed to create LockComponent - allocation failed");
        return nullptr;
    }
    return component;
}

std::unique_ptr<ConfigComponent> ComponentFactory::CreateConfigComponent(IStyleService* style)
{
    // ConfigComponent doesn't use style service in constructor
    auto component = std::make_unique<ConfigComponent>();
    if (!component) {
        log_e("Failed to create ConfigComponent - allocation failed");
        return nullptr;
    }
    return component;
}