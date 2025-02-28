#include "panels/splash_panel.h"

/// @brief SplashPanel constructor, generates a component and sensor
SplashPanel::SplashPanel()
{
    _component = new ClarityComponent();
}

/// @brief Set the callback to be called when splash animation completes
void SplashPanel::set_completion_callback(PanelCompletionCallback callback)
{
    _completion_callback = callback;
    SerialLogger().log_point("SplashPanel::set_completion_callback()", "Callback set");
}

/// @brief Initialize the screen with component and sensor
void SplashPanel::init(IDevice *device)
{
    _device = device;

    // create blank screen to be loaded initially, and faded out to again at the end
    _blank_screen = lv_obj_create(NULL);
    LvTools().init_blank_screen(_blank_screen);

    // Create splash screen
    _screen = lv_obj_create(NULL);
    _component->init(_screen);

    SerialLogger().log_point("SplashPanel::init()", "Completed");
}

/// @brief Show the screen
void SplashPanel::show()
{
    SerialLogger().log_point("SplashPanel::show()", "Entry");

    // Initially show a blank screen, than fade in the splash
    lv_scr_load(_blank_screen);

    lv_timer_t *transition_timer = lv_timer_create(SplashPanel::fade_in_timer_callback, 100, this);

    SerialLogger().log_point("SplashPanel::show()", "Completed");
}

void SplashPanel::fade_in_timer_callback(lv_timer_t *timer)
{
    SerialLogger().log_point("SplashPanel::fade_in_timer_callback()", "Entry");

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

    SerialLogger().log_point("SplashPanel::fade_in_timer_callback()", "Completed");
}

void SplashPanel::fade_out_timer_callback(lv_timer_t *timer)
{
    SerialLogger().log_point("SplashPanel::fade_out_timer_callback()", "Entry");

    // Get the splash panel instance
    SplashPanel *panel = static_cast<SplashPanel *>(lv_timer_get_user_data(timer));

    // Fade out back to blank screen
    lv_scr_load_anim(panel->_blank_screen,
                     LV_SCR_LOAD_ANIM_FADE_OUT,
                     ANIMATION_TIME,
                     0,
                     false);

    // Create a timer for the completion callback
    lv_timer_t *completion_timer = lv_timer_create(SplashPanel::animation_completion_callback,
                                                   ANIMATION_TIME + 50, // Small extra delay to ensure animation is complete
                                                   panel);

    // Remove the timer after transition, this replaces having to set a repeat on the timer
    lv_timer_del(timer);

    SerialLogger().log_point("SplashPanel::fade_out_timer_callback()", "Completed");
}

/// @brief Callback for when the animation is complete
void SplashPanel::animation_completion_callback(lv_timer_t *timer)
{
    SerialLogger().log_point("SplashPanel::animation_completion_callback()", "Entry");

    // Get the splash panel instance
    SplashPanel *panel = static_cast<SplashPanel *>(lv_timer_get_user_data(timer));

    SerialLogger().log_point("SplashPanel::animation_completion_callback()",
                             "Setting _is_splash_complete flag to true. Device address: " +
                                 String((uintptr_t)panel->_device, HEX));

    // Debug check in the callback
    bool current_value = panel->_device->_is_splash_complete;
    panel->_device->_is_splash_complete = true;
    bool after_value = panel->_device->_is_splash_complete;
    SerialLogger().log_point("Flag Debug", "Before: " + String(current_value) +
                                               " After: " + String(after_value));

    // Call the completion callback if it exists
    if (panel->_completion_callback)
    {
        SerialLogger().log_point("SplashPanel::animation_completion_callback()", "Executing callback");
        panel->_completion_callback();
    }
    else
        SerialLogger().log_point("SplashPanel::animation_completion_callback()", "No callback set");

    // Delete the timer
    lv_timer_del(timer);

    SerialLogger().log_point("SplashPanel::animation_completion_callback()", "Completed");
}

/// @brief Update the reading on the screen
void SplashPanel::update()
{
    // Not needed but required to satisfy interface
}

/// @brief SplashPanel destructor to clean up dynamically allocated objects
SplashPanel::~SplashPanel()
{
    if (_component)
        delete _component;

    // if (_transition_timer)
    //     lv_timer_del(_transition_timer);
}