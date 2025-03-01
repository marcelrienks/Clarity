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
    
    // Initially set the screen as transparent
    lv_obj_set_style_opa(_screen, LV_OPA_TRANSP, 0);

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
    lv_scr_load(_blank_screen);

    // Reset the opacity of the splash screen before the animation starts
    lv_obj_set_style_opa(_screen, LV_OPA_TRANSP, 0);
    
    // Start the animation sequence
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

// Static timer callbacks for each animation stage
static void animation_timer_cb(lv_timer_t *timer) {
    SplashPanel *panel = static_cast<SplashPanel *>(lv_timer_get_user_data(timer));
    if (panel) {
        panel->advance_animation();
    }
    lv_timer_del(timer);
}

/// @brief Advances to the next animation step
void SplashPanel::advance_animation() {
    _animation_sequence++;
    run_animation_workflow_handler();
}

/// @brief Handles the sequence of animations to run through in order to show a splash screen
void SplashPanel::run_animation_workflow_handler()
{
    SerialLogger().log_point("SplashPanel::run_animation_workflow_handler()", "Sequence: " + String(_animation_sequence));

    switch (_animation_sequence)
    {
    case 0:
    {
        // Step 1: Fade in splash screen
        
        // Load the splash screen with fade-in animation
        lv_scr_load_anim(_screen, LV_SCR_LOAD_ANIM_FADE_IN, ANIMATION_TIME, 0, false);
        
        // Set up a timer for the next step
        lv_timer_t *timer = lv_timer_create(animation_timer_cb, ANIMATION_TIME, this);
        
        break;
    }

    case 1:
    {
        // Step 2: Display time - pause before fading out
        lv_timer_t *timer = lv_timer_create(animation_timer_cb, DISPLAY_TIME, this);
        
        break;
    }

    case 2:
    {
        // Step 3: Fade out splash screen to the blank screen
        lv_scr_load_anim(_blank_screen, LV_SCR_LOAD_ANIM_FADE_OUT, ANIMATION_TIME, 0, false);
        
        // Set up a timer for the next step
        lv_timer_t *timer = lv_timer_create(animation_timer_cb, ANIMATION_TIME, this);
        
        break;
    }

    case 3:
    {
        // Step 4: Animation sequence complete
        SerialLogger().log_point("SplashPanel::run_animation_workflow_handler()", "Animation sequence complete");

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