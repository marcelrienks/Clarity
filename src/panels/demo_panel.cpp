#include "panels/demo_panel.h"
#include "components/demo_component.h"
#include "sensors/demo_sensor.h"
#include "utilities/lv_tools.h"

// TODO: there is a blurring of lines here, for init/show all logic is in panel, and only graphics is in component
//  But for update componenet contains graphics and logic. I need to figure out where the line is, and clean things up

DemoPanel::DemoPanel(PanelIteration panel_iteration)
    : _component(std::make_shared<DemoComponent>()), _sensor(std::make_shared<DemoSensor>()), _iteration(panel_iteration) {}

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
void DemoPanel::init(IDevice *device)//TODO: convert logic here into a render function, to seperate init and rendering
{
    SerialLogger().log_point("DemoPanel::init()", "...");

    _device = device;
    _screen = LvTools::create_blank_screen();
    _sensor->init();
    _component->init(_screen);
}

/// @brief Show the panel
/// @param callback_function to be called when the panel show is completed
void DemoPanel::show(std::function<void()> callback_function)// Ensure show calls render function, to mach flow of update
{
    SerialLogger().log_point("DemoPanel::show()", "...");
    _callback_function = callback_function;

    lv_obj_add_event_cb(_screen, DemoPanel::show_panel_completion_callback,
                        LV_EVENT_SCREEN_LOADED, this);

    lv_scr_load(_screen);
}

/// @brief Update the reading on the screen
void DemoPanel::update(std::function<void()> callback_function)
{
    SerialLogger().log_point("DemoPanel::update()", "...");
    _callback_function = callback_function;
    Reading reading = _sensor->get_reading();
    
    _component->render_reading(reading, callback_function);
}

/// @brief The callback to be run once show panel has completed
/// @param event LVGL event that was used to call this
void DemoPanel::show_panel_completion_callback(lv_event_t *event)
{
    auto this_instance = static_cast<DemoPanel *>(lv_event_get_user_data(event));
    this_instance->_callback_function();
}

/// @brief The callback to be run once update panel has completed
/// @param event LVGL event that was used to call this
void DemoPanel::update_panel_completion_callback(lv_event_t *event)
{
    auto this_instance = static_cast<DemoPanel *>(lv_event_get_user_data(event));
    this_instance->_callback_function();
}