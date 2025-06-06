#include "main.h"

void setup() {
  try {
    log_d("...");

    //PreferenceManager preference_manager = PreferenceManager::get_instance();
    //preference_manager.init();

    Device::get_instance().prepare();
    Ticker::handle_lv_tasks();

    StyleManager::get_instance().init(Themes::Day);
    Ticker::handle_lv_tasks();

    PanelManager &panel_manager = PanelManager::get_instance();
    panel_manager.init();
    panel_manager.load_panel_with_Splash(PanelNames::Demo);
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
    
    // Process any pending LVGL tasks
    Ticker::handle_lv_tasks();

    PanelManager::get_instance().refresh_panel();

    // Process LVGL tasks again to render the changes immediately
    Ticker::handle_lv_tasks();

    // Adaptive Timing to generate a ~60fps refresh rate
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