#include "main.h"

void setup() {
  try {
    log_d("...");

    // Initialise
    Device::get_instance().prepare();

    PreferenceManager preference_manager = PreferenceManager::get_instance();
    preference_manager.init();

    StyleManager::get_instance().init(preference_manager.config.theme);
    PanelManager::get_instance().init(preference_manager.config.panel_name);
  }
  catch (const std::exception &e) {
    log_e("%s", e.what());
    throw;
  }
  catch (...) {
    log_e("Unknown exception occurred");
    throw;
  }
}

void loop()
{
  try
  {
    // log_d("...");
    uint32_t start_time = millis();

    // Process any pending LVGL tasks
    //Ticker::handle_lv_tasks();

    PanelManager::get_instance().refresh_panel();

    // Process LVGL tasks again to render the changes immediately
    //Ticker::handle_lv_tasks();

    // Adaptive Timing to generate a ~60fps refresh rate
    Ticker::handle_dynamic_delay(start_time);
  }
  catch (const std::exception &e)
  {
    log_e("%s", e.what());
    throw;
  }
  catch (...)
  {
    log_e("Unknown exception occurred");
    throw;
  }
}