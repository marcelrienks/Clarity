#include "main.h"

void setup()
{
  log_d("...");

  PreferenceManager &preference_manager = PreferenceManager::GetInstance();
  preference_manager.init();

  Device::GetInstance().prepare();
  Ticker::handle_lv_tasks();

  StyleManager::GetInstance().init(Themes::DAY);
  Ticker::handle_lv_tasks();

  PanelManager &panel_manager = PanelManager::GetInstance();
  panel_manager.init();

  panel_manager.CreateAndLoadPanelWithSplash(preference_manager.config.panelName.c_str());
  
  Ticker::handle_lv_tasks();
}

void loop()
{
  log_d("...");

  // Core 0 responsibilities: UI updates and trigger message processing
  PanelManager::GetInstance().UpdatePanel(); // Now includes trigger message processing
  Ticker::handle_lv_tasks();

  Ticker::handle_dynamic_delay(millis());
}