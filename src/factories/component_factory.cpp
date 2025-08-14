#include "factories/component_factory.h"
#include "components/clarity_component.h"
#include "components/oem/oem_oil_pressure_component.h"
#include "components/oem/oem_oil_temperature_component.h"
#include "components/error_component.h"
#include "components/key_component.h"
#include "components/lock_component.h"
#include "components/config_component.h"
#include "interfaces/i_style_service.h"

#ifdef CLARITY_DEBUG
    #include "esp32-hal-log.h"
    #define LOG_TAG "ComponentFactory"
#else
    #define log_d(...)
    #define log_e(...)
#endif

ComponentFactory& ComponentFactory::Instance()
{
    static ComponentFactory instance;
    return instance;
}

std::unique_ptr<ClarityComponent> ComponentFactory::CreateClarityComponent(IStyleService* style)
{
    log_d("Creating ClarityComponent");
    return std::make_unique<ClarityComponent>(style);
}

std::unique_ptr<IComponent> ComponentFactory::CreateOilPressureComponent(IStyleService* style)
{
    log_d("Creating OilPressureComponent");
    return std::make_unique<OemOilPressureComponent>(style);
}

std::unique_ptr<IComponent> ComponentFactory::CreateOilTemperatureComponent(IStyleService* style)
{
    log_d("Creating OilTemperatureComponent");
    return std::make_unique<OemOilTemperatureComponent>(style);
}

std::unique_ptr<ErrorComponent> ComponentFactory::CreateErrorComponent(IStyleService* style)
{
    log_d("Creating ErrorComponent");
    return std::make_unique<ErrorComponent>(style);
}

std::unique_ptr<KeyComponent> ComponentFactory::CreateKeyComponent(IStyleService* style)
{
    log_d("Creating KeyComponent");
    return std::make_unique<KeyComponent>(style);
}

std::unique_ptr<LockComponent> ComponentFactory::CreateLockComponent(IStyleService* style)
{
    log_d("Creating LockComponent");
    return std::make_unique<LockComponent>(style);
}

std::unique_ptr<ConfigComponent> ComponentFactory::CreateConfigComponent(IStyleService* style)
{
    log_d("Creating ConfigComponent");
    // ConfigComponent doesn't use style service in constructor
    return std::make_unique<ConfigComponent>();
}