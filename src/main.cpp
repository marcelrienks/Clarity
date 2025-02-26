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
    _splash_panel->show();

    // _demo_panel = new DemoPanel();
    // _demo_panel->init(&_device);
    // _demo_panel->show();

    SerialLogger().log_point("main::setup()", "Completed");
  }
  catch (const std::exception &e)
  {
    SerialLogger().log_message(e.what());
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
    SerialLogger().log_point("main::loop()", "Entry");

    // if (_device._is_splash_complete)
    //   _demo_panel->update();

    // else
    //   SerialLogger().log_point("main::loop()", "splash running...");

    // Handle all tasks, to allow screen to render
    Ticker::handle_lv_tasks();

    //_demo_screen->update();
    delay(250);

    SerialLogger().log_point("main::loop()", "Completed");
  }
  catch (const std::exception &e)
  {
    SerialLogger().log_message(e.what());
    throw;
  }
  catch (...)
  {
    SerialLogger().log_point("main::loop()", "Unknown exception occurred");
    throw;
  }
}