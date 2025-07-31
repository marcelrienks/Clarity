#include "clarity_bootstrap.h"
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

void ClarityBootstrap::registerProductionComponents(IComponentFactory* componentFactory) {
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

void ClarityBootstrap::registerServices() {
    // Register device service
    serviceContainer_->registerSingleton<IDevice>([]() {
        return std::make_unique<Device>();
    });

    // Register hardware providers directly
    serviceContainer_->registerSingleton<IGpioProvider>([]() {
        return std::make_unique<Esp32GpioProvider>();
    });

    serviceContainer_->registerSingleton<IDisplayProvider>([this]() {
        auto device = serviceContainer_->resolve<IDevice>();
        // Cast IDevice to Device to access screen member
        Device* deviceImpl = static_cast<Device*>(device);
        return std::make_unique<LvglDisplayProvider>(deviceImpl->screen);
    });

    // Register managers as services (direct interface implementation)
    serviceContainer_->registerSingleton<IStyleService>([]() {
        auto manager = std::make_unique<StyleManager>();
        manager->init(Themes::DAY);
        return std::unique_ptr<IStyleService>(std::move(manager));
    });

    serviceContainer_->registerSingleton<IPreferenceService>([]() {
        auto manager = std::make_unique<PreferenceManager>();
        manager->init();
        return std::unique_ptr<IPreferenceService>(std::move(manager));
    });

    // Register component factory
    serviceContainer_->registerSingleton<IComponentFactory>([this]() {
        auto factory = std::make_unique<ComponentFactory>(
            serviceContainer_->resolve<IStyleService>(),
            serviceContainer_->resolve<IDisplayProvider>()
        );
        
        // Register production components
        registerProductionComponents(factory.get());
        
        return std::unique_ptr<IComponentFactory>(std::move(factory));
    });

    // Register panel factory
    serviceContainer_->registerSingleton<IPanelFactory>([this]() {
        return std::make_unique<PanelFactory>(
            serviceContainer_->resolve<IComponentFactory>(),
            serviceContainer_->resolve<IDisplayProvider>(),
            serviceContainer_->resolve<IGpioProvider>()
        );
    });

    serviceContainer_->registerSingleton<IPanelService>([this]() {
        return std::unique_ptr<IPanelService>(std::make_unique<PanelManager>(
            serviceContainer_->resolve<IDisplayProvider>(),
            serviceContainer_->resolve<IGpioProvider>(),
            serviceContainer_->resolve<IPanelFactory>()
        ));
    });

    serviceContainer_->registerSingleton<ITriggerService>([this]() {
        return std::unique_ptr<ITriggerService>(std::make_unique<TriggerManager>(
            serviceContainer_->resolve<IGpioProvider>(),
            serviceContainer_->resolve<IPanelService>(),
            serviceContainer_->resolve<IStyleService>()
        ));
    });
}

void ClarityBootstrap::initialize() {
    log_d("Starting Clarity application setup - using dependency injection container");

    // Create service container
    serviceContainer_ = std::make_unique<ServiceContainer>();

    // Register all services
    registerServices();

    // Initialize hardware through DI container
    auto device = serviceContainer_->resolve<IDevice>();
    device->prepare();
    Ticker::handleLvTasks();

    // Create application with injected service interfaces
    application_ = std::make_unique<ClarityApplication>(
        serviceContainer_->resolve<IPanelService>(),    // PanelManager as IPanelService
        serviceContainer_->resolve<ITriggerService>(),  // TriggerManager as ITriggerService
        serviceContainer_->resolve<IPreferenceService>() // PreferenceManager as IPreferenceService
    );
    
    // Initialize the application
    application_->initialize();
    
    Ticker::handleLvTasks();
}

void ClarityBootstrap::run() {
    // Application loop with dependency injection
    if (application_) {
        application_->update();
    }
}

void ClarityBootstrap::shutdown() {
    // Clean shutdown - reset in reverse order of creation
    if (application_) {
        application_.reset();
    }
    if (serviceContainer_) {
        serviceContainer_.reset();
    }
}