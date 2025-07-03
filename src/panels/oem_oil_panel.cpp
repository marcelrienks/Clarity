#include "panels/oem_oil_panel.h"

OemOilPanel::OemOilPanel()
    : _oem_oil_pressure_component(std::make_shared<OemOilPressureComponent>()),
      _oem_oil_temperature_component(std::make_shared<OemOilTemperatureComponent>()),
      _oem_oil_pressure_sensor(std::make_shared<OilPressureSensor>()),
      _oem_oil_temperature_sensor(std::make_shared<OilTemperatureSensor>()) {}

OemOilPanel::~OemOilPanel()
{
    if (_screen)
        lv_obj_delete(_screen);

    if (_oem_oil_pressure_component)
        _oem_oil_pressure_component.reset();

    if (_oem_oil_temperature_component)
        _oem_oil_temperature_component.reset();

    if (_oem_oil_pressure_sensor)
        _oem_oil_pressure_sensor.reset();

    if (_oem_oil_temperature_sensor)
        _oem_oil_temperature_sensor.reset();
}

/// @brief Initialize the panel for showing Oil related information
/// @param device
void OemOilPanel::init()
{
    log_d("...");

    _screen = LvTools::create_blank_screen();

    _oem_oil_pressure_sensor->init();
    _current_oil_pressure_value = 0;

    _oem_oil_temperature_sensor->init();
    _current_oil_temperature_value = 0;
}

/// @brief Show the panel
/// @param callback_function to be called when the panel show is completed
void OemOilPanel::load(std::function<void()> show_panel_completion_callback)
{
    log_d("...");
    _callback_function = show_panel_completion_callback;

    // Create location parameters for components
    ComponentLocation pressure_location(LV_ALIGN_TOP_LEFT, 0, 0);
    ComponentLocation temperature_location(LV_ALIGN_TOP_RIGHT, 0, 0);
    
    _oem_oil_pressure_component->render_load(_screen, pressure_location);
    _oem_oil_temperature_component->render_load(_screen, temperature_location);
    lv_obj_add_event_cb(_screen, OemOilPanel::show_panel_completion_callback, LV_EVENT_SCREEN_LOADED, this);

    log_v("loading...");
    lv_scr_load(_screen);
}

/// @brief Update the reading on the screen
void OemOilPanel::update(std::function<void()> update_panel_completion_callback)
{
    log_d("...");

    _callback_function = update_panel_completion_callback;

    log_v("updating...");
    OemOilPanel::update_oil_pressure();
    OemOilPanel::update_oil_temperature();
}

/// @brief Update the oil pressure reading on the screen
void OemOilPanel::update_oil_pressure()
{
    log_d("...");

    auto value = std::get<int32_t>(_oem_oil_pressure_sensor->get_reading());
    static lv_anim_t update_pressure_animation;
    _oem_oil_pressure_component->render_update(&update_pressure_animation, _current_oil_pressure_value, value);

    lv_anim_set_var(&update_pressure_animation, this);
    lv_anim_set_user_data(&update_pressure_animation, (void *)static_cast<uintptr_t>(OilSensorTypes::Pressure));
    lv_anim_set_exec_cb(&update_pressure_animation, OemOilPanel::execute_pressure_animation_callback);
    lv_anim_set_completed_cb(&update_pressure_animation, OemOilPanel::update_panel_completion_callback);

    _is_pressure_animation_running = true;
    log_d("animating...");
    lv_anim_start(&update_pressure_animation);
}

/// @brief Update the oil temperature reading on the screen
void OemOilPanel::update_oil_temperature()
{
    log_d("...");

    auto value = std::get<int32_t>(_oem_oil_temperature_sensor->get_reading());
    static lv_anim_t update_temperature_animation;
    _oem_oil_temperature_component->render_update(&update_temperature_animation, _current_oil_temperature_value, value);

    lv_anim_set_var(&update_temperature_animation, this);
    lv_anim_set_user_data(&update_temperature_animation, (void *)static_cast<uintptr_t>(OilSensorTypes::Temperature));
    lv_anim_set_exec_cb(&update_temperature_animation, OemOilPanel::execute_temperature_animation_callback);
    lv_anim_set_completed_cb(&update_temperature_animation, OemOilPanel::update_panel_completion_callback);

    _is_temperature_animation_running = true;
    log_d("animating...");
    lv_anim_start(&update_temperature_animation);
}

/// @brief The callback to be run once show panel has completed
/// @param event LVGL event that was used to call this
void OemOilPanel::show_panel_completion_callback(lv_event_t *event)
{
    log_d("...");

    auto this_instance = static_cast<OemOilPanel *>(lv_event_get_user_data(event));
    this_instance->_callback_function();
}

/// @brief Callback when animation has completed. aka update complete
/// @param animation the object that was animated
void OemOilPanel::update_panel_completion_callback(lv_anim_t *animation)
{
    log_d("...");

    auto this_instance = static_cast<OemOilPanel *>(animation->var);
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
void OemOilPanel::execute_pressure_animation_callback(void *target, int32_t value)
{
    log_d("...");

    lv_anim_t *animation = lv_anim_get(target, execute_pressure_animation_callback); // get the animation
    auto this_instance = static_cast<OemOilPanel *>(animation->var);                    // use the animation to get the var which is this instance
    this_instance->_oem_oil_pressure_component.get()->set_value(value);
}

/// @brief callback used by the animation to set the values smoothly until ultimate value is reached
/// @param target the object being animated
/// @param value the next value in a sequence to create a smooth transition
void OemOilPanel::execute_temperature_animation_callback(void *target, int32_t value)
{
    log_d("...");

    lv_anim_t *animation = lv_anim_get(target, execute_temperature_animation_callback); // get the animation
    auto this_instance = static_cast<OemOilPanel *>(animation->var);                       // use the animation to get the var which is this instance
    this_instance->_oem_oil_temperature_component.get()->set_value(value);
}