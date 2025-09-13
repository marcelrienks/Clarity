#pragma once

// Project Includes
#include "components/clarity_component.h"
#include "interfaces/i_action_service.h"
#include "interfaces/i_display_provider.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_panel.h"
#include "interfaces/i_panel_service.h"
#include "interfaces/i_panel_notification_service.h"
#include "interfaces/i_preference_service.h"
#include "interfaces/i_style_service.h"
#include <memory>

// Forward declarations
class IComponentFactory;

/**
 * @class SplashPanel
 * @brief Startup splash screen with animated Clarity branding
 *
 * @details This panel displays the Clarity logo with fade-in/fade-out animations
 * during system startup. It provides a professional bootup experience while
 * the system initializes in the background.
 *
 * @presenter_role Coordinates the ClarityComponent for branding display
 * @animation_sequence Initial delay (100ms) → Fade-in (1000ms) → Display (configurable) → Fade-out (1000ms) → Complete
 * (100ms delay) → Next panel
 * @timing_total ~2200ms + configurable display time
 *
 * @transitions_to OemOilPanel (or user-configured default panel)
 * @memory_usage Minimal - single component with timer-based animations
 *
 * @ui_layout:
 * - ClarityComponent centered on screen
 * - Black background with animated opacity
 * - Smooth fade transitions using LVGL timers
 *
 * @context This is the first panel shown on startup. It uses timer-based
 * animations for smooth transitions and automatically proceeds to the main
 * application panel when animation completes.
 */
class SplashPanel : public IPanel
{
  public:
    // Constructors and Destructors
    SplashPanel(IGpioProvider *gpio, IDisplayProvider *display, IStyleService *styleService,
                IComponentFactory* componentFactory = nullptr, 
                IPanelNotificationService* notificationService = nullptr);
    ~SplashPanel();

    // Core Functionality Methods
    static constexpr const char *NAME = PanelNames::SPLASH;
    void Init() override;
    void Load() override;
    void Update() override;

    // Manager injection method
    void SetManagers(IPanelService *panelService, IStyleService *styleService) override;

    // Preference service injection
    void SetPreferenceService(IPreferenceService *preferenceService);

    // IActionService Interface Implementation (inherited through IPanel)
    void (*GetShortPressFunction())(void* panelContext) override;
    void (*GetLongPressFunction())(void* panelContext) override;
    void* GetPanelContext() override;
    
    // Public action handler
    void HandleLongPress();

  private:
    // Private Data Members
    // Panel specific constants
    static constexpr const int _DISPLAY_TIME = 500;
    static constexpr const int _DELAY_TIME = 200; //NOTE: the delay time is essential, to give LVGL and the code time to cleanup, else memory becomes corrupted

    // Calculate timing based on splash duration from config
    int GetAnimationTime() const;

    // Dependencies
    IGpioProvider *gpioProvider_;
    IDisplayProvider *displayProvider_;
    IStyleService *styleService_;
    IPanelService *panelService_;
    IPreferenceService *preferenceService_ = nullptr;
    IComponentFactory *componentFactory_;
    IPanelNotificationService *notificationService_;

    // Components
    // screen_ is inherited from IPanel base class
    std::shared_ptr<IComponent> component_;
    lv_obj_t *blankScreen_;

    // Static Callback Methods
    static void fade_in_timer_callback(lv_timer_t *timer);
    static void display_timer_callback(lv_timer_t *timer);
    static void fade_out_timer_callback(lv_timer_t *timer);
};