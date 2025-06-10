#include "panels/oil_panel.h"

OilPanel::OilPanel()
    : _oil_pressure_component(std::make_shared<OilPressureComponent>()),
      _oil_temperature_component(std::make_shared<OilTemperatureComponent>()),
      _sensor(std::make_shared<BoschPstf1Sensor>()) {}

OilPanel::~OilPanel()
{
    if (_screen)
        lv_obj_delete(_screen);

    if (_oil_pressure_component)
        _oil_pressure_component.reset();

    if (_oil_temperature_component)
        _oil_temperature_component.reset();

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
    _current_oil_pressure_value = 0;
    _current_oil_temperature_value = 0;
}

/// @brief Show the panel
/// @param callback_function to be called when the panel show is completed
void OilPanel::load(std::function<void()> show_panel_completion_callback)
{
    log_d("...");
    _callback_function = show_panel_completion_callback;

    _oil_pressure_component->render_load(_screen);
    //_oil_temperature_component->render_load(_screen);
    lv_obj_add_event_cb(_screen, OilPanel::show_panel_completion_callback, LV_EVENT_SCREEN_LOADED, this);

    log_v("loading...");
    lv_scr_load(_screen);
}

/// @brief Update the reading on the screen
void OilPanel::update(std::function<void()> update_panel_completion_callback)
{
    log_d("...");

    _callback_function = update_panel_completion_callback;

    log_v("updating...");
    OilPanel::update_oil_pressure();
    //OilPanel::update_oil_temperature();
}

/// @brief Update the oil pressure reading on the screen
void OilPanel::update_oil_pressure()
{
    auto value = std::get<int32_t>(_sensor->get_reading(OilSensorTypes::Pressure));
    static lv_anim_t update_pressure_animation;
    _oil_pressure_component->render_update(&update_pressure_animation, _current_oil_pressure_value, value);

    lv_anim_set_var(&update_pressure_animation, this);
    lv_anim_set_user_data(&update_pressure_animation, (void*)static_cast<uintptr_t>(OilSensorTypes::Pressure));
    lv_anim_set_exec_cb(&update_pressure_animation, OilPanel::execute_pressure_animation_callback);
    lv_anim_set_completed_cb(&update_pressure_animation, OilPanel::update_panel_completion_callback);

    _is_pressure_animation_running = true;
    lv_anim_start(&update_pressure_animation);
}

/// @brief Update the oil temperature reading on the screen
void OilPanel::update_oil_temperature()
{
    auto value = std::get<int32_t>(_sensor->get_reading(OilSensorTypes::Temperature));
    static lv_anim_t update_temperature_animation;
    _oil_pressure_component->render_update(&update_temperature_animation, _current_oil_pressure_value, value);

    lv_anim_set_var(&update_temperature_animation, this);
    lv_anim_set_user_data(&update_temperature_animation, (void*)static_cast<uintptr_t>(OilSensorTypes::Temperature));
    lv_anim_set_exec_cb(&update_temperature_animation, OilPanel::execute_temperature_animation_callback);
    lv_anim_set_completed_cb(&update_temperature_animation, OilPanel::update_panel_completion_callback);

    _is_temperature_animation_running = true;
    lv_anim_start(&update_temperature_animation);
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
    auto sensor_type = static_cast<OilSensorTypes>(reinterpret_cast<uintptr_t>(animation->user_data));

    // Determine which animation has completed and update the corresponding value
    if (sensor_type == OilSensorTypes::Pressure)
    {
        this_instance->_current_oil_pressure_value = animation->current_value;
        this_instance->_is_pressure_animation_running = false;
    }
    else if (sensor_type == OilSensorTypes::Temperature)
    {
        this_instance->_current_oil_temperature_value = animation->current_value;
        this_instance->_is_temperature_animation_running = false;
    }
    
    // Only call the callback function if both animations are not running
    if (!this_instance->_is_pressure_animation_running && !this_instance->_is_temperature_animation_running)
        this_instance->_callback_function();
}

/// @brief callback used by the animation to set the values smoothly until ultimate value is reached
/// @param target the object being animated
/// @param value the next value in a sequence to create a smooth transition
void OilPanel::execute_pressure_animation_callback(void *target, int32_t value)
{
    log_d("...");

    lv_anim_t *animation = lv_anim_get(target, execute_pressure_animation_callback); // get the animation
    auto this_instance = static_cast<OilPanel *>(animation->var);                    // use the animation to get the var which is this instance
    this_instance->_oil_pressure_component.get()->set_value(value);
}

/// @brief callback used by the animation to set the values smoothly until ultimate value is reached
/// @param target the object being animated
/// @param value the next value in a sequence to create a smooth transition
void OilPanel::execute_temperature_animation_callback(void *target, int32_t value)
{
    log_d("...");

    lv_anim_t *animation = lv_anim_get(target, execute_temperature_animation_callback); // get the animation
    auto this_instance = static_cast<OilPanel *>(animation->var);                       // use the animation to get the var which is this instance
    this_instance->_oil_temperature_component.get()->set_value(value);
}