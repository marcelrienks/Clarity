#include "panels/demo_panel.h"
#include "components/demo_component.h"
#include "sensors/demo_sensor.h"
#include "device.h"

/// @brief DemoPanel constructor, generates a component and sensor
DemoPanel::DemoPanel()
{
    _component = std::make_shared<DemoComponent>();
    _sensor = std::make_shared<DemoSensor>();
}

/// @brief Set the function to be called on completion of this panel animations
/// @param callback_function the function to be executed when animation is complete
void DemoPanel::set_completion_callback(std::function<void()> callback_function)
{
    _callback_function = callback_function;
}

/// @brief Initialize the screen with component and sensor
void DemoPanel::init(IDevice *device)
{
    SerialLogger().log_point("DemoPanel::init()", "Entry...");

    _device = device;
    _screen = LvTools::create_blank_screen();

    // Initialize the sensor
    _sensor->init();

    // Initialize the component with the screen
    _component->init(_screen);
}

/// @brief Show the screen
void DemoPanel::show()
{
    SerialLogger().log_point("DemoPanel::show()", "Entry...");

    lv_scr_load(_screen);

    // Call the completion callback immediately since we have no animations
    if (_callback_function)
    {
        _callback_function();
    }
}

/// @brief Update the reading on the screen
void DemoPanel::update()
{
    //SerialLogger().log_point("DemoPanel::update()", "Entry...");

    Reading reading = _sensor->get_reading();
    _component->update(reading);
}

/// @brief DemoPanel destructor to clean up dynamically allocated objects
DemoPanel::~DemoPanel()
{
    // Component and sensor are managed by shared_ptr and will be cleaned up automatically
}