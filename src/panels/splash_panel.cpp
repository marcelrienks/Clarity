#include "panels/splash_panel.h"

/// @brief SplashPanel constructor, generates a component and sensor
SplashPanel::SplashPanel()
{
    _component = new ClarityComponent();
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
    Ticker().handle_lv_tasks();//TODO: confirm if this is required
    
    lv_timer_t *transition_timer = lv_timer_create(SplashPanel::fade_in_timer_callback, 0, _screen);

    SerialLogger().log_point("SplashPanel::show()", "Completed");
}

void SplashPanel::fade_in_timer_callback(lv_timer_t *timer)
{
    SerialLogger().log_point("SplashPanel::fade_in_timer_callback()", "Entry");

    // Get the screen pointer that was added to the user data
    lv_obj_t *screen = (lv_obj_t*)lv_timer_get_user_data(timer);

    // Transition to the next screen with a fade-in animation
    lv_scr_load_anim(screen, LV_SCR_LOAD_ANIM_FADE_IN, ANIMATION_TIME, DELAY_TIME, false);
    Ticker().handle_lv_tasks();//TODO: confirm if this is required

    // Remove the timer after transition, this replaces having to set a repeat on the timer
    lv_timer_del(timer);

    SerialLogger().log_point("SplashPanel::fade_in_timer_callback()", "Completed");
}

void SplashPanel::fade_out_timer_callback(lv_timer_t *timer)
{
    SerialLogger().log_point("SplashPanel::fade_out_timer_callback()", "Entry");

    // Get the screen pointer that was added to the user data
    lv_obj_t *screen = (lv_obj_t*)lv_timer_get_user_data(timer);

    // Transition to the next screen with a fade-in animation
    lv_scr_load_anim(screen, LV_SCR_LOAD_ANIM_FADE_IN, ANIMATION_TIME, DISPLAY_TIME, false);
    Ticker().handle_lv_tasks();//TODO: confirm if this is required

    // Remove the timer after transition, this replaces having to set a repeat on the timer
    lv_timer_del(timer);

    SerialLogger().log_point("SplashPanel::fade_out_timer_callback()", "Completed");
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