#include "panels/splash_panel.h"
#include "managers/style_manager.h"

// Constructors and Destructors

SplashPanel::SplashPanel(IComponentFactory* componentFactory)
    : componentFactory_(componentFactory)
{
    // Component will be created during load() method using the component factory
}

SplashPanel::~SplashPanel()
{
    if (screen_)
    {
        lv_obj_del(screen_);
    }

    if (blankScreen_)
    {
        lv_obj_del(blankScreen_);
    }

    if (component_)
    {
        component_.reset();
    }
}

// Core Functionality Methods

/// @brief Initialize the screen with component
/// Creates blank screens for animation transitions
void SplashPanel::init(IGpioProvider* gpio, IDisplayProvider* display)
{
    log_d("Initializing splash panel screen and animation components");
    if (!display) {
        log_e("SplashPanel requires display provider");
        return;
    }

    blankScreen_ = display->createScreen();
    screen_ = display->createScreen();
}

/// @brief Show the screen
/// @param callbackFunction the function to call when the splash screen is complete
void SplashPanel::load(std::function<void()> callbackFunction, IGpioProvider* gpio, IDisplayProvider* display)
{
    log_i("Loading splash panel with fade-in animation");

    callbackFunction_ = callbackFunction;

    // Create component using the injected component factory
    // The factory now has all required dependencies (style service and display provider) injected
    component_ = componentFactory_->createComponent("clarity");

    // Create location parameters for the splash component
    ComponentLocation splashLocation(LV_ALIGN_CENTER, 0, 0);

    // Render the component
    component_->render(screen_, splashLocation, display);
    lv_timer_t *transition_timer = lv_timer_create(SplashPanel::fade_in_timer_callback, 100, this);
}

/// @brief Update the reading on the screen
void SplashPanel::update(std::function<void()> callbackFunction, IGpioProvider* gpio, IDisplayProvider* display)
{
    // Immediately call the completion callback so that lock/unlock logic is processed
    callbackFunction();
}

// Static Callback Methods

/// @brief Callback for when the animation is complete
/// @param animation_timer the animation_timer that has completed
void SplashPanel::animation_complete_timer_callback(lv_timer_t *animationTimer)
{
    log_d("Animation sequence complete - executing callback");

    // Get the splash panel instance
    auto *thisInstance = static_cast<SplashPanel *>(lv_timer_get_user_data(animationTimer));
    thisInstance->callbackFunction_();

    // Delete the animation_timer
    lv_timer_del(animationTimer);
}

/// @brief Callback function for the fade in fade_in_timer completion
/// @param fade_in_timer the fade_in_timer that has completed
void SplashPanel::fade_in_timer_callback(lv_timer_t *fadeInTimer)
{
    log_d("Animation sequence complete - executing callback");

    // Get the screen pointer that was added to the user data
    auto *panel = static_cast<SplashPanel *>(lv_timer_get_user_data(fadeInTimer));

    log_v("Fading in...");
    lv_screen_load_anim(panel->screen_,
                        LV_SCR_LOAD_ANIM_FADE_IN,
                        _ANIMATION_TIME,
                        _DELAY_TIME,
                        false);

    // Schedule the fade-out animation
    auto *fadeOutTimer = lv_timer_create(SplashPanel::fade_out_timer_callback,
                                           _ANIMATION_TIME + _DISPLAY_TIME,
                                           panel);

    // Remove the fade_in_timer after transition
    lv_timer_del(fadeInTimer);
}

/// @brief Callback function for the fade out animation_timer completion
/// @param fade_out_timer the animation_timer that has completed
void SplashPanel::fade_out_timer_callback(lv_timer_t *fadeOutTimer)
{
    log_d("Animation sequence complete - executing callback");

    // Get the splash panel instance
    auto *panel = static_cast<SplashPanel *>(lv_timer_get_user_data(fadeOutTimer));

    log_v("Fading out...");
    lv_scr_load_anim(panel->blankScreen_,
                     LV_SCR_LOAD_ANIM_FADE_OUT,
                     _ANIMATION_TIME,
                     0,
                     false);

    // Create a animation_timer for the completion callback
    auto *completionTimer = lv_timer_create(SplashPanel::animation_complete_timer_callback,
                                             _ANIMATION_TIME + _DELAY_TIME, // Small extra delay to ensure animation is complete
                                             panel);

    // Remove the fade_out_timer after transition, this replaces having to set a repeat on the animation_timer
    lv_timer_del(fadeOutTimer);
}