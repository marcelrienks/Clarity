#include "main.h"
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
#include "panels/key_panel.h"
#include "panels/lock_panel.h"
#include "panels/splash_panel.h"
#include "panels/oem_oil_panel.h"
#include "components/key_component.h"
#include "components/lock_component.h"
#include "components/clarity_component.h"
#include "components/oem/oem_oil_component.h"
#include "components/oem/oem_oil_pressure_component.h"
#include "components/oem/oem_oil_temperature_component.h"

// Service container and application state
std::unique_ptr<ServiceContainer> g_serviceContainer;
IPanelService* g_panelService = nullptr;
ITriggerService* g_triggerService = nullptr;
IPreferenceService* g_preferenceService = nullptr;

// Step 4.5: Global service pointers for legacy code compatibility
// TODO: Remove these when all components use proper dependency injection
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

  // Resolve services for use in loop()
  g_panelService = g_serviceContainer->resolve<IPanelService>();
  g_triggerService = g_serviceContainer->resolve<ITriggerService>();
  g_preferenceService = g_serviceContainer->resolve<IPreferenceService>();
  
  // Step 4.5: Set legacy global service pointers for backward compatibility
  g_styleService = g_serviceContainer->resolve<IStyleService>();
  
  // Initialize trigger service after all dependencies are resolved
  g_triggerService->init();

  // Step 4.5: Removed singleton GetInstance() methods and global manager instances
  // All services now accessed through dependency injection container

  Ticker::handle_lv_tasks();

  // Check if startup triggers require a specific panel, otherwise use config default
  const char* startupPanel = g_triggerService->getStartupPanelOverride();
  if (startupPanel) {
    log_i("Using startup panel override: %s", startupPanel);
    g_panelService->createAndLoadPanelWithSplash(startupPanel);
  } else {
    auto config = g_preferenceService->getConfig();
    log_i("Using config default panel: %s", config.panelName.c_str());
    g_panelService->createAndLoadPanelWithSplash(config.panelName.c_str());
  }
  
  Ticker::handle_lv_tasks();
}

void loop()
{
  // Core 0 responsibilities: process trigger events directly (simplified)
  g_triggerService->processTriggerEvents();
  
  g_panelService->updatePanel();
  Ticker::handle_lv_tasks();
  Ticker::handle_dynamic_delay(millis());
}

