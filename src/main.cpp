#include "main.h"
#include "factories/manager_factory.h"
#include "system/component_registry.h"
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

// Global manager instances (replacing singletons for Step 3)
std::unique_ptr<PreferenceManager> g_preferenceManager;
std::unique_ptr<StyleManager> g_styleManager;
std::unique_ptr<TriggerManager> g_triggerManager;
std::unique_ptr<PanelManager> g_panelManager;

void registerProductionComponents() {
  auto& registry = ComponentRegistry::GetInstance();
  
  // Register panels
  registry.registerPanel("key", [](IGpioProvider* gpio, IDisplayProvider* display) {
    return std::make_unique<KeyPanel>();
  });
  
  registry.registerPanel("lock", [](IGpioProvider* gpio, IDisplayProvider* display) {
    return std::make_unique<LockPanel>();
  });
  
  registry.registerPanel("splash", [](IGpioProvider* gpio, IDisplayProvider* display) {
    return std::make_unique<SplashPanel>();
  });
  
  registry.registerPanel("oem_oil", [](IGpioProvider* gpio, IDisplayProvider* display) {
    return std::make_unique<OemOilPanel>();
  });
  
  // Register components
  registry.registerComponent("key", [](IDisplayProvider* display) {
    return std::make_unique<KeyComponent>();
  });
  
  registry.registerComponent("lock", [](IDisplayProvider* display) {
    return std::make_unique<LockComponent>();
  });
  
  registry.registerComponent("clarity", [](IDisplayProvider* display) {
    return std::make_unique<ClarityComponent>();
  });
  
  
  registry.registerComponent("oem_oil_pressure", [](IDisplayProvider* display) {
    return std::make_unique<OemOilPressureComponent>(&StyleManager::GetInstance());
  });
  
  registry.registerComponent("oem_oil_temperature", [](IDisplayProvider* display) {
    return std::make_unique<OemOilTemperatureComponent>(&StyleManager::GetInstance());
  });
}

void setup()
{
  log_d("Starting Clarity application setup - using factory pattern");

  // Register production components (Step 5)
  registerProductionComponents();

  // Create managers using factory pattern
  g_preferenceManager = ManagerFactory::createPreferenceManager();

  Device::GetInstance().prepare();
  Ticker::handle_lv_tasks();

  g_styleManager = ManagerFactory::createStyleManager(Themes::DAY);
  Ticker::handle_lv_tasks();

  // Get providers from Device (Step 4 implementation)
  auto& device = Device::GetInstance();
  IGpioProvider* gpioProvider = device.getGpioProvider();
  IDisplayProvider* displayProvider = device.getDisplayProvider();

  g_triggerManager = ManagerFactory::createTriggerManager(gpioProvider);
  g_panelManager = ManagerFactory::createPanelManager(displayProvider, gpioProvider);

  // Check if startup triggers require a specific panel, otherwise use config default
  const char* startupPanel = g_triggerManager->GetStartupPanelOverride();
  if (startupPanel) {
    log_i("Using startup panel override: %s", startupPanel);
    g_panelManager->CreateAndLoadPanelWithSplash(startupPanel);
    
  } else {
    log_i("Using config default panel: %s", g_preferenceManager->config.panelName.c_str());
    g_panelManager->CreateAndLoadPanelWithSplash(g_preferenceManager->config.panelName.c_str());
  }
  
  Ticker::handle_lv_tasks();
}

void loop()
{
  // Core 0 responsibilities: process trigger events directly (simplified)
  g_triggerManager->ProcessTriggerEvents();
  
  g_panelManager->UpdatePanel();
  Ticker::handle_lv_tasks();
  Ticker::handle_dynamic_delay(millis());
}

