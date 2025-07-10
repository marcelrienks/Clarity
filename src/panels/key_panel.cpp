#include "panels/key_panel.h"
#include <variant>

// Constructors and Destructors
KeyPanel::KeyPanel()
    : _key_component(std::make_shared<KeyComponent>()),
      _key_sensor(std::make_shared<KeySensor>()) {}

KeyPanel::~KeyPanel()
{
    if (_screen) {
        lv_obj_delete(_screen);
    }

    if (_key_component)
    {
        _key_component.reset();
    }

    if (_key_sensor)
    {
        _key_sensor.reset();
    }
}

// Core Functionality Methods
/// @brief Initialize the key panel and its components
void KeyPanel::init()
{
    log_d("...");

    _screen = LvTools::create_blank_screen();
    _center_location = ComponentLocation(LV_ALIGN_CENTER, 0, 0);

    _key_sensor->init();
    _is_key_present = false;
}

/// @brief Load the key panel UI components
void KeyPanel::load(std::function<void()> callback_function)
{
    log_d("...");
    _callback_function = callback_function;

    // Create the key component cantered on screen, anf immediately refresh it with the current key status
    _key_component->render(_screen, _center_location);
    _key_component->refresh(Reading{_is_key_present});
    lv_obj_add_event_cb(_screen, KeyPanel::show_panel_completion_callback, LV_EVENT_SCREEN_LOADED, this);

    log_v("loading...");
    lv_screen_load(_screen);
}

/// @brief Update the key panel with current sensor data
void KeyPanel::update(std::function<void()> callback_function)
{
    log_d("...");

    // Get current key status from sensor
    bool is_key_present = std::get<bool>(_key_sensor->get_reading());

    // Skip update only if value is exactly the same as last update
    if (is_key_present != _is_key_present)
    {
        _is_key_present = is_key_present;
        _key_component->refresh(Reading{_is_key_present});
    }

    callback_function();
}

// Static Methods
/// @brief The callback to be run once show panel has completed
/// @param event LVGL event that was used to call this
void KeyPanel::show_panel_completion_callback(lv_event_t *event)
{
    log_d("...");

    auto this_instance = static_cast<KeyPanel *>(lv_event_get_user_data(event));
    this_instance->_callback_function();
}