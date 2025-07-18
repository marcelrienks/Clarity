#include "panels/oem_oil_panel.h"

// Constructors and Destructors

OemOilPanel::OemOilPanel()
    : _oem_oil_pressure_widget(std::make_shared<OemOilPressureWidget>()),
      _oem_oil_temperature_widget(std::make_shared<OemOilTemperatureWidget>()),
      _oem_oil_pressure_sensor(std::make_shared<OilPressureSensor>()),
      _oem_oil_temperature_sensor(std::make_shared<OilTemperatureSensor>()) {}

OemOilPanel::~OemOilPanel()
{
    // Stop any running animations before destroying the panel
    if (_is_pressure_animation_running) {
        lv_anim_delete(&_pressure_animation, nullptr);
        _is_pressure_animation_running = false;
    }
    
    if (_is_temperature_animation_running) {
        lv_anim_delete(&_temperature_animation, nullptr);
        _is_temperature_animation_running = false;
    }

    if (_screen) {
        lv_obj_delete(_screen);
    }

    if (_oem_oil_pressure_widget) {
        _oem_oil_pressure_widget.reset();
    }

    if (_oem_oil_temperature_widget) {
        _oem_oil_temperature_widget.reset();
    }

    if (_oem_oil_pressure_sensor) {
        _oem_oil_pressure_sensor.reset();
    }

    if (_oem_oil_temperature_sensor) {
        _oem_oil_temperature_sensor.reset();
    }
}

// Core Functionality Methods

/// @brief Initialize the panel for showing Oil related information
/// Creates screen and initializes sensors with sentinel values
void OemOilPanel::init()
{
    log_d("...");

    _screen = LvTools::create_blank_screen();

    _oem_oil_pressure_sensor->init();
    _current_oil_pressure_value = -1; // Sentinel value to ensure first update

    _oem_oil_temperature_sensor->init();
    _current_oil_temperature_value = -1; // Sentinel value to ensure first update
}

/// @brief Load the panel with component rendering and screen display
/// @param show_panel_completion_callback to be called when the panel load is completed
void OemOilPanel::load(std::function<void()> show_panel_completion_callback)
{
    log_d("...");
    _callback_function = show_panel_completion_callback;

    // Create location parameters with rotational start points for scales
    WidgetLocation pressure_location(210); // rotation starting at 210 degrees
    WidgetLocation temperature_location(30); // rotation starting at 30 degrees
    
    _oem_oil_pressure_widget->render(_screen, pressure_location);
    _oem_oil_temperature_widget->render(_screen, temperature_location);
    lv_obj_add_event_cb(_screen, OemOilPanel::show_panel_completion_callback, LV_EVENT_SCREEN_LOADED, this);

    log_v("loading...");
    lv_screen_load(_screen);
}

/// @brief Update the reading on the screen
void OemOilPanel::update(std::function<void()> update_panel_completion_callback)
{
    log_d("...");

    _callback_function = update_panel_completion_callback;

    log_v("updating...");
    OemOilPanel::update_oil_pressure();
    OemOilPanel::update_oil_temperature();
    
    // If no animations were started, call completion callback immediately
    if (!_is_pressure_animation_running && !_is_temperature_animation_running) {
        log_d("No animations running, calling completion callback immediately");
        _callback_function();
    }
}

// Private Methods

/// @brief Update the oil pressure reading on the screen
void OemOilPanel::update_oil_pressure()
{
    log_d("...");

    // Skip update if pressure animation is already running
    if (_is_pressure_animation_running) {
        log_d("Pressure animation running, skipping update");
        return;
    }

    // Use delta-based updates for better performance
    auto sensor_value = std::get<int32_t>(_oem_oil_pressure_sensor->get_reading());
    auto value = map_pressure_value(sensor_value);
    
    // Skip update only if value is exactly the same as last update
    if (value == _current_oil_pressure_value) {
        log_d("Pressure value unchanged (%d), skipping update", value);
        return;
    }
    
    log_i("Updating pressure from %d to %d", _current_oil_pressure_value, value);
    _oem_oil_pressure_widget->refresh(Reading{value});

    // Setup animation
    lv_anim_init(&_pressure_animation);
    lv_anim_set_duration(&_pressure_animation, _animation_duration);
    lv_anim_set_repeat_count(&_pressure_animation, 0);
    lv_anim_set_playback_duration(&_pressure_animation, 0);
    lv_anim_set_values(&_pressure_animation, _current_oil_pressure_value, value);
    lv_anim_set_var(&_pressure_animation, this);
    lv_anim_set_user_data(&_pressure_animation, (void *)static_cast<uintptr_t>(OilSensorTypes::Pressure));
    lv_anim_set_exec_cb(&_pressure_animation, OemOilPanel::execute_pressure_animation_callback);
    lv_anim_set_completed_cb(&_pressure_animation, OemOilPanel::update_panel_completion_callback);

    _is_pressure_animation_running = true;
    log_d("animating...");
    lv_anim_start(&_pressure_animation);
}

