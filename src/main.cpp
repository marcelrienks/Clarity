#define LGFX_USE_V1

#include "main.h"

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
    _panel_manager->register_panel("splash", _splash_panel);
    _panel_manager->register_panel("demo", _demo_panel);
    
    // Set default transition
    TransitionConfig defaultTransition;
    defaultTransition.type = TransitionType::FADE_IN;
    defaultTransition.duration = 500;
    defaultTransition.delay = 0;
    _panel_manager->set_default_transition(defaultTransition);
    
    // Start with splash panel and set callback for when it completes
    TransitionConfig splashTransition;
    splashTransition.type = TransitionType::FADE_IN;
    splashTransition.duration = 1000;
    
    _panel_manager->show_panel("splash", splashTransition, []() {
      SerialLogger().log_point("main::setup()", "Splash panel completed...");
      _is_setup_complete = true;
      
      // Show demo panel with fade transition
      TransitionConfig demoTransition;
      demoTransition.type = TransitionType::FADE_IN;
      demoTransition.duration = 500;
      _panel_manager->show_panel("demo", demoTransition);
    });
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