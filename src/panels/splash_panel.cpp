#include "panels/splash_panel.h"

/// @brief SplashPanel constructor, generates a component and sensor
SplashPanel::SplashPanel()
{
    _component = new ClarityComponent();
}

/// @brief Initialize the screen with component and sensor
void SplashPanel::init(IDevice *device)
{
    SerialLogger().log_point("SplashPanel::init()", "Entry");
    _device = device;

    // create blank screen to be loaded initially, and faded out to again at the end
    _blank_screen = lv_obj_create(NULL);
    LvTools().init_blank_screen(_blank_screen);

    // Create splash screen
    _screen = lv_obj_create(NULL);
    _component->init(_screen);

    SerialLogger().log_point("SplashPanel::init()", "Completed");
}

/// @brief Set the function to be called on completion of this panel animations
/// @param callback_function the function to be executed when animation is complete
void SplashPanel::set_callback(std::function<void()> callback_function)
{
    _callback_function = callback_function;
}

/// @brief Register an animation
/// @param screen the screen to be animated
/// @param screen_load the animation to be applied
/// @param animation_time time taken to complete animation
/// @param delay_time delay before starting animation
/// @param _callback_function the callback to be called when animation is complete
void SplashPanel::register_animation(lv_obj_t *screen, lv_screen_load_anim_t screen_load, uint32_t animation_time, uint32_t delay_time, lv_anim_completed_cb_t _callback_function)
{
    // Create an animation
    lv_anim_t animation;
    lv_anim_init(&animation);

    if (_callback_function)
    {
        SerialLogger().log_point("SplashPanel::register_animation()", "call back set");
        lv_anim_set_ready_cb(&animation, _callback_function);
    }

    SerialLogger().log_point("SplashPanel::register_animation()", "animation registered");
    lv_scr_load_anim(screen, screen_load, animation_time, delay_time, &animation);
}

/// @brief Register all screens that need to be animated in order, and handle lv tasks
void SplashPanel::show()
{
    SerialLogger().log_point("SplashPanel::show()", "Entry");

    // register screen animations in specific order so that they are handled in order
    SplashPanel::register_animation(_blank_screen, LV_SCR_LOAD_ANIM_NONE, 0, 0, nullptr);
    SplashPanel::register_animation(_screen, LV_SCR_LOAD_ANIM_FADE_IN, ANIMATION_TIME, DELAY_TIME, nullptr);
    SplashPanel::register_animation(_blank_screen, LV_SCR_LOAD_ANIM_FADE_OUT, ANIMATION_TIME, DELAY_TIME, nullptr);

    Ticker::handle_lv_tasks();
    SerialLogger().log_point("SplashPanel::show()", "Completed");
}

/// @brief Callback to be called when the last screen animation is completed
/// @param animation user data object
void SplashPanel::animation_complete_callback(lv_anim_t *animation)
{
    SerialLogger().log_point("SplashPanel::animation_complete_callback()", "Entry");

    SplashPanel *panel = static_cast<SplashPanel *>(lv_anim_get_user_data(animation));
    if (panel->_callback_function)
        panel->_callback_function();

    else
        SerialLogger().log_point("SplashPanel::animation_completion_callback()", "No callback set");

    SerialLogger().log_point("SplashPanel::animation_complete_callback()", "Completed");
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