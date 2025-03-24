#include "panels/demo_panel.h"
#include "components/demo_component.h"
#include "sensors/demo_sensor.h"
#include "utilities/lv_tools.h"

//TODO: there is a blurring of lines here, for init/show all logic is in panel, and only graphics is in component
// But for update componenet contains graphics and logic. I need to figure out where the line is, and clean things up

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
    _sensor->init();
    _component->init(_screen);
}

/// @brief Show the panel
/// @param callback_function to be called when the panel show is completed
void DemoPanel::show(std::function<void()> show_panel_completion_callback)
{
    SerialLogger().log_point("DemoPanel::show()", "...");
    _show_panel_completion_callback = show_panel_completion_callback;

    lv_obj_add_event_cb(_screen, DemoPanel::show_panel_completion_callback,
                        LV_EVENT_SCREEN_LOADED, this);

    lv_scr_load(_screen);
}

/// @brief Update the reading on the screen
void DemoPanel::update(std::function<void()> update_panel_completion_callback)
{
    SerialLogger().log_point("DemoPanel::update()", "...");

    Reading reading = _sensor->get_reading();
    _component->render_reading(reading, update_panel_completion_callback);
}

void DemoPanel::show_panel_completion_callback(lv_event_t *event)
{
    auto this_instance = static_cast<DemoPanel*>(lv_event_get_user_data(event));
    this_instance->_show_panel_completion_callback();
}