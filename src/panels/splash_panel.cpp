#include "panels/splash_panel.h"

/// @brief SplashPanel constructor, generates a component
SplashPanel::SplashPanel()
{
    _clarity_component = std::make_shared<ClarityComponent>();
}

/// @brief Set the function to be called on completion of this panel animations
/// @param callback_function the function to be executed when animation is complete
void SplashPanel::set_completion_callback(std::function<void()> callback_function)
{
    _callback_function = callback_function;
}

/// @brief Initialize the screen with component
/// @param device the device housing the screens
void SplashPanel::init(IDevice *device)
{
    SerialLogger().log_point("SplashPanel::init()", "Entry...");

    _device = device;

    // create blank screen to be loaded initially, and faded out to again at the end
    _blank_screen = lv_obj_create(NULL);
    LvTools().init_blank_screen(_blank_screen);

    // Create splash screen
    _screen = lv_obj_create(NULL);
    _clarity_component->init(_screen);
}

/// @brief Show the screen
void SplashPanel::show()
{
    SerialLogger().log_point("SplashPanel::show()", "Entry...");

    // Initially show a blank screen, than fade in the splash
    lv_scr_load(_blank_screen);

    lv_timer_t *transition_timer = lv_timer_create(SplashPanel::fade_in_timer_callback, 100, this);
}

/// @brief Callback function for the fade in timer completion
/// @param timer the timer that has completed
void SplashPanel::fade_in_timer_callback(lv_timer_t *timer)
{
    SerialLogger().log_point("SplashPanel::fade_in_timer_callback()", "Entry...");

    // Get the screen pointer that was added to the user data
    SplashPanel *panel = static_cast<SplashPanel *>(lv_timer_get_user_data(timer));

    // Transition to the next screen with a fade-in animation
    lv_scr_load_anim(panel->_screen,
                     LV_SCR_LOAD_ANIM_FADE_IN,
                     ANIMATION_TIME,
                     DELAY_TIME,
                     false);

    // Schedule the fade-out animation
    lv_timer_t *fade_out_timer = lv_timer_create(SplashPanel::fade_out_timer_callback,
                                                 ANIMATION_TIME + DISPLAY_TIME,
                                                 panel);

    // Remove the timer after transition, this replaces having to set a repeat on the timer
    lv_timer_del(timer);
}

/// @brief Callback function for the fade out timer completion
/// @param timer the timer that has completed
void SplashPanel::fade_out_timer_callback(lv_timer_t *timer)
{
    SerialLogger().log_point("SplashPanel::fade_out_timer_callback()", "Entry...");

    // Get the splash panel instance
    SplashPanel *panel = static_cast<SplashPanel *>(lv_timer_get_user_data(timer));

    // Fade out back to blank screen
    lv_scr_load_anim(panel->_blank_screen,
                     LV_SCR_LOAD_ANIM_FADE_OUT,
                     ANIMATION_TIME,
                     0,
                     false);

    // Create a timer for the completion callback
    lv_timer_t *completion_timer = lv_timer_create(SplashPanel::animation_complete_timer_callback,
                                                   ANIMATION_TIME + 50, // Small extra delay to ensure animation is complete
                                                   panel);

    // Remove the timer after transition, this replaces having to set a repeat on the timer
    lv_timer_del(timer);
}

/// @brief Callback for when the animation is complete
void SplashPanel::animation_complete_timer_callback(lv_timer_t *timer)
{
    SerialLogger().log_point("SplashPanel::animation_complete_timer_callback()", "Entry...");

    // Get the splash panel instance
    SplashPanel *panel = static_cast<SplashPanel *>(lv_timer_get_user_data(timer));

    // Call the completion callback if it exists
    if (panel->_callback_function)
    {
        SerialLogger().log_point("SplashPanel::animation_complete_timer_callback()", "Executing splash callback");
        panel->_callback_function();
    }
    else
        SerialLogger().log_point("SplashPanel::animation_complete_timer_callback()", "No callback set");

    // Delete the timer
    lv_timer_del(timer);
}

/// @brief Update the reading on the screen
void SplashPanel::update()
{
    // Not needed but required to satisfy interface
}

/// @brief SplashPanel destructor to clean up dynamically allocated objects
SplashPanel::~SplashPanel()
{
    // LVGL screens will be automatically deleted by LVGL
    // when new screens are loaded, no need to delete them here
}