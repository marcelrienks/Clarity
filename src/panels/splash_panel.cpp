#include "panels/splash_panel.h"
#include "factories/component_factory.h"
#include "interfaces/i_component_factory.h"
#include "managers/error_manager.h"
#include "managers/interrupt_manager.h"
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
    log_v("SplashPanel constructor called");
}

SplashPanel::~SplashPanel()
{
    log_v("~SplashPanel() destructor called");

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
    log_v("Init() called");

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
    log_v("Load() called");

    callbackFunction_ = callbackFunction;
    
    // Set BUSY at start of load - animations will be running
    if (panelService_)
    {
        panelService_->SetUiState(UIState::BUSY);
    }

    if (!componentFactory_)
    {
        log_e("SplashPanel requires component factory");
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "SplashPanel", "ComponentFactory is null");
        return;
    }

    // Create component using injected factory
    component_ = componentFactory_->CreateClarityComponent(styleService_);
    if (!component_)
    {
        log_e("Failed to create clarity component for splash panel");
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "SplashPanel", "Component creation failed");
        return;
    }

    // Create location parameters for the splash component
    ComponentLocation splashLocation(LV_ALIGN_CENTER, 0, 0);

    // Render the component
    component_->Render(screen_, splashLocation, displayProvider_);

    lv_screen_load_anim(screen_, LV_SCR_LOAD_ANIM_FADE_IN, GetAnimationTime(), 0, false);

    // Schedule the fade-in animation
    lv_timer_create(SplashPanel::fade_in_timer_callback, GetAnimationTime(), this);
}

/// @brief Update the reading on the screen
void SplashPanel::Update(std::function<void()> callbackFunction)
{
    log_v("Update() called");
    // Splash panel doesn't need regular updates - animation handles its own state
    callbackFunction();
}

// Static Callback Methods

/// @brief Callback function for the fade in fade_in_timer completion
/// @param fade_in_timer the fade_in_timer that has completed
void SplashPanel::fade_in_timer_callback(lv_timer_t *fadeInTimer)
{
    log_v("fade_in_timer_callback() called");
    // Get the screen pointer that was added to the user data
    auto *panel = static_cast<SplashPanel *>(lv_timer_get_user_data(fadeInTimer));

    // Create a timer for the callback
    lv_timer_create(SplashPanel::display_timer_callback, _DISPLAY_TIME, panel);

    // Remove the fade_in_timer after transition
    lv_timer_del(fadeInTimer);
}

/// @brief Callback for displaying the splash screen for a period of time
/// @param fadeOutTimer the animation_timer that has completed
void SplashPanel::display_timer_callback(lv_timer_t *fadeOutTimer)
{
    log_v("display_timer_callback() called");
    // Get the splash panel instance
    auto *panel = static_cast<SplashPanel *>(lv_timer_get_user_data(fadeOutTimer));

    // During display period, no animations are running - set IDLE to allow actions
    if (panel->panelService_)
    {
        panel->panelService_->SetUiState(UIState::IDLE);
        log_d("SplashPanel: Display timer - setting IDLE (no animations active)");
    }

    
    // About to start fade-out animation - set BUSY
    if (panel->panelService_)
    {
        panel->panelService_->SetUiState(UIState::BUSY);
        log_d("SplashPanel: Starting fade-out animation - setting BUSY");
    }
    
    lv_screen_load_anim(panel->blankScreen_, LV_SCR_LOAD_ANIM_FADE_OUT, panel->GetAnimationTime(), 0, false);

    // Schedule the fade-out animation
    lv_timer_create(SplashPanel::fade_out_timer_callback, panel->GetAnimationTime() + _DELAY_TIME, panel); //NOTE: the delay time is essential, to give LVGL and the code time to cleanup, else memory becomes corrupted
    
    // Delete the animation_timer
    lv_timer_del(fadeOutTimer);
}

/// @brief Callback function for the fade out animation_timer completion
/// @param fade_out_timer the animation_timer that has completed
void SplashPanel::fade_out_timer_callback(lv_timer_t *fadeOutTimer)
{
    log_v("fade_out_timer_callback() called");
    // Get the splash panel instance
    auto *panel = static_cast<SplashPanel *>(lv_timer_get_user_data(fadeOutTimer));

    // PanelManager will set IDLE in its completion callback, no need to set it here
    
    panel->callbackFunction_();

    // Remove the fade_out_timer after transition, this replaces having to set a repeat on the animation_timer
    lv_timer_del(fadeOutTimer);
}

// IInputService Interface Implementation

Action SplashPanel::GetShortPressAction()
{
    log_v("GetShortPressAction() called");

    // Short press during splash: Skip animation (future enhancement)
    return Action(nullptr);
}

Action SplashPanel::GetLongPressAction()
{
    log_v("GetLongPressAction() called");

    // Long press during splash: Switch to CONFIG panel

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
    log_v("SetManagers() called");
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
    log_v("SetPreferenceService() called");
    preferenceService_ = preferenceService;
}

int SplashPanel::GetAnimationTime() const
{
    log_v("GetAnimationTime() called");
    int animTime = (preferenceService_->GetConfig().splashDuration - _DISPLAY_TIME) / 2;

    return animTime;
}