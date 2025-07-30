#include "main.h"
#include "factories/manager_factory.h"
#include "system/component_registry.h"
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

// Global manager instances (replacing singletons for Step 3)
std::unique_ptr<PreferenceManager> g_preferenceManager;
std::unique_ptr<StyleManager> g_styleManager;
std::unique_ptr<TriggerManager> g_triggerManager;
std::unique_ptr<PanelManager> g_panelManager;

void registerProductionComponents() {
  auto& registry = ComponentRegistry::GetInstance();
  
  // Register panels using PanelNames constants
  registry.registerPanel(PanelNames::KEY, [&registry](IGpioProvider* gpio, IDisplayProvider* display) {
    return std::make_unique<KeyPanel>(&registry);
  });
  
  registry.registerPanel(PanelNames::LOCK, [&registry](IGpioProvider* gpio, IDisplayProvider* display) {
    return std::make_unique<LockPanel>(&registry);
  });
  
  registry.registerPanel(PanelNames::SPLASH, [&registry](IGpioProvider* gpio, IDisplayProvider* display) {
    return std::make_unique<SplashPanel>(&registry);
  });
  
  registry.registerPanel(PanelNames::OIL, [&registry](IGpioProvider* gpio, IDisplayProvider* display) {
    return std::make_unique<OemOilPanel>(&registry);
  });
  
  // Register components
  registry.registerComponent("key", [](IDisplayProvider* display, IStyleService* style) {
    return std::make_unique<KeyComponent>(style);
  });
  
  registry.registerComponent("lock", [](IDisplayProvider* display, IStyleService* style) {
    return std::make_unique<LockComponent>(style);
  });
  
  registry.registerComponent("clarity", [](IDisplayProvider* display, IStyleService* style) {
    return std::make_unique<ClarityComponent>(style);
  });
  
  
  registry.registerComponent("oem_oil_pressure", [](IDisplayProvider* display, IStyleService* style) {
    return std::make_unique<OemOilPressureComponent>(style);
  });
  
  registry.registerComponent("oem_oil_temperature", [](IDisplayProvider* display, IStyleService* style) {
    return std::make_unique<OemOilTemperatureComponent>(style);
  });
}

void setup()
{
  log_d("Starting Clarity application setup - using factory pattern");

  // Get the component registry for panel/component registration and DI
  auto& registry = ComponentRegistry::GetInstance();
  
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
  g_panelManager = ManagerFactory::createPanelManager(displayProvider, gpioProvider, &registry);

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

