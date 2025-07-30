#include "main.h"
#include "factories/manager_factory.h"

// Global manager instances (replacing singletons for Step 3)
std::unique_ptr<PreferenceManager> g_preferenceManager;
std::unique_ptr<StyleManager> g_styleManager;
std::unique_ptr<TriggerManager> g_triggerManager;
std::unique_ptr<PanelManager> g_panelManager;

void setup()
{
  log_d("Starting Clarity application setup - using factory pattern");

  // Create managers using factory pattern
  g_preferenceManager = ManagerFactory::createPreferenceManager();

  Device::GetInstance().prepare();
  Ticker::handle_lv_tasks();

  g_styleManager = ManagerFactory::createStyleManager(Themes::DAY);
  Ticker::handle_lv_tasks();

  // TODO: Replace with actual GPIO provider from Device in Step 4 of roadmap
  g_triggerManager = ManagerFactory::createTriggerManager(nullptr);

  // TODO: Replace with actual providers from Device in Step 4 of roadmap
  g_panelManager = ManagerFactory::createPanelManager(nullptr, nullptr);

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

