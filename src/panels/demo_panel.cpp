#include "panels/demo_panel.h"
#include "components/demo_component.h"
#include "sensors/demo_sensor.h"
#include "utilities/lv_tools.h"

DemoPanel::DemoPanel()
{
    _component = std::make_shared<DemoComponent>();
    _sensor = std::make_shared<DemoSensor>();
}

DemoPanel::~DemoPanel()
{
    if (_screen)
        lv_obj_delete(_screen);

    if (_component)
        _component.reset();

    if (_sensor)
        _sensor.reset();
}

/// @brief Initialize the panel for illustrating a demo
/// @param device
void DemoPanel::init(IDevice *device)
{
    SerialLogger().log_point("DemoPanel::init()", "...");

    _device = device;
    _screen = LvTools::create_blank_screen();
    
    // Initialize the sensor
    _sensor->init();

    // Initialize the component with the screen
    _component->init(_screen);
}

/// @brief Show the panel
/// @param callback_function to be called when the panel show is completed
void DemoPanel::show(std::function<void()> callback_function)
{
    SerialLogger().log_point("DemoPanel::show()", "...");

    _callback_function = callback_function;

    lv_scr_load(_screen);

    // Call the completion callback immediately since we have no animations
    if (_callback_function)
        _callback_function();
}

/// @brief Update the reading on the screen
void DemoPanel::update()
{
    SerialLogger().log_point("DemoPanel::update()", "...");

    Reading reading = _sensor->get_reading();
    _component->update(reading);
}
