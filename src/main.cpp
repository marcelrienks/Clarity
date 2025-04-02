#include "main.h"

void setup() {
  try {
    SerialLogger().init();
    SerialLogger().log_point("main::setup()", "...");

    // Initialize 
    _preferences.init();
    _device.prepare();
    _panel_manager.init(&_device);
  }
  catch (const std::exception& e) {
    SerialLogger().log(e.what());
    throw;
  }
  catch (...) {
    SerialLogger().log_point("main::setup()", "Unknown exception occurred");
    throw;
  }
}

void loop()
{
  try
  {
    // SerialLogger().log_point("main::loop()", "...");
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
    SerialLogger().log(e.what());
    throw;
  }
  catch (...)
  {
    SerialLogger().log_point("main::loop()", "Unknown exception occurred");
    throw;
  }
}