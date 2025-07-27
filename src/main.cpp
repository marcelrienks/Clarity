#include "main.h"

void setup()
{
  log_d("Starting Clarity application setup - initializing managers and display");

  PreferenceManager &preference_manager = PreferenceManager::GetInstance();
  preference_manager.init();

  Device::GetInstance().prepare();
  Ticker::handle_lv_tasks();

  StyleManager::GetInstance().init(Themes::DAY);
  Ticker::handle_lv_tasks();

  TriggerManager &trigger_manager = TriggerManager::GetInstance();
  trigger_manager.init();

  PanelManager &panel_manager = PanelManager::GetInstance();
  panel_manager.init();

  // Check if startup triggers require a specific panel, otherwise use config default
  const char* startupPanel = trigger_manager.GetStartupPanelOverride();
  if (startupPanel) {
    log_i("Using startup panel override: %s", startupPanel);
    panel_manager.CreateAndLoadPanelWithSplash(startupPanel);
    
  } else {
    log_i("Using config default panel: %s", preference_manager.config.panelName.c_str());
    panel_manager.CreateAndLoadPanelWithSplash(preference_manager.config.panelName.c_str());
  }
  
  Ticker::handle_lv_tasks();
}

void loop()
{
  log_d("Processing main loop - updating triggers and panels");

  // Core 0 responsibilities: process trigger events directly (simplified)
  TriggerManager::GetInstance().ProcessTriggerEvents();
  
  PanelManager::GetInstance().UpdatePanel();
  Ticker::handle_lv_tasks();
  Ticker::handle_dynamic_delay(millis());
}

