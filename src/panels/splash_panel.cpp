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

    // Initially load the blank screen without animation
    lv_scr_load(_blank_screen); // TODO: is this required, this already happens in device

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

    else
        SerialLogger().log_point("SplashPanel::animation_callback()", "ERROR: Panel is null");
}

/// @brief Handles the dequence of animations to run through in order to show a splash screen
void SplashPanel::run_animation_workflow_handler()
{
    /*
     * NOTE this function manually builds an animation, rather than using the lv_scr_load_anim() function in order to leverage callbacks
     * If lv_scr_load_anim() function is used to simplify logic, you have to implement timers, matching the animation time.
     * The benefit of below benefits from having callbacks wired up which I think is safer
     */

    SerialLogger().log_point("SplashPanel::run_animation_workflow_handler()", "Sequence: " + String(_animation_sequence));

    switch (_animation_sequence)
    {
    case 0:
    {
        // Step 1: Fade in splash screen
        lv_anim_t fade_in_animation;
        lv_anim_init(&fade_in_animation);
        lv_anim_set_var(&fade_in_animation, _screen);
        lv_anim_set_time(&fade_in_animation, ANIMATION_TIME);
        lv_anim_set_delay(&fade_in_animation, 0);
        lv_anim_set_user_data(&fade_in_animation, this);
        lv_anim_set_ready_cb(&fade_in_animation, animation_callback);

        // Create a proper fade-in effect
        lv_anim_set_values(&fade_in_animation, LV_OPA_TRANSP, LV_OPA_COVER);
        lv_anim_set_exec_cb(&fade_in_animation, [](void *obj, int32_t value)
                            { lv_obj_set_style_opa(static_cast<lv_obj_t *>(obj), value, 0); });

        // Load the screen first without animation
        lv_scr_load(_screen);

        // Initially make it transparent
        lv_obj_set_style_opa(_screen, LV_OPA_TRANSP, 0);

        // Start the animation
        lv_anim_start(&fade_in_animation);

        _animation_sequence++;
        break;
    }

    case 1:
    {
        // Step 2: Display time - pause before fading out
        lv_anim_t display_animation;
        lv_anim_init(&display_animation);
        lv_anim_set_var(&display_animation, _screen);
        lv_anim_set_time(&display_animation, DISPLAY_TIME);
        lv_anim_set_user_data(&display_animation, this);
        lv_anim_set_ready_cb(&display_animation, animation_callback);

        // This animation doesn't change anything visually, it's just a timer
        lv_anim_set_values(&display_animation, 0, 0);
        lv_anim_set_exec_cb(&display_animation, [](void *obj, int32_t value)
                            {
                                // Do nothing, just waiting
                            });

        lv_anim_start(&display_animation);

        _animation_sequence++;
        break;
    }

    case 2:
    {
        // Step 3: Fade out splash screen
        lv_anim_t fade_out_animation;
        lv_anim_init(&fade_out_animation);
        lv_anim_set_var(&fade_out_animation, _screen);
        lv_anim_set_time(&fade_out_animation, ANIMATION_TIME);
        lv_anim_set_delay(&fade_out_animation, 0);
        lv_anim_set_user_data(&fade_out_animation, this);
        lv_anim_set_ready_cb(&fade_out_animation, animation_callback);

        // Create a proper fade-out effect
        lv_anim_set_values(&fade_out_animation, LV_OPA_COVER, LV_OPA_TRANSP);
        lv_anim_set_exec_cb(&fade_out_animation, [](void *obj, int32_t value)
                            { lv_obj_set_style_opa(static_cast<lv_obj_t *>(obj), value, 0); });

        lv_anim_start(&fade_out_animation);

        _animation_sequence++;
        break;
    }

    case 3:
    {
        // Step 4: Animation sequence complete
        SerialLogger().log_point("SplashPanel::run_animation_workflow_handler()", "Animation sequence complete");

        // Switch back to blank screen
        lv_scr_load(_blank_screen);

        // Set the splash complete flag
        _device->_is_splash_complete = true;

        // Call the callback if set
        if (_callback_function)
            _callback_function();

        else
            SerialLogger().log_point("SplashPanel::run_animation_workflow_handler()", "No callback set");

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