#include "main.h"

// TODO: analyse all classes, and their objects between stack vs heap, or use smart pointers (like std::unique_ptr or std::shared_ptr)
// based on research dynamic allocation is preferred for lv objects
// Automatic storage (stack allocation, tied to a specific scope)
//    SomeClass localObj;       // Object on stack
//    SomeClass* ptr = &localObj; // Pointer to stack object
// and Dynamic allocation (heap allocation, independent of scope, manually managed)
//    SomeClass* ptr = new SomeClass(); // Object on heap

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
    _panel_manager->init();
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
    // SerialLogger().log_point("main::loop()", "...");
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