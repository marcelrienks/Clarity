#include "panels/oil_panel.h"

OilPanel::OilPanel(IDevice *device)
    : _device(device), _component(std::make_shared<OilPressureComponent>()), _sensor(std::make_shared<OilPressureSensor>()) {}

OilPanel::~OilPanel()
{
    if (_screen)
        lv_obj_delete(_screen);

    if (_component)
        _component.reset();

    if (_sensor)
        _sensor.reset();
}

/// @brief Initialize the panel for showing Oil related information
/// @param device
void OilPanel::init()
{
    log_d("...");
    _screen = LvTools::create_blank_screen();
    _sensor->init();
    _current_value = 0;
}

/// @brief Show the panel
/// @param callback_function to be called when the panel show is completed
void OilPanel::show(std::function<void()> show_panel_completion_callback)
{
    log_i("...");
    _callback_function = show_panel_completion_callback;

    _component->render_show(_screen);
    lv_obj_add_event_cb(_screen, OilPanel::show_panel_completion_callback, LV_EVENT_SCREEN_LOADED, this);

    log_d("load...");
    lv_scr_load(_screen);
}

/// @brief Update the reading on the screen
void OilPanel::update(std::function<void()> update_panel_completion_callback)
{
    log_i("...");
    _callback_function = update_panel_completion_callback;

    auto value = std::get<int32_t>(_sensor->get_reading());
    static lv_anim_t update_animation;
    _component->render_update(&update_animation, _current_value, value);

    lv_anim_set_var(&update_animation, this);
    lv_anim_set_exec_cb(&update_animation, OilPanel::execute_animation_callback);
    lv_anim_set_completed_cb(&update_animation, OilPanel::update_panel_completion_callback);

    log_d("animate...");
    lv_anim_start(&update_animation);
}

/// @brief The callback to be run once show panel has completed
/// @param event LVGL event that was used to call this
void OilPanel::show_panel_completion_callback(lv_event_t *event)
{
    log_d("...");
    auto this_instance = static_cast<OilPanel *>(lv_event_get_user_data(event));
    this_instance->_callback_function();
}

/// @brief Callback when animation has completed. aka update complete
/// @param animation the object that was animated
void OilPanel::update_panel_completion_callback(lv_anim_t *animation)
{
    log_d("...");
    auto this_instance = static_cast<OilPanel *>(animation->var);
    this_instance->_current_value = animation->current_value;
    this_instance->_callback_function();
}

/// @brief callback used by the animation to set the values smoothly until ultimate value is reached
/// @param target the object being animated
/// @param value the next value in a sequence to create a smooth transition
void OilPanel::execute_animation_callback(void *target, int32_t value)
{
    log_d("...");
    lv_anim_t *animation = lv_anim_get(target, execute_animation_callback); // get the animation
    auto this_instance = static_cast<OilPanel *>(animation->var);          // use the animation to get the var which is this instance
    this_instance->_component.get()->set_value(value);
}