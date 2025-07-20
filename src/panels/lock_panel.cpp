#include "panels/lock_panel.h"
#include <variant>

// Constructors and Destructors
LockPanel::LockPanel()
    : _lock_component(std::make_shared<LockComponent>()),
      _lock_sensor(std::make_shared<LockSensor>()) {}

LockPanel::~LockPanel()
{
    if (_screen)
    {
        lv_obj_delete(_screen);
    }

    if (_lock_component)
    {
        _lock_component.reset();
    }

    if (_lock_sensor)
    {
        _lock_sensor.reset();
    }
}

// Core Functionality Methods

/// @brief Initialize the lock panel and its components
void LockPanel::init()
{
    log_d("...");

    _screen = LvTools::create_blank_screen();
    _center_location = ComponentLocation(LV_ALIGN_CENTER, 0, 0);

    _lock_sensor->init();
    _is_lock_engaged = false;
}

/// @brief Load the lock panel UI components
void LockPanel::load(std::function<void()> callback_function)
{
    log_d("...");
    _callback_function = callback_function;

    // Create the lock component centered on screen, and immediately refresh it with the current lock status
    _lock_component->render(_screen, _center_location);
    _lock_component->refresh(Reading{_is_lock_engaged});
    lv_obj_add_event_cb(_screen, LockPanel::show_panel_completion_callback, LV_EVENT_SCREEN_LOADED, this);

    log_v("loading...");
    lv_screen_load(_screen);
}

/// @brief Update the lock panel with current sensor data
void LockPanel::update(std::function<void()> callback_function)
{
    // Immediately call the completion callback so that lock/unlock logic is processed
    callback_function();
}

// Static Methods

/// @brief The callback to be run once show panel has completed
/// @param event LVGL event that was used to call this
void LockPanel::show_panel_completion_callback(lv_event_t *event)
{
    log_d("...");

    auto this_instance = static_cast<LockPanel *>(lv_event_get_user_data(event));
    this_instance->_callback_function();
}