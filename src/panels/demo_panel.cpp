#include "panels/demo_panel.h"
#include "components/demo_component.h"
#include "sensors/demo_sensor.h"
#include "device.h"

/// @brief DemoPanel constructor, generates a component and sensor
DemoPanel::DemoPanel()
{
    _component = new DemoComponent();
    _sensor = new DemoSensor();
}

/// @brief Set the callback to be called on panel completion
void DemoPanel::set_completion_callback(PanelCompletionCallback callback)
{
    _completion_callback = callback;
    SerialLogger().log_point("DemoPanel::set_completion_callback()", "Callback set");
}

/// @brief Initialize the screen with component and sensor 
void DemoPanel::init(IDevice *device)
{
    _device = device;

    this->_screen = lv_obj_create(NULL);
    _component->init(this->_screen);

    SerialLogger().log_point("DemoPanel::init()", "Completed");
}

/// @brief Show the screen
void DemoPanel::show()
{
    SerialLogger().log_point("DemoPanel::show()", "Entry");

    lv_scr_load(this->_screen);

    SerialLogger().log_point("DemoPanel::show()", "Completed");
}

/// @brief Update the reading on the screen
void DemoPanel::update()
{
    SerialLogger().log_point("DemoPanel::update()", "Entry");
    
    int reading = _sensor->get_reading();
    _component->update(std::make_shared<int>(reading));

    SerialLogger().log_point("DemoPanel::update()", "Completed");
}

/// @brief DemoPanel destructor to clean up dynamically allocated objects
DemoPanel::~DemoPanel()
{
    if (_component)
        delete _component;

    if (_sensor)
        delete _sensor;
}