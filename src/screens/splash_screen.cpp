#include "screens/splash_screen.h"

/// @brief SplashScreen constructor, generates a component and sensor
SplashScreen::SplashScreen()
{
    _component = new ClarityComponent();
}

/// @brief Initialize the screen with component and sensor
void SplashScreen::init(IDevice *device)
{
    _device = device;

    this->_virtual_screen = lv_screen_active();
    _component->init(this->_virtual_screen);

    SerialLogger().log_point("SplashScreen::init()", "Completed");
}

/// @brief Show the screen
void SplashScreen::show()
{
    SerialLogger().log_point("SplashScreen::show()", "Entry");
    SplashScreen::fade_in_splash_screen();

    lv_timer_t *splash_timer = lv_timer_create([](lv_timer_t *anonymous_splash_timer){
        SerialLogger().log_point("SplashScreen::show()", "splash_timer Entry");

        // Get the instance, and fade out the splash screen
        SplashScreen *splash_timer_instance = static_cast<SplashScreen*>(lv_timer_get_user_data(anonymous_splash_timer));
        splash_timer_instance->SplashScreen::fade_out_splash_screen();

        lv_timer_t *blank_timer = lv_timer_create([](lv_timer_t *anonymous_blank_timer){
            SerialLogger().log_point("SplashScreen::show()", "blank_timer Entry");

            // Get the instance, and mark the flag to enable looping
            SplashScreen *blank_timer_instance = static_cast<SplashScreen*>(lv_timer_get_user_data(anonymous_blank_timer));
            blank_timer_instance->_device->_is_splash_complete = true;

        }, (ANIMATION_TIME + DISPLAY_TIME), splash_timer_instance);// Note: this timer must run for both animation and display times
        lv_timer_set_repeat_count(blank_timer, 1);

    }, ANIMATION_TIME, this);
    lv_timer_set_repeat_count(splash_timer, 1);

    SerialLogger().log_point("SplashScreen::show()", "Completed");
}

void SplashScreen::fade_in_splash_screen() {
    SerialLogger().log_point("SplashScreen::fade_in_splash_screen()", "fade_in_splash_screen Entry");

    lv_scr_load_anim(_virtual_screen, LV_SCR_LOAD_ANIM_FADE_IN, ANIMATION_TIME, DELAY_TIME, false);
    
    SerialLogger().log_point("SplashScreen::fade_in_splash_screen()", "splash animated");
}

void SplashScreen::fade_out_splash_screen() {
    SerialLogger().log_point("SplashScreen::fade_out_splash_screen()", "fade_out_splash_screen Entry");

    auto *blank = lv_obj_create(NULL);//TODO: rather than creating a new screen, can I reload the original blank? Possibly by having that passed in, and then creating and deleting a new one for splash
    LvTools().init_blank_screen(blank);
    lv_scr_load_anim(blank, LV_SCR_LOAD_ANIM_FADE_IN, ANIMATION_TIME, DISPLAY_TIME, false);

    SerialLogger().log_point("SplashScreen::fade_out_splash_screen()", "blank animated");
}

/// @brief Update the reading on the screen
void SplashScreen::update()
{
    // Not needed but required to satisfy interface
}

/// @brief SplashScreen destructor to clean up dynamically allocated objects
SplashScreen::~SplashScreen()
{
    if (_component)
        delete _component;

    // if (_transition_timer)
    //     lv_timer_del(_transition_timer);
}