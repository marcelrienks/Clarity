#include "panels/splash_panel.h"
#include "managers/error_manager.h"
#include "managers/interrupt_manager.h"
#include "managers/panel_manager.h"
#include "managers/style_manager.h"
#include "utilities/constants.h"
#include "utilities/logging.h"
#include <algorithm>
#include <esp32-hal-log.h>

// Constructors and Destructors

/**
 * @brief Constructs a splash panel with required dependencies
 * @param gpio GPIO provider interface for hardware interaction
 * @param display Display provider interface for screen creation
 * @param styleService Style service interface for theming
 * @param notificationService Notification service for panel lifecycle events
 *
 * Initializes the splash panel with dependency injection pattern. Creates the
 * splash component and sets up notification service with fallback to default.
 * The component is stack-allocated for better memory management.
 */
SplashPanel::SplashPanel(IGpioProvider *gpio, IDisplayProvider *display, IStyleService *styleService,
                         IPanelNotificationService* notificationService)
    : gpioProvider_(gpio), displayProvider_(display), styleService_(styleService), panelService_(nullptr),
      component_(styleService), componentInitialized_(false),
      notificationService_(notificationService ? notificationService : &PanelManager::NotificationService())
{
    log_v("SplashPanel constructor called");
}

/**
 * @brief Destructor that cleans up LVGL screen resources
 *
 * Safely deletes both the main screen and blank screen used for animation
 * transitions. The component is stack-allocated and automatically destroyed.
 * Ensures no LVGL memory leaks when panel is destroyed.
 */
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

    // Component is now stack-allocated and will be automatically destroyed
}

// Core Functionality Methods

/**
 * @brief Initializes the screen with component and creates blank screens for animation transitions
 *
 * Creates both the main screen for content display and a blank screen for
 * smooth fade-out animation transitions. Validates display provider availability
 * and reports critical errors if dependencies are missing. Part of the panel
 * lifecycle initialization phase.
 */
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

/**
 * @brief Shows the screen with fade-in animation
 *
 * Renders the splash component at the center of the screen and initiates
 * the fade-in animation sequence. Creates LVGL timers to manage the animation
 * lifecycle including fade-in, display, and fade-out phases. The animation
 * duration is configurable through preferences.
 */
void SplashPanel::Load()
{
    log_v("Load() called");
    // Component is now stack-allocated and initialized in constructor
    componentInitialized_ = true;

    // Create location parameters for the splash component
    ComponentLocation splashLocation(LV_ALIGN_CENTER, 0, 0);

    // Render the component
    component_.Render(screen_, splashLocation, displayProvider_);

    lv_screen_load_anim(screen_, LV_SCR_LOAD_ANIM_FADE_IN, GetAnimationTime(), 0, false);

    // Schedule the fade-in animation
    lv_timer_create(SplashPanel::fade_in_timer_callback, GetAnimationTime(), this);

    log_t("SplashPanel loaded successfully");
}

/**
 * @brief Updates the panel state (minimal for splash panel)
 *
 * Splash panel doesn't require regular content updates as it's animation-driven.
 * Simply notifies the notification service that the update cycle is complete.
 * The animation timers handle the panel's lifecycle independently.
 */
void SplashPanel::Update()
{
    log_v("Update() called");
    // Splash panel doesn't need regular updates - animation handles its own state
}

// Static Callback Methods

/**
 * @brief Callback function for fade-in animation completion
 * @param fadeInTimer The LVGL timer that has completed
 *
 * Called when the fade-in animation finishes. Sets UI state to IDLE during
 * the display period to allow button actions, then schedules the display
 * timer for the next phase. Cleans up the completed fade-in timer.
 */
void SplashPanel::fade_in_timer_callback(lv_timer_t *fadeInTimer)
{
    log_v("fade_in_timer_callback() called");
    // Get the screen pointer that was added to the user data
    auto *panel = static_cast<SplashPanel *>(lv_timer_get_user_data(fadeInTimer));

    // During display period, no animations are running - set IDLE to allow actions
    if (panel->panelService_)
    {
        panel->panelService_->SetUiState(UIState::IDLE);
    }

    // Create a timer for the callback
    lv_timer_create(SplashPanel::display_timer_callback, _DISPLAY_TIME, panel);

    // Remove the fade_in_timer after transition
    lv_timer_del(fadeInTimer);
}

/**
 * @brief Callback for display timer completion after splash screen display period
 * @param fadeOutTimer The LVGL timer that has completed
 *
 * Called when the display period expires. Sets UI state to BUSY for fade-out
 * animation, loads the blank screen with fade-out effect, and schedules the
 * final fade-out timer. Includes essential delay time for LVGL cleanup.
 */
void SplashPanel::display_timer_callback(lv_timer_t *fadeOutTimer)
{
    log_v("display_timer_callback() called");
    // Get the splash panel instance
    auto *panel = static_cast<SplashPanel *>(lv_timer_get_user_data(fadeOutTimer));

    // About to start fade-out animation - set BUSY
    if (panel->panelService_)
    {
        panel->panelService_->SetUiState(UIState::BUSY);
    }

    lv_screen_load_anim(panel->blankScreen_, LV_SCR_LOAD_ANIM_FADE_OUT, panel->GetAnimationTime(), 0, false);

    // Schedule the fade-out animation
    lv_timer_create(SplashPanel::fade_out_timer_callback, panel->GetAnimationTime() + _DELAY_TIME,
                    panel); // NOTE: the delay time is essential, to give LVGL and the code time to cleanup, else memory
                            // becomes corrupted

    // Delete the animation_timer
    lv_timer_del(fadeOutTimer);
}

