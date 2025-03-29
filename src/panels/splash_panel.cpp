#include "panels/splash_panel.h"

SplashPanel::SplashPanel(PanelIteration panel_iteration)
    : _component(std::make_shared<ClarityComponent>()), _iteration(panel_iteration) {}

SplashPanel::~SplashPanel()
{
    if (_screen)
        lv_obj_clean(_screen);

    if (_blank_screen)
        lv_obj_clean(_blank_screen);

    if (_component)
        _component.reset();
}

/// @brief Initialize the screen with component
/// @param device the device housing the screens
void SplashPanel::init(IDevice *device)
{
    SerialLogger().log_point("SplashPanel::init()", "...");

    _device = device;
    _blank_screen = LvTools::create_blank_screen();
    _screen = LvTools::create_blank_screen();
    _component->render_show(_screen);
}

/// @brief Show the screen
/// @param show_panel_completion_callback the function to call when the splash screen is complete
void SplashPanel::show(std::function<void()> show_panel_completion_callback)
{
    SerialLogger().log_point("SplashPanel::show()", "...");

    _callback_function = show_panel_completion_callback;
    lv_timer_t *transition_timer = lv_timer_create(SplashPanel::fade_in_timer_callback, 100, this);
}

/// @brief Update the reading on the screen
void SplashPanel::update(std::function<void()> update_panel_completion_callback)
{
    // Immediately call the completion callback so that lock/unlock logic is processed
    update_panel_completion_callback();
}

/// @brief Callback function for the fade in fade_in_timer completion
/// @param fade_in_timer the fade_in_timer that has completed
void SplashPanel::fade_in_timer_callback(lv_timer_t *fade_in_timer)
{
    SerialLogger().log_point("SplashPanel::fade_in_timer_callback()", "...");

    // Get the screen pointer that was added to the user data
    auto *panel = static_cast<SplashPanel *>(lv_timer_get_user_data(fade_in_timer));

    // Transition to the next screen with a fade-in animation
    lv_scr_load_anim(panel->_screen,
                     LV_SCR_LOAD_ANIM_FADE_IN,
                     _animation_time,
                     _delay_time,
                     false);

    // Schedule the fade-out animation
    auto *fade_out_timer = lv_timer_create(SplashPanel::fade_out_timer_callback,
                                           _animation_time + _display_time,
                                           panel);

    // Remove the fade_in_timer after transition, this replaces having to set a repeat on the fade_in_timer
    lv_timer_del(fade_in_timer);
}

/// @brief Callback function for the fade out animation_timer completion
/// @param fade_out_timer the animation_timer that has completed
void SplashPanel::fade_out_timer_callback(lv_timer_t *fade_out_timer)
{
    SerialLogger().log_point("SplashPanel::fade_out_timer_callback()", "...");

    // Get the splash panel instance
    auto *panel = static_cast<SplashPanel *>(lv_timer_get_user_data(fade_out_timer));

    // Fade out back to blank screen
    lv_scr_load_anim(panel->_blank_screen,
                     LV_SCR_LOAD_ANIM_FADE_OUT,
                     _animation_time,
                     0,
                     false);

    // Create a animation_timer for the completion callback
    auto *completion_timer = lv_timer_create(SplashPanel::animation_complete_timer_callback,
                                             _animation_time + 50, // Small extra delay to ensure animation is complete
                                             panel);

    // Remove the fade_out_timer after transition, this replaces having to set a repeat on the animation_timer
    lv_timer_del(fade_out_timer);
}

/// @brief Callback for when the animation is complete
/// @param animation_timer the animation_timer that has completed
void SplashPanel::animation_complete_timer_callback(lv_timer_t *animation_timer)
{
    SerialLogger().log_point("SplashPanel::animation_complete_timer_callback()", "...");

    // Get the splash panel instance
    auto *this_instance = static_cast<SplashPanel *>(lv_timer_get_user_data(animation_timer));

    // Call the completion callback if it exists
    if (this_instance->_callback_function)
    {
        SerialLogger().log_point("SplashPanel::animation_complete_timer_callback()", "Executing splash callback");
        this_instance->_callback_function();
    }
    else
        SerialLogger().log_point("SplashPanel::animation_complete_timer_callback()", "No callback set");

    // Delete the animation_timer
    lv_timer_del(animation_timer);
}