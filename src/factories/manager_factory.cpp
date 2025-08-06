#include "factories/manager_factory.h"
#include "utilities/types.h"
#include "managers/panel_manager.h"
#include "managers/style_manager.h"
#include "managers/trigger_manager.h"
#include "managers/preference_manager.h"
#include "sensors/key_sensor.h"
#include "sensors/lock_sensor.h"
#include "sensors/light_sensor.h"
#include <esp32-hal-log.h>

// Factory Methods

std::unique_ptr<PanelManager> ManagerFactory::createPanelManager(IDisplayProvider *display, IGpioProvider *gpio, IStyleService *styleService)
{
    log_d("Creating PanelManager with injected dependencies");
    
    if (!display) {
        throw std::invalid_argument("ManagerFactory::createPanelManager requires valid IDisplayProvider");
    }
    if (!gpio) {
        throw std::invalid_argument("ManagerFactory::createPanelManager requires valid IGpioProvider");
    }
    if (!styleService) {
        throw std::invalid_argument("ManagerFactory::createPanelManager requires valid IStyleService");
    }
    
    // Create PanelManager with direct dependencies - no factory needed
    auto manager = std::make_unique<PanelManager>(display, gpio, styleService);
    manager->init();
    
    return manager;
}

std::unique_ptr<StyleManager> ManagerFactory::createStyleManager(const char *theme)
{
    log_d("Creating StyleManager with theme: %s", theme ? theme : "default");
    
    auto manager = std::make_unique<StyleManager>();
    manager->Init(theme ? theme : Themes::NIGHT);
    
    return manager;
}

std::unique_ptr<TriggerManager> ManagerFactory::createTriggerManager(IGpioProvider *gpio, IPanelService *panelService, IStyleService *styleService)
{
    log_d("Creating TriggerManager with injected sensor dependencies");
    
    if (!gpio) {
        throw std::invalid_argument("ManagerFactory::createTriggerManager requires valid IGpioProvider");
    }
    if (!panelService) {
        throw std::invalid_argument("ManagerFactory::createTriggerManager requires valid IPanelService");
    }
    if (!styleService) {
        throw std::invalid_argument("ManagerFactory::createTriggerManager requires valid IStyleService");
    }
    
    // Create sensors that TriggerManager needs
    auto keySensor = std::make_shared<KeySensor>(gpio);
    auto lockSensor = std::make_shared<LockSensor>(gpio);
    auto lightSensor = std::make_shared<LightSensor>(gpio);
    
    auto manager = std::make_unique<TriggerManager>(keySensor, lockSensor, lightSensor, panelService, styleService);
    manager->init();
    
    return manager;
}

std::unique_ptr<PreferenceManager> ManagerFactory::createPreferenceManager()
{
    log_d("Creating PreferenceManager");
    
    auto manager = std::make_unique<PreferenceManager>();
    manager->init();
    
    return manager;
}