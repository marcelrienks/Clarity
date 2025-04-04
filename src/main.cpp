#include "main.h"

void setup() {
  try {
    log_d("...");

    // Initialize 
    _preferences.init();
    _device.prepare();
    _panel_manager.init(&_device);
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

    _panel_manager.show_all_panels();
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