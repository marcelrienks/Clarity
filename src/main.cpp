#include "main.h"

void setup() {
  try {
    log_d("...");

    // Initialise
    Device::get_instance().prepare();
    PreferenceManager::get_instance().init();
    StyleManager::get_instance().init(PreferenceManager::get_instance().config.theme);
    PanelManager::get_instance().init(&PreferenceManager::get_instance().config.panel);
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
    Ticker::handle_lv_tasks();

    //TODO: show splash, than configured screen should that be handled by this, or Panel_manager, either way new func in panel manager needed
    _panel_manager.update_current_panel();

    // Process LVGL tasks again to render the changes immediately
    Ticker::handle_lv_tasks();

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