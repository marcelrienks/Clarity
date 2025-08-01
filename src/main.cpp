#include "main.h"
#include "device.h"
#include "factories/component_factory.h"
#include "factories/panel_factory.h"
#include "managers/preference_manager.h"
#include "managers/style_manager.h"
#include "managers/trigger_manager.h"
#include "managers/panel_manager.h"
#include "providers/esp32_gpio_provider.h"
#include "providers/lvgl_display_provider.h"
#include "utilities/types.h"
#include "utilities/ticker.h"
#include "components/key_component.h"
#include "components/lock_component.h"
#include "components/clarity_component.h"
#include "components/oem/oem_oil_component.h"
#include "components/oem/oem_oil_pressure_component.h"
#include "components/oem/oem_oil_temperature_component.h"

// Global service container and services
std::unique_ptr<ServiceContainer> serviceContainer;
IPanelService* panelService = nullptr;
ITriggerService* triggerService = nullptr;
IPreferenceService* preferenceService = nullptr;

void registerProductionComponents(IComponentFactory* componentFactory) {
    // Register components with the factory
    componentFactory->registerComponent("key", [](IDisplayProvider* display, IStyleService* style) {
        return std::make_unique<KeyComponent>(style);
    });
    
    componentFactory->registerComponent("lock", [](IDisplayProvider* display, IStyleService* style) {
        return std::make_unique<LockComponent>(style);
    });
    
    componentFactory->registerComponent("clarity", [](IDisplayProvider* display, IStyleService* style) {
        return std::make_unique<ClarityComponent>(style);
    });
    
    componentFactory->registerComponent("oem_oil_pressure", [](IDisplayProvider* display, IStyleService* style) {
        return std::make_unique<OemOilPressureComponent>(style);
    });
    
    componentFactory->registerComponent("oem_oil_temperature", [](IDisplayProvider* display, IStyleService* style) {
        return std::make_unique<OemOilTemperatureComponent>(style);
    });
}

void registerServices() {
    // Register device service
    serviceContainer->registerSingleton<IDevice>([]() {
        return std::make_unique<Device>();
    });

    // Register hardware providers directly
    serviceContainer->registerSingleton<IGpioProvider>([]() {
        return std::make_unique<Esp32GpioProvider>();
    });

    serviceContainer->registerSingleton<IDisplayProvider>([&]() {
        auto device = serviceContainer->resolve<IDevice>();
        // Cast IDevice to Device to access screen member
        Device* deviceImpl = static_cast<Device*>(device);
        return std::make_unique<LvglDisplayProvider>(deviceImpl->screen);
    });

    // Register managers as services
    serviceContainer->registerSingleton<IStyleService>([]() {
        auto manager = std::make_unique<StyleManager>();
        manager->init(Themes::DAY);
        return std::unique_ptr<IStyleService>(std::move(manager));
    });

    serviceContainer->registerSingleton<IPreferenceService>([]() {
        auto manager = std::make_unique<PreferenceManager>();
        manager->init();
        return std::unique_ptr<IPreferenceService>(std::move(manager));
    });

    // Register component factory
    serviceContainer->registerSingleton<IComponentFactory>([&]() {
        auto factory = std::make_unique<ComponentFactory>(
            serviceContainer->resolve<IStyleService>(),
            serviceContainer->resolve<IDisplayProvider>()
        );
        
        // Register production components
        registerProductionComponents(factory.get());
        
        return std::unique_ptr<IComponentFactory>(std::move(factory));
    });

    // Register panel factory
    serviceContainer->registerSingleton<IPanelFactory>([&]() {
        return std::make_unique<PanelFactory>(
            serviceContainer->resolve<IComponentFactory>(),
            serviceContainer->resolve<IDisplayProvider>(),
            serviceContainer->resolve<IGpioProvider>()
        );
    });

    serviceContainer->registerSingleton<IPanelService>([&]() {
        return std::unique_ptr<IPanelService>(std::make_unique<PanelManager>(
            serviceContainer->resolve<IDisplayProvider>(),
            serviceContainer->resolve<IGpioProvider>(),
            serviceContainer->resolve<IPanelFactory>()
        ));
    });

    serviceContainer->registerSingleton<ITriggerService>([&]() {
        return std::unique_ptr<ITriggerService>(std::make_unique<TriggerManager>(
            serviceContainer->resolve<IGpioProvider>(),
            serviceContainer->resolve<IPanelService>(),
            serviceContainer->resolve<IStyleService>()
        ));
    });
}

void setup()
{
    log_d("Starting Clarity application setup - using dependency injection container");

    // Create service container
    serviceContainer = std::make_unique<ServiceContainer>();

    // Register all services
    registerServices();

    // Initialize hardware through DI container
    auto device = serviceContainer->resolve<IDevice>();
    device->prepare();
    Ticker::handleLvTasks();

    // Resolve services
    panelService = serviceContainer->resolve<IPanelService>();
    triggerService = serviceContainer->resolve<ITriggerService>();
    preferenceService = serviceContainer->resolve<IPreferenceService>();
    
    // Initialize application
    log_d("Initializing Clarity application");
    
    // Initialize trigger service after all dependencies are resolved
    triggerService->init();

    Ticker::handleLvTasks();

    // Check if startup triggers require a specific panel, otherwise use config default
    const char* startupPanel = triggerService->getStartupPanelOverride();
    if (startupPanel) {
        log_i("Using startup panel override: %s", startupPanel);
        panelService->createAndLoadPanelWithSplash(startupPanel);
    } else {
        auto config = preferenceService->getConfig();
        log_i("Using config default panel: %s", config.panelName.c_str());
        panelService->createAndLoadPanelWithSplash(config.panelName.c_str());
    }
    
    Ticker::handleLvTasks();
}

void loop()
{
    // Process trigger events directly
    triggerService->processTriggerEvents();
    
    panelService->updatePanel();
    Ticker::handleLvTasks();
    Ticker::handleDynamicDelay(millis());
}

