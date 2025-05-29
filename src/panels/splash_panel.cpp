#include "panels/splash_panel.h"

SplashPanel::SplashPanel()
    : _component(std::make_shared<ClarityComponent>()) {}

SplashPanel::~SplashPanel()
{
    if (_screen)
        lv_obj_del(_screen);

    if (_blank_screen)
        lv_obj_del(_blank_screen);

    if (_component)
        _component.reset();
}

/// @brief Initialize the screen with component
/// @param device the device housing the screens
void SplashPanel::init()
{
    log_d("...");

    _blank_screen = LvTools::create_blank_screen();
    _screen = LvTools::create_blank_screen();
}

/// @brief Show the screen
/// @param show_panel_completion_callback the function to call when the splash screen is complete
void SplashPanel::load(std::function<void()> show_panel_completion_callback)
{
    log_i("...");
    _callback_function = show_panel_completion_callback;

    _component->render_show(_screen);
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
    log_d("fade_in_timer_callback start");

    // Get the screen pointer that was added to the user data
    auto *panel = static_cast<SplashPanel *>(lv_timer_get_user_data(fade_in_timer));
    if (!panel) {
        log_e("Panel is null!");
        return;
    }
    log_d("Panel retrieved successfully");

    if (!panel->_screen) {
        log_e("Panel screen is null!");
        return;
    }
    log_d("Panel screen is valid");

    // Try without animation first to isolate the issue
    log_d("About to call lv_scr_load_anim");
    
    // Replace lv_scr_load_anim with regular lv_scr_load to test
    lv_scr_load(panel->_screen);
    log_d("Screen loaded successfully");

    // Schedule the fade-out animation
    log_d("Creating fade_out_timer");
    auto *fade_out_timer = lv_timer_create(SplashPanel::fade_out_timer_callback,
                                           _animation_time + _display_time,
                                           panel);
    if (!fade_out_timer) {
        log_e("Failed to create fade_out_timer!");
        return;
    }
    log_d("fade_out_timer created successfully");

    // Remove the fade_in_timer after transition
    log_d("Deleting fade_in_timer");
    lv_timer_del(fade_in_timer);
    log_d("fade_in_timer deleted, callback complete");
}

/// @brief Callback function for the fade out animation_timer completion
/// @param fade_out_timer the animation_timer that has completed
void SplashPanel::fade_out_timer_callback(lv_timer_t *fade_out_timer)
{
    log_d("...");

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
    // Get the splash panel instance
    auto *this_instance = static_cast<SplashPanel *>(lv_timer_get_user_data(animation_timer));

    log_v("Executing splash callback");
    this_instance->_callback_function();

    // Delete the animation_timer
    lv_timer_del(animation_timer);
}