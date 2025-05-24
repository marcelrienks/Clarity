#include "main.h"

void setup() {
  try {
    log_d("...");

    // Initialise Device and LVGL first
    Device::get_instance().prepare();
    
    // Give LVGL a chance to initialize properly
    Ticker::handle_lv_tasks();

    PreferenceManager preference_manager = PreferenceManager::get_instance();
    preference_manager.init();

    StyleManager::get_instance().init(preference_manager.config.theme);
    
    // Process LVGL tasks after style initialization
    Ticker::handle_lv_tasks();

    PanelManager::get_instance().init(preference_manager.config.panel_name);
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
    uint32_t start_time = millis();

    // Process any pending LVGL tasks
    Ticker::handle_lv_tasks();

    PanelManager::get_instance().refresh_panel();

    // Process LVGL tasks again to render the changes immediately
    Ticker::handle_lv_tasks();

    // Adaptive Timing to generate a ~60fps refresh rate
    Ticker::handle_dynamic_delay(start_time);
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