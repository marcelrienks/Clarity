#include "factories/manager_factory.h"
#include "utilities/types.h"
#include "managers/panel_manager.h"
#include "managers/style_manager.h"
#include "managers/trigger_manager.h"
#include "managers/preference_manager.h"
#include "sensors/key_sensor.h"
#include "sensors/lock_sensor.h"
#include "sensors/light_sensor.h"
#include "sensors/debug_error_sensor.h"
#include "sensors/input_button_sensor.h"
#include "managers/input_manager.h"
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
    manager->Init();
    
    return manager;
}

std::unique_ptr<StyleManager> ManagerFactory::createStyleManager(const char *theme)
{
    log_d("Creating StyleManager with theme: %s", theme ? theme : "default");
    
    auto manager = std::make_unique<StyleManager>(theme ? theme : Themes::NIGHT);
    
    return manager;
}

std::unique_ptr<TriggerManager> ManagerFactory::createTriggerManager(IGpioProvider *gpio, IPanelService *panelService, IStyleService *styleService)
{
    log_i("Creating TriggerManager with sensors");
    
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
    auto debugErrorSensor = std::make_shared<DebugErrorSensor>(gpio);
    
    auto manager = std::make_unique<TriggerManager>(keySensor, lockSensor, lightSensor, debugErrorSensor, panelService, styleService);
    manager->Init();
    
    return manager;
}

std::unique_ptr<PreferenceManager> ManagerFactory::createPreferenceManager()
{
    log_d("Creating PreferenceManager");
    
    auto manager = std::make_unique<PreferenceManager>();
    manager->Init();
    
    return manager;
}

std::unique_ptr<InputManager> ManagerFactory::createInputManager(IGpioProvider* gpio, IPanelService* panelService)
{
    log_i("Creating InputManager");
    
    if (!gpio) {
        throw std::invalid_argument("ManagerFactory::createInputManager requires valid IGpioProvider");
    }
    
    if (!panelService) {
        throw std::invalid_argument("ManagerFactory::createInputManager requires valid IPanelService");
    }
    
    // Create InputButtonSensor that InputManager needs
    auto inputButtonSensor = std::make_shared<InputButtonSensor>(gpio);
    
    auto manager = std::make_unique<InputManager>(inputButtonSensor, panelService);
    manager->Init();
    
    return manager;
}