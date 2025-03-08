#include "main.h"

// TODO: Update all method comments, and ensure they are copied to headers
// TODO: in the headers put all private declarations at the bottom, all constructor/deconstructors together at the top, and match the order in the classes

void setup()
{
  try
  {
    SerialLogger().init();
    SerialLogger().log_point("main::setup()", "Entry...");

    // Initialize device
    _device.prepare();

    // Create panel manager
    _panel_manager = std::make_shared<PanelManager>(&_device);

    // Create panels
    _splash_panel = std::make_shared<SplashPanel>();
    _demo_panel = std::make_shared<DemoPanel>();

    // Register panels with the manager
    _panel_manager->register_panel(_splash_panel);
    _panel_manager->register_panel(_demo_panel);


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
    uint32_t start_time = millis();

    // Process any pending LVGL tasks
    Ticker::handle_lv_tasks();

    if (_is_setup_complete)
    {
      // Update the current panel via the panel manager
      _panel_manager->update();

      // Process LVGL tasks again to render the changes immediately
      Ticker::handle_lv_tasks();
    }
    else
    {
      SerialLogger().log_point("main::loop()", "splash running...");
    }

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