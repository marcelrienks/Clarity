#define LGFX_USE_V1

#include "main.h"

void setup()
{
  try
  {
//TODO: convert this logic into a list of Panels that get rendered in order, using a Panel manager
// This will allow for generic configuration of any screen to be shown, pending configs
// Also see if it's possible to allow for a default transition, but each Panel can override that

    SerialLogger().init();
    SerialLogger().log_point("main::setup()", "Entry...");

    _device.prepare();

    // Splash screen
    _splash_panel = new SplashPanel();
    _splash_panel->init(&_device);

    _demo_panel = new DemoPanel();
    _demo_panel->init(&_device);

    // Set up the callback for when splash screen completes
    _splash_panel->set_completion_callback([&]() {
      SerialLogger().log_point("main::set_completion_callback", "Splash Panel Completion...");
      _is_setup_complete = true;
      _demo_panel->show(); });

    // Start with splash panel
    _splash_panel->show();
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
    //SerialLogger().log_point("main::loop()", "Entry");
    uint32_t start_time = millis();

    // First process any pending LVGL tasks
    Ticker::handle_lv_tasks();

    if (_is_setup_complete)
    {
      _demo_panel->update();

      // Process LVGL tasks again to render the changes immediately
      Ticker::handle_lv_tasks();
    }

    else
      SerialLogger().log_point("main::loop()", "splash running...");

    // Adaptive Timing to generate a ~60fps refresh rate
    Ticker::handle_dynamic_delay(start_time);

    //SerialLogger().log_point("main::loop()", "Completed");
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