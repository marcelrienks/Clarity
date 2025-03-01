#include "panels/splash_panel.h"

/// @brief SplashPanel constructor, generates a component
SplashPanel::SplashPanel()
{
    _component = new ClarityComponent();
    _animation_sequence = 0;
}

/// @brief Initialize the screen with component and device
void SplashPanel::init(IDevice *device)
{
    SerialLogger().log_point("SplashPanel::init()", "Entry");
    _device = device;

    // Create splash screen
    _screen = lv_obj_create(NULL);
    _component->init(_screen);

    // Create blank screen to be loaded initially, and faded out to again at the end
    _blank_screen = lv_obj_create(NULL);
    LvTools().init_blank_screen(_blank_screen);

    SerialLogger().log_point("SplashPanel::init()", "Completed");
}

/// @brief Set the function to be called on completion of this panel animations
/// @param callback_function the function to be executed when animation is complete
void SplashPanel::set_callback(std::function<void()> callback_function)
{
    _callback_function = callback_function;
}

/// @brief Show the panel with sequential animations using callbacks
void SplashPanel::show()
{
    SerialLogger().log_point("SplashPanel::show()", "Entry");

    // Reset animation sequence counter
    _animation_sequence = 0;

    run_animation_workflow_handler();

    // Process LVGL tasks to start the animation immediately
    Ticker::handle_lv_tasks();

    SerialLogger().log_point("SplashPanel::show()", "Completed");
}

/// @brief Callback function for animation completion
/// @param animation Animation that just completed
void SplashPanel::animation_callback(lv_anim_t *animation)
{
    SerialLogger().log_point("SplashPanel::animation_callback()", "Entry");

    // Get the panel from user data
    auto *panel = static_cast<SplashPanel *>(lv_anim_get_user_data(animation));
    if (panel)
        panel->run_animation_workflow_handler();
}

/// @brief Handles the animation sequence
void SplashPanel::run_animation_workflow_handler()
{
    SerialLogger().log_point("SplashPanel::start_next_animation()", "Sequence: " + String(_animation_sequence));

    switch (_animation_sequence)
    {
    case 0:
    {
        // Step 1: Fade in splash screen
        lv_anim_t fade_in_animation;
        lv_anim_init(&fade_in_animation);
        lv_anim_set_user_data(&fade_in_animation, this);
        lv_anim_set_ready_cb(&fade_in_animation, animation_callback);
        lv_scr_load_anim(_screen, LV_SCR_LOAD_ANIM_FADE_IN, ANIMATION_TIME, 100, &fade_in_animation);
        _animation_sequence++;
        break;
    }

    case 1:
    {
        // Step 2: Fade out splash screen
        lv_anim_t fade_out_animation;
        lv_anim_init(&fade_out_animation);
        lv_anim_set_user_data(&fade_out_animation, this);
        lv_anim_set_ready_cb(&fade_out_animation, animation_callback);
        lv_scr_load_anim(_blank_screen, LV_SCR_LOAD_ANIM_FADE_ON, ANIMATION_TIME, DELAY_TIME, &fade_out_animation);
        _animation_sequence++;
        break;
    }

    case 2:
    {
        // Step 3: Animation sequence complete
        SerialLogger().log_point("SplashPanel::start_next_animation()", "Animation sequence complete");

        // Set the splash complete flag
        _device->_is_splash_complete = true;

        // Call the callback if set
        if (_callback_function)
            _callback_function();

        else
            SerialLogger().log_point("SplashPanel::animation_completion_callback()", "No callback set");

        break;
    }
    }
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
}