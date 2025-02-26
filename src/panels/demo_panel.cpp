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
    lv_scr_load(this->_screen);

    SerialLogger().log_point("DemoPanel::show()", "Completed");
}

/// @brief Update the reading on the screen
void DemoPanel::update()
{
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