/// @brief Update the oil temperature reading on the screen
void OemOilPanel::update_oil_temperature()
{
    log_d("...");

    // Skip update if temperature animation is already running
    if (_is_temperature_animation_running) {
        log_d("Temperature animation running, skipping update");
        return;
    }

    // Use delta-based updates for better performance
    auto sensor_value = std::get<int32_t>(_oem_oil_temperature_sensor->get_reading());
    auto value = map_temperature_value(sensor_value);
    
    // Skip update only if value is exactly the same as last update
    if (value == _current_oil_temperature_value) {
        log_d("Temperature value unchanged (%d), skipping update", value);
        return;
    }
    
    log_i("Updating temperature from %d to %d", _current_oil_temperature_value, value);
    _oem_oil_temperature_widget->refresh(Reading{value});

    // Setup animation
    lv_anim_init(&_temperature_animation);
    lv_anim_set_duration(&_temperature_animation, _animation_duration);
    lv_anim_set_repeat_count(&_temperature_animation, 0);
    lv_anim_set_playback_duration(&_temperature_animation, 0);
    lv_anim_set_values(&_temperature_animation, _current_oil_temperature_value, value);
    lv_anim_set_var(&_temperature_animation, this);
    lv_anim_set_user_data(&_temperature_animation, (void *)static_cast<uintptr_t>(OilSensorTypes::Temperature));
    lv_anim_set_exec_cb(&_temperature_animation, OemOilPanel::execute_temperature_animation_callback);
    lv_anim_set_completed_cb(&_temperature_animation, OemOilPanel::update_panel_completion_callback);

    _is_temperature_animation_running = true;
    log_d("animating...");
    lv_anim_start(&_temperature_animation);
}

// Static Callback Methods

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
    if (sensor_type == OilSensorTypes::Pressure) {
        this_instance->_current_oil_pressure_value = animation->current_value;
        this_instance->_is_pressure_animation_running = false;
    } else if (sensor_type == OilSensorTypes::Temperature) {
        this_instance->_current_oil_temperature_value = animation->current_value;
        this_instance->_is_temperature_animation_running = false;
    }

    // Only call the callback function if both animations are not running
    if (!this_instance->_is_pressure_animation_running && !this_instance->_is_temperature_animation_running) {
        this_instance->_callback_function();
    }
}

/// @brief callback used by the animation to set the values smoothly until ultimate value is reached
/// @param target the object being animated
/// @param value the next value in a sequence to create a smooth transition
void OemOilPanel::execute_pressure_animation_callback(void *target, int32_t value)
{
    log_d("...");

    lv_anim_t *animation = lv_anim_get(target, execute_pressure_animation_callback); // get the animation
    auto this_instance = static_cast<OemOilPanel *>(animation->var);                    // use the animation to get the var which is this instance
    this_instance->_oem_oil_pressure_widget.get()->set_value(value);
}

/// @brief callback used by the animation to set the values smoothly until ultimate value is reached
/// @param target the object being animated
/// @param value the next value in a sequence to create a smooth transition
void OemOilPanel::execute_temperature_animation_callback(void *target, int32_t value)
{
    log_d("...");

    lv_anim_t *animation = lv_anim_get(target, execute_temperature_animation_callback); // get the animation
    auto this_instance = static_cast<OemOilPanel *>(animation->var);                       // use the animation to get the var which is this instance
    this_instance->_oem_oil_temperature_widget.get()->set_value(value);
}

// Value mapping methods

/// @brief Map oil pressure sensor value to display scale
/// @param sensor_value Raw sensor value (1-10 Bar)
/// @return Mapped value for display (0-60, representing 0.0-6.0 Bar x10)
int32_t OemOilPanel::map_pressure_value(int32_t sensor_value)
{
    // Clamp sensor value to valid range (1-10 Bar)
    if (sensor_value < 1) sensor_value = 1;
    if (sensor_value > 10) sensor_value = 10;
    
    // Map 1-10 Bar to 0-60 display units
    // Formula: (sensor_value - 1) * 60 / 9
    // This maps: 1 Bar -> 0, 10 Bar -> 60
    // Display represents 0.0-6.0 Bar with 0.1 precision (x10 multiplier)
    return ((sensor_value - 1) * 60) / 9;
}

/// @brief Map oil temperature sensor value to display scale
/// @param sensor_value Raw sensor value (0-120°C)
/// @return Mapped value for display
int32_t OemOilPanel::map_temperature_value(int32_t sensor_value)
{
    // Temperature mapping is direct 1:1 as the display range matches sensor range
    // Clamp to valid range (0-120°C)
    if (sensor_value < 0) sensor_value = 0;
    if (sensor_value > 120) sensor_value = 120;
    
    return sensor_value;
}