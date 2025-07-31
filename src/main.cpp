#include "main.h"
#include "clarity_application.h"
#include "system/service_container.h"
#include "factories/component_factory.h"
#include "factories/panel_factory.h"
#include "managers/preference_manager.h"
#include "managers/style_manager.h"
#include "managers/trigger_manager.h"
#include "managers/panel_manager.h"
#include "providers/esp32_gpio_provider.h"
#include "providers/lvgl_display_provider.h"
#include "utilities/types.h"
#include "components/key_component.h"
#include "components/lock_component.h"
#include "components/clarity_component.h"
#include "components/oem/oem_oil_component.h"
#include "components/oem/oem_oil_pressure_component.h"
#include "components/oem/oem_oil_temperature_component.h"

// Service container and application
std::unique_ptr<ServiceContainer> g_serviceContainer;
std::unique_ptr<ClarityApplication> g_application;

// Legacy global service pointer for backward compatibility 
// TODO Sprint 6: Remove when all legacy components use proper dependency injection
IStyleService* g_styleService = nullptr;

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

void setup()
{
  log_d("Starting Clarity application setup - using dependency injection container");

  // Create service container
  g_serviceContainer = std::make_unique<ServiceContainer>();

  // Initialize hardware first
  Device::GetInstance().prepare();
  Ticker::handle_lv_tasks();

  // Register hardware providers
  g_serviceContainer->registerSingleton<IGpioProvider>([]() {
    return std::make_unique<Esp32GpioProvider>();
  });

  g_serviceContainer->registerSingleton<IDisplayProvider>([&]() {
    auto& device = Device::GetInstance();
    return std::make_unique<LvglDisplayProvider>(device.screen);
  });

  // Register managers as services (direct interface implementation)
  g_serviceContainer->registerSingleton<IStyleService>([&]() {
    auto manager = std::make_unique<StyleManager>();
    manager->init(Themes::DAY);
    return std::unique_ptr<IStyleService>(std::move(manager));
  });

  g_serviceContainer->registerSingleton<IPreferenceService>([]() {
    auto manager = std::make_unique<PreferenceManager>();
    manager->init();
    return std::unique_ptr<IPreferenceService>(std::move(manager));
  });

  // Register component factory
  g_serviceContainer->registerSingleton<IComponentFactory>([&]() {
    auto factory = std::make_unique<ComponentFactory>(
      g_serviceContainer->resolve<IStyleService>(),
      g_serviceContainer->resolve<IDisplayProvider>()
    );
    
    // Register production components
    registerProductionComponents(factory.get());
    
    return std::unique_ptr<IComponentFactory>(std::move(factory));
  });

  // Register panel factory
  g_serviceContainer->registerSingleton<IPanelFactory>([&]() {
    return std::make_unique<PanelFactory>(
      g_serviceContainer->resolve<IComponentFactory>(),
      g_serviceContainer->resolve<IDisplayProvider>(),
      g_serviceContainer->resolve<IGpioProvider>()
    );
  });

  g_serviceContainer->registerSingleton<IPanelService>([&]() {
    return std::unique_ptr<IPanelService>(std::make_unique<PanelManager>(
      g_serviceContainer->resolve<IDisplayProvider>(),
      g_serviceContainer->resolve<IGpioProvider>(),
      g_serviceContainer->resolve<IPanelFactory>()
    ));
  });

  g_serviceContainer->registerSingleton<ITriggerService>([&]() {
    return std::unique_ptr<ITriggerService>(std::make_unique<TriggerManager>(
      g_serviceContainer->resolve<IGpioProvider>(),
      g_serviceContainer->resolve<IPanelService>(),
      g_serviceContainer->resolve<IStyleService>()
    ));
  });

  // Create application with injected service interfaces (Step 5.1 complete)
  g_application = std::make_unique<ClarityApplication>(
    g_serviceContainer->resolve<IPanelService>(),    // PanelManager as IPanelService
    g_serviceContainer->resolve<ITriggerService>(),  // TriggerManager as ITriggerService
    g_serviceContainer->resolve<IPreferenceService>() // PreferenceManager as IPreferenceService
  );
  
  // Set legacy global service pointer for backward compatibility
  g_styleService = g_serviceContainer->resolve<IStyleService>();
  
  // Initialize the application
  g_application->initialize();
  
  Ticker::handle_lv_tasks();
}

void loop()
{
  // Application loop with dependency injection (Step 5.2 complete)
  g_application->update();
}

