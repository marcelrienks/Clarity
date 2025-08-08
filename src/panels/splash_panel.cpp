#include "panels/splash_panel.h"
#include "factories/ui_factory.h"
#include "managers/style_manager.h"
#include "managers/trigger_manager.h"

// Constructors and Destructors

SplashPanel::SplashPanel(IGpioProvider* gpio, IDisplayProvider* display, IStyleService* styleService)
    : gpioProvider_(gpio), displayProvider_(display), styleService_(styleService),
      animationSkipped_(false), currentTimer_(nullptr)
{
    // Component will be created during load() method
}

SplashPanel::~SplashPanel()
{
    // Clean up any active timer
    if (currentTimer_)
    {
        lv_timer_del(currentTimer_);
        currentTimer_ = nullptr;
    }

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
void SplashPanel::Init(IGpioProvider* gpio, IDisplayProvider* display)
{
    log_d("Initializing splash panel screen and animation components");
    if (!display) {
        log_e("SplashPanel requires display provider");
        return;
    }

    blankScreen_ = display->CreateScreen();
    screen_ = display->CreateScreen();
}

/// @brief Show the screen
/// @param callbackFunction the function to call when the splash screen is complete
void SplashPanel::Load(std::function<void()> callbackFunction, IGpioProvider* gpio, IDisplayProvider* display)
{
    log_i("Loading splash panel with fade-in animation");

    callbackFunction_ = callbackFunction;

    // Create component directly using UIFactory
    component_ = UIFactory::createClarityComponent(styleService_);

    // Create location parameters for the splash component
    ComponentLocation splashLocation(LV_ALIGN_CENTER, 0, 0);

    // Render the component
    component_->Render(screen_, splashLocation, display);
    
    // Store timer reference and reset skip flag
    animationSkipped_ = false;
    currentTimer_ = lv_timer_create(SplashPanel::fade_in_timer_callback, 100, this);
}

/// @brief Update the reading on the screen
void SplashPanel::Update(std::function<void()> callbackFunction, IGpioProvider* gpio, IDisplayProvider* display)
{
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

    // Schedule the fade-out animation (Display time added to allow a short little period where the logo is fully visible, this does not happen with fade out)
    auto *fadeOutTimer = lv_timer_create(SplashPanel::fade_out_timer_callback,
                                           _ANIMATION_TIME + _DELAY_TIME + _DISPLAY_TIME,
                                           panel);
    panel->currentTimer_ = fadeOutTimer;

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
    panel->currentTimer_ = completionTimer;

    // Remove the fade_out_timer after transition, this replaces having to set a repeat on the animation_timer
    lv_timer_del(fadeOutTimer);
}

// IInputService Interface Implementation

void SplashPanel::OnShortPress()
{
    // Short press skips animation and loads default panel immediately
    if (animationSkipped_)
    {
        log_d("SplashPanel: Animation already skipped");
        return;
    }
    
    log_i("SplashPanel: Short press - skipping animation");
    animationSkipped_ = true;
    
    // Cancel current timer if exists
    if (currentTimer_)
    {
        lv_timer_del(currentTimer_);
        currentTimer_ = nullptr;
    }
    
    // Load blank screen immediately without animation
    lv_scr_load(blankScreen_);
    
    // Execute callback to load default panel
    callbackFunction_();
}

void SplashPanel::OnLongPress()
{
    // Long press: Handled by InputManager action system
    // This method exists to satisfy IInputService interface but does nothing
}