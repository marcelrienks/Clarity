#include "main.h"

//TODO: Review all static variables, in terms of lifecycle and cleaning up
//TODO: Review all destructors

void setup() {
  try {
    SerialLogger().init();
    SerialLogger().log_point("main::setup()", "...");

    // Initialize device
    _device.prepare();
    
    // Initialize preferences
    if (!_preferences.begin()) {
      SerialLogger().log_point("main::setup()", "Failed to initialize preferences");
    }

    // Create panel manager with preferences
    _panel_manager = std::make_shared<PanelManager>(&_device, &_preferences);
    _panel_manager->init();
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

    _panel_manager->show_all_panels();
    _panel_manager->update_current_panel();

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