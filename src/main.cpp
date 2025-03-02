#define LGFX_USE_V1

#include "main.h"

// TODO: create a screen level duration variable to control the duration of the animation / or delays if that screen does not animate
// TODO: create an interface for screen & component, with generics, so that swapping them in and out is standardised
// TODO: create a screen manager to handle the screen transitions
// TODO: refactor startup screen function to use screen manager
// TODO: see if any refactoring of splash screen is needed

void setup()
{
  try
  {
    SerialLogger().init();
    SerialLogger().log_point("main::setup()", "Entry");

    _device.prepare();

    // Splash screen
    _splash_panel = new SplashPanel();
    _splash_panel->init(&_device);

    _demo_panel = new DemoPanel();
    _demo_panel->init(&_device);

    // Set up the callback for when splash screen completes
    _splash_panel->set_completion_callback([&]() {
      SerialLogger().log_point("main::set_completion_callback", "Completion callback running");
      _is_setup_complete = true;
      _demo_panel->show(); });//TODO: should this be pulled out into a separate method, also considering generics

    // Start with splash panel
    _splash_panel->show();

    SerialLogger().log_point("main::setup()", "Completed");
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