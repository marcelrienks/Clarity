#include "factories/manager_factory.h"
#include "utilities/types.h"
#include <esp32-hal-log.h>

// Factory Methods

std::unique_ptr<PanelManager> ManagerFactory::createPanelManager(IDisplayProvider* display, IGpioProvider* gpio)
{
    log_d("Creating PanelManager with injected dependencies");
    
    auto manager = std::make_unique<PanelManager>(display, gpio);
    manager->init();
    
    return manager;
}

std::unique_ptr<StyleManager> ManagerFactory::createStyleManager(const char* theme)
{
    log_d("Creating StyleManager with theme: %s", theme ? theme : "default");
    
    auto manager = std::make_unique<StyleManager>();
    manager->init(theme ? theme : Themes::NIGHT);
    
    return manager;
}

std::unique_ptr<TriggerManager> ManagerFactory::createTriggerManager(IGpioProvider* gpio)
{
    log_d("Creating TriggerManager with injected dependencies");
    
    auto manager = std::make_unique<TriggerManager>(gpio);
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