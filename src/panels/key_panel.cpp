#include "panels/key_panel.h"
#include "hardware/gpio_pins.h"
#include <variant>
#include <Arduino.h>

// Constructors and Destructors
KeyPanel::KeyPanel() 
    : _key_component(std::make_shared<KeyComponent>()) {}

KeyPanel::~KeyPanel()
{
    if (_screen) {
        lv_obj_delete(_screen);
    }

    if (_key_component)
    {
        _key_component.reset();
    }
}

// Core Functionality Methods
/// @brief Initialize the key panel and its components
void KeyPanel::init()
{
    log_d("...");

    _screen = LvTools::create_blank_screen();
    _center_location = ComponentLocation(LV_ALIGN_CENTER, 0, 0);

    // Get current key state by reading GPIO pins directly
    bool pin25_high = digitalRead(GpioPins::KEY_PRESENT);
    bool pin26_high = digitalRead(GpioPins::KEY_NOT_PRESENT);
    
    if (pin25_high && pin26_high)
    {
        _current_key_state = KeyState::Inactive; // Both pins HIGH - invalid state
    }
    else if (pin25_high)
    {
        _current_key_state = KeyState::Present;
    }
    else if (pin26_high)
    {
        _current_key_state = KeyState::NotPresent;
    }
    else
    {
        _current_key_state = KeyState::Inactive; // Both pins LOW
    }
}

/// @brief Load the key panel UI components
void KeyPanel::load(std::function<void()> callback_function)
{
    log_d("...");
    _callback_function = callback_function;

    _key_component->render(_screen, _center_location);
    _key_component->refresh(Reading{static_cast<int32_t>(_current_key_state)});
    
    lv_obj_add_event_cb(_screen, KeyPanel::show_panel_completion_callback, LV_EVENT_SCREEN_LOADED, this);

    log_v("loading...");
    lv_screen_load(_screen);
}

/// @brief Update the key panel with current sensor data
void KeyPanel::update(std::function<void()> callback_function)
{
    log_d("...");

    // Note: KeyPanel doesn't need to update key state since the trigger system
    // handles state changes and will restore/switch panels automatically
    // The panel just displays the key state it was initialized with
    
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