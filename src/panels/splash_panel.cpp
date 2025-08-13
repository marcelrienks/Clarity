#include "panels/splash_panel.h"
#include "factories/component_factory.h"
#include "interfaces/i_component_factory.h"
#include "managers/error_manager.h"
#include "managers/style_manager.h"
#include "managers/trigger_manager.h"
#include <algorithm>
#include <esp32-hal-log.h>

// Constructors and Destructors

SplashPanel::SplashPanel(IGpioProvider *gpio, IDisplayProvider *display, IStyleService *styleService,
                         IComponentFactory* componentFactory)
    : gpioProvider_(gpio), displayProvider_(display), styleService_(styleService), panelService_(nullptr),
      componentFactory_(componentFactory ? componentFactory : &ComponentFactory::Instance())
{
    // Component will be created during load() method
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
void SplashPanel::Init()
{
    // Initializing splash panel screen and animation components
    if (!displayProvider_)
    {
        log_e("SplashPanel requires display provider");
        ErrorManager::Instance().ReportCriticalError("SplashPanel", "Cannot initialize - display provider is null");
        return;
    }

    blankScreen_ = displayProvider_->CreateScreen();
    screen_ = displayProvider_->CreateScreen();
}

/// @brief Show the screen
/// @param callbackFunction the function to call when the splash screen is complete
void SplashPanel::Load(std::function<void()> callbackFunction)
{
    log_i("Loading splash panel with fade-in animation");

    callbackFunction_ = callbackFunction;
    
    // Set UIState to LOADING during splash animation to prevent action processing
    if (panelService_)
    {
        panelService_->SetUiState(UIState::LOADING);
    }

    // Create component using injected factory
    component_ = componentFactory_->CreateClarityComponent(styleService_);

    // Create location parameters for the splash component
    ComponentLocation splashLocation(LV_ALIGN_CENTER, 0, 0);

    // Render the component
    component_->Render(screen_, splashLocation, displayProvider_);

    // Start animation sequence with initial delay before fade-in
    lv_timer_create(SplashPanel::fade_in_timer_callback, 100, this);
}

/// @brief Update the reading on the screen
void SplashPanel::Update(std::function<void()> callbackFunction)
{
    callbackFunction();
}

// Static Callback Methods

/// @brief Callback for when the animation is complete
/// @param animation_timer the animation_timer that has completed
void SplashPanel::fading_out_timer_callback(lv_timer_t *animationTimer)
{
    // Animation sequence complete - executing callback

    // Get the splash panel instance
    auto *thisInstance = static_cast<SplashPanel *>(lv_timer_get_user_data(animationTimer));
    
    // Reset UIState to IDLE when splash animation completes
    if (thisInstance->panelService_)
    {
        thisInstance->panelService_->SetUiState(UIState::IDLE);
    }
    
    thisInstance->callbackFunction_();

    // Delete the animation_timer
    lv_timer_del(animationTimer);
}

/// @brief Callback function for the fade in fade_in_timer completion
/// @param fade_in_timer the fade_in_timer that has completed
void SplashPanel::fade_in_timer_callback(lv_timer_t *fadeInTimer)
{
    // Animation sequence complete - executing callback

    // Get the screen pointer that was added to the user data
    auto *panel = static_cast<SplashPanel *>(lv_timer_get_user_data(fadeInTimer));

    log_v("Fading in...");
    lv_screen_load_anim(panel->screen_, LV_SCR_LOAD_ANIM_FADE_IN, panel->GetAnimationTime(), _DELAY_TIME, false);

    // Schedule the fade-out animation after display duration
    auto *fadingInTimer = lv_timer_create(SplashPanel::fading_in_timer_callback, panel->GetFadeInDuration(), panel);

    // Remove the fade_in_timer after transition
    lv_timer_del(fadeInTimer);
}

/// @brief Callback function for the fade out animation_timer completion
/// @param fade_out_timer the animation_timer that has completed
void SplashPanel::fading_in_timer_callback(lv_timer_t *fadeOutTimer)
{
    // Animation sequence complete - executing callback

    // Get the splash panel instance
    auto *panel = static_cast<SplashPanel *>(lv_timer_get_user_data(fadeOutTimer));

    log_v("Fading out...");
    lv_scr_load_anim(panel->blankScreen_, LV_SCR_LOAD_ANIM_FADE_OUT, panel->GetAnimationTime(), 0, false);

    // Create a animation_timer for the completion callback
    auto *fadingOutTimer = lv_timer_create(SplashPanel::fading_out_timer_callback, panel->GetFadeOutDuration(), panel);

    // Remove the fade_out_timer after transition, this replaces having to set a repeat on the animation_timer
    lv_timer_del(fadeOutTimer);
}

// IInputService Interface Implementation

Action SplashPanel::GetShortPressAction()
{
    // Short press during splash: Skip animation (future enhancement)
    // For now, return NoAction since we don't interrupt animations
    log_d("SplashPanel: Short press action requested - returning NoAction (animation will complete)");
    return Action(nullptr);
}

Action SplashPanel::GetLongPressAction()
{
    // Long press during splash: Switch to CONFIG panel
    log_i("SplashPanel: Long press - switching to CONFIG panel");

    // Return an action that directly calls PanelService interface
    if (panelService_)
    {
        return Action(
            [this]()
            {
                panelService_->CreateAndLoadPanel(
                    PanelNames::CONFIG,
                    []()
                    {
                        // Panel switch callback handled by service
                    },
                    true); // Mark as trigger-driven to avoid overwriting restoration panel
            });
    }

    log_w("SplashPanel: PanelService not available, returning no action");
    return Action(nullptr);
}


// Manager injection method
void SplashPanel::SetManagers(IPanelService *panelService, IStyleService *styleService)
{
    panelService_ = panelService;
    // styleService_ is already set in constructor, but update if different instance provided
    if (styleService != styleService_)
    {
        styleService_ = styleService;
    }
    // Managers injected successfully
}

void SplashPanel::SetPreferenceService(IPreferenceService *preferenceService)
{
    preferenceService_ = preferenceService;
}

int SplashPanel::GetSplashDuration() const
{
    if (preferenceService_)
    {
        const Configs &config = preferenceService_->GetConfig();
        return config.splashDuration;
    }
    // Fallback to default timing if no preference service
    return 1500;
}

int SplashPanel::GetAnimationTime() const
{
    // Animation time is fixed at 1000ms regardless of splash duration
    return (SplashPanel::GetSplashDuration() - _DISPLAY_TIME) / 2;
}

int SplashPanel::GetFadeInDuration() const
{
    // Duration for fade-in timer (animation + display + delay)
    return GetAnimationTime() + _DISPLAY_TIME + _DELAY_TIME;
}

int SplashPanel::GetFadeOutDuration() const
{
    // Duration for fade-out completion timer (animation + delay)
    return GetAnimationTime() + _DELAY_TIME;
}