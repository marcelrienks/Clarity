#include "factories/manager_factory.h"
#include "factories/panel_factory.h"
#include "utilities/types.h"
#include "managers/panel_manager.h"
#include "managers/style_manager.h"
#include "managers/trigger_manager.h"
#include "managers/preference_manager.h"
#include <esp32-hal-log.h>

// Factory Methods

std::unique_ptr<PanelManager> ManagerFactory::createPanelManager(IDisplayProvider* display, IGpioProvider* gpio, IComponentFactory* componentFactory)
{
    log_d("Creating PanelManager with injected dependencies");
    
    if (!display) {
        throw std::invalid_argument("ManagerFactory::createPanelManager requires valid IDisplayProvider");
    }
    if (!gpio) {
        throw std::invalid_argument("ManagerFactory::createPanelManager requires valid IGpioProvider");
    }
    if (!componentFactory) {
        throw std::invalid_argument("ManagerFactory::createPanelManager requires valid IComponentFactory");
    }
    
    // Create the PanelFactory with injected dependencies
    auto panelFactory = std::make_unique<PanelFactory>(componentFactory, display, gpio);
    
    // Create PanelManager with the PanelFactory
    // Note: We need to manage the PanelFactory lifetime. For now, we'll use a static instance
    // This will be improved in later steps when we have a proper service container
    static std::unique_ptr<PanelFactory> g_panelFactory;
    g_panelFactory = std::move(panelFactory);
    
    auto manager = std::make_unique<PanelManager>(display, gpio, g_panelFactory.get());
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

std::unique_ptr<TriggerManager> ManagerFactory::createTriggerManager(IGpioProvider* gpio, IPanelService* panelService, IStyleService* styleService)
{
    log_d("Creating TriggerManager with injected dependencies");
    
    if (!gpio) {
        throw std::invalid_argument("ManagerFactory::createTriggerManager requires valid IGpioProvider");
    }
    if (!panelService) {
        throw std::invalid_argument("ManagerFactory::createTriggerManager requires valid IPanelService");
    }
    if (!styleService) {
        throw std::invalid_argument("ManagerFactory::createTriggerManager requires valid IStyleService");
    }
    
    auto manager = std::make_unique<TriggerManager>(gpio, panelService, styleService);
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