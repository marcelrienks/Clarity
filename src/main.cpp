#include "main.h"

// How can the show_panel/show_panels_recursively be used, in setup vs main, in order to show all panels, in a loop, but also allow regular updates of panels
// TODO: Update all method comments, and ensure they are copied to headers
// TODO: in the headers put all private declarations at the bottom, all constructor/deconstructors together at the top, and match the order in the classes

void setup()
{
  try
  {
    SerialLogger().init();
    SerialLogger().log_point("main::setup()", "...");

    // Initialize device
    _device.prepare();

    // Create panel manager
    _panel_manager = std::make_shared<PanelManager>(&_device);

    // Register panels with the manager
    _panel_manager->register_panel(std::make_shared<SplashPanel>());
    _panel_manager->register_panel(std::make_shared<DemoPanel>());
  }
  catch (const std::exception &e)
  {
    SerialLogger().log(e.what());
    throw;
  }
  catch (...)
  {
    SerialLogger().log_point("main::setup()", "Unknown exception occurred");
    throw;
  }
}

void loop()
{
  try
  {
    SerialLogger().log_point("main::loop()", "...");
    uint32_t start_time = millis();

    // Process any pending LVGL tasks
    Ticker::handle_lv_tasks();

    _panel_manager->show_all_panels();

    // Update the current panel via the panel manager
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