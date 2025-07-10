#include "main.h"

void setup()
{
  log_d("...");

  PreferenceManager &preference_manager = PreferenceManager::get_instance();
  preference_manager.init();

  Device::get_instance().prepare();
  Ticker::handle_lv_tasks();

  StyleManager::get_instance().init(Themes::Day);
  Ticker::handle_lv_tasks();

  PanelManager &panel_manager = PanelManager::get_instance();
  panel_manager.init();

  // panel_manager.create_and_load_panel_with_splash(preference_manager.config.panel_name.c_str());
  panel_manager.create_and_load_panel(preference_manager.config.panel_name.c_str());
  // panel_manager.create_and_load_panel(PanelNames::Key);
  
  Ticker::handle_lv_tasks();
}

void loop()
{
  log_d("...");

  PanelManager::get_instance().update_panel();
  Ticker::handle_lv_tasks();

  Ticker::handle_dynamic_delay(millis());
}