/**
 * @brief Callback function for fade-out animation completion
 * @param fadeOutTimer The LVGL timer that has completed
 *
 * Called when the fade-out animation finishes. Notifies the notification
 * service that panel loading is complete, which triggers transition to the
 * target panel. Cleans up the fade-out timer to complete the animation cycle.
 */
void SplashPanel::fade_out_timer_callback(lv_timer_t *fadeOutTimer)
{
    log_v("fade_out_timer_callback() called");
    // Get the splash panel instance
    auto *panel = static_cast<SplashPanel *>(lv_timer_get_user_data(fadeOutTimer));

    panel->notificationService_->OnPanelLoadComplete(panel);

    // Remove the fade_out_timer after transition, this replaces having to set a repeat on the animation_timer
    lv_timer_del(fadeOutTimer);
}


/**
 * @brief Injects manager service dependencies
 * @param panelService Panel service for UI state management and panel operations
 * @param styleService Style service for theme management (updated if different)
 *
 * Updates the panel and style service references for runtime services.
 * Style service is already set in constructor but can be updated if a
 * different instance is provided during panel lifecycle.
 */
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

/**
 * @brief Injects preference service dependency for configurable splash duration
 * @param preferenceService Preference service for reading splash configuration
 *
 * Enables the splash panel to read configurable splash duration from
 * preferences. The splash duration determines the total time for fade-in,
 * display, and fade-out animations.
 */
void SplashPanel::SetPreferenceService(IPreferenceService *preferenceService)
{
    log_v("SetPreferenceService() called");
    preferenceService_ = preferenceService;
}

/**
 * @brief Calculates animation time based on configurable splash duration
 * @return Animation duration in milliseconds for fade-in and fade-out
 *
 * Reads the splash duration from preferences and calculates the time for
 * fade-in and fade-out animations. The display time is subtracted and the
 * remaining time is split equally between fade-in and fade-out phases.
 */
int SplashPanel::GetAnimationTime() const
{
    log_v("GetAnimationTime() called");

    // Get splash duration preference with default
    std::string splashDurationStr = preferenceService_ ? preferenceService_->GetPreference("system.splash_duration") : "";
    int splashDuration = splashDurationStr.empty() ? 1500 : std::stoi(splashDurationStr);

    int animTime = (splashDuration - _DISPLAY_TIME) / 2;

    return animTime;
}

// IActionService Interface Implementation

/**
 * @brief Static function for handling short button press during splash
 * @param panelContext Pointer to the splash panel instance (unused)
 *
 * Short press actions are disabled during splash screen animation to prevent
 * interference with the animation sequence. This maintains visual consistency
 * and prevents unintended actions during the splash display.
 */
static void SplashPanelShortPress(void* panelContext)
{
    log_v("SplashPanelShortPress() called");
    // No action during splash screen animation
}

/**
 * @brief Static function for handling long button press during splash
 * @param panelContext Pointer to the splash panel instance
 *
 * Long press during splash screen loads the configuration panel. This provides
 * a quick way to access settings during system startup without waiting for
 * the splash animation to complete.
 */
static void SplashPanelLongPress(void* panelContext)
{
    log_v("SplashPanelLongPress() called");

    auto* panel = static_cast<SplashPanel*>(panelContext);
    if (panel)
    {
        // Call public method to handle the long press
        panel->HandleLongPress();
    }
}

/**
 * @brief Gets the short press handler function pointer
 * @return Function pointer for short press handling
 *
 * Part of the IActionService interface. Returns the static function that
 * handles short press events for this panel. Short press is disabled during
 * splash screen to maintain animation integrity.
 */
void (*SplashPanel::GetShortPressFunction())(void* panelContext)
{
    return SplashPanelShortPress;
}

/**
 * @brief Gets the long press handler function pointer
 * @return Function pointer for long press handling
 *
 * Part of the IActionService interface. Returns the static function that
 * handles long press events for this panel. Long press loads the configuration
 * panel for quick access to settings during startup.
 */
void (*SplashPanel::GetLongPressFunction())(void* panelContext)
{
    return SplashPanelLongPress;
}

/**
 * @brief Gets the panel instance pointer for button handler context
 * @return Pointer to this panel instance
 *
 * Part of the IActionService interface. Provides the panel instance as
 * context for button handler functions. This allows static button handlers
 * to access the specific panel instance that should handle the action.
 */
void* SplashPanel::GetPanelContext()
{
    return this;
}

/**
 * @brief Handles long press action by loading configuration panel
 *
 * Provides immediate access to the configuration panel during splash screen
 * animation. Marks the panel load as trigger-driven to ensure proper restoration
 * behavior. Reports warning if panel service is not available.
 */
void SplashPanel::HandleLongPress()
{
    if (panelService_)
    {
        log_i("SplashPanel long press - loading config panel");
        panelService_->CreateAndLoadPanel(PanelNames::CONFIG, true);
    }
    else
    {
        log_w("SplashPanel: Cannot load config panel - panelService not available");
    }
}