#include "main.h"

void setup() {
  try {
    log_d("...");

    PreferenceManager &preference_manager = PreferenceManager::get_instance();
    preference_manager.init();

    Device::get_instance().prepare();
    Ticker::handle_lv_tasks();

    StyleManager::get_instance().init(Themes::Day);
    Ticker::handle_lv_tasks();

    PanelManager &panel_manager = PanelManager::get_instance();
    panel_manager.init();
    //panel_manager.load_panel_with_Splash(preference_manager.config.panel_name.c_str());
    panel_manager.load_panel(preference_manager.config.panel_name.c_str());
    //panel_manager.load_panel(PanelNames::Key);
    Ticker::handle_lv_tasks();
  }
  catch (const std::exception &e) {
    log_e("Exception in setup: %s", e.what());
    throw;
  }
  catch (...) {
    log_e("Unknown exception occurred in setup");
    throw;
  }
}

void loop()
{
  try
  {
    log_d("...");

    PanelManager::get_instance().refresh_panel();
    Ticker::handle_lv_tasks();

    Ticker::handle_dynamic_delay(millis());
  }
  catch (const std::exception &e)
  {
    log_e("Exception in loop: %s", e.what());
    throw;
  }
  catch (...)
  {
    log_e("Unknown exception occurred in loop");
    throw;
  }
}