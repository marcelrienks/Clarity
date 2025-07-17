#include "triggers/lights_trigger.h"
#include "managers/style_manager.h"
#include <variant>
#include <lvgl.h>

// Constructors and Destructors

/// @brief Constructor for LightsTrigger
/// @param enable_restoration Restoration flag (unused for lights trigger)
LightsTrigger::LightsTrigger(bool enable_restoration)
    : _last_lights_state(false) // Initialize to lights off/day theme (false)
{
    // Lights trigger doesn't use restoration functionality
    // Parameter is ignored but required for template compatibility
    (void)enable_restoration;
}

// Core Functionality Methods

/// @brief Initialize the lights trigger and sensor
void LightsTrigger::init()
{
    log_d("...");

    // Initialize the lights sensor
    _lights_sensor.init();

    // Read initial lights state
    auto reading = _lights_sensor.get_reading();
    _last_lights_state = std::get<bool>(reading);

    // Apply initial theme based on lights state
    Themes initial_theme = _last_lights_state ? Themes::Night : Themes::Day;
    log_i("Setting initial theme: %s", _last_lights_state ? "Night" : "Day");
    StyleManager::get_instance().set_theme(initial_theme);
}

/// @brief Evaluate lights switch state and apply theme changes
/// @return bool indicating if theme changed (always false for panel switching)
bool LightsTrigger::evaluate()
{
    log_d("...");

    // Read current lights switch state
    auto reading = _lights_sensor.get_reading();
    bool current_lights_state = std::get<bool>(reading);

    // Check if lights state has changed
    if (current_lights_state != _last_lights_state)
    {
        log_i("Lights switch state changed: %s -> %s",
              _last_lights_state ? "Night" : "Day",
              current_lights_state ? "Night" : "Day");

        // Apply new theme
        Themes new_theme = current_lights_state ? Themes::Night : Themes::Day;
        StyleManager::get_instance().set_theme(new_theme);

        // Apply theme to current screen
        lv_obj_t *current_screen = lv_scr_act();
        if (current_screen != nullptr)
        {
            StyleManager::get_instance().apply_theme_to_screen(current_screen);
        }

        // Update last state
        _last_lights_state = current_lights_state;

        log_i("Theme changed to: %s", current_lights_state ? "Night" : "Day");
    }

    // This trigger does not change panels, only themes
    // Always return false to prevent panel switching
    return false;
}

/// @brief Get target panel (none for lights trigger)
/// @return nullptr since lights trigger doesn't change panels
const char *LightsTrigger::get_target_panel() const
{
    return nullptr;
}

/// @brief Get unique trigger identifier
/// @return Trigger ID string
const char *LightsTrigger::get_id() const
{
    return TRIGGER_ID;
}

/// @brief Check if trigger supports restoration
/// @return false since lights trigger doesn't change panels
bool LightsTrigger::should_restore() const
{
    return false;
}