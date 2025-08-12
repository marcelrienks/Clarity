#pragma once // preventing duplicate definitions, alternative to the traditional include guards

// Project Includes
#include "interfaces/i_panel.h"
#include "interfaces/i_action_service.h"
#include "interfaces/i_panel_service.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_display_provider.h"
#include "interfaces/i_style_service.h"
#include "interfaces/i_preference_service.h"
#include "components/clarity_component.h"
#include "utilities/lv_tools.h"
#include <memory>

/**
 * @class SplashPanel
 * @brief Startup splash screen with animated Clarity branding
 * 
 * @details This panel displays the Clarity logo with fade-in/fade-out animations
 * during system startup. It provides a professional bootup experience while
 * the system initializes in the background.
 * 
 * @presenter_role Coordinates the ClarityComponent for branding display
 * @animation_sequence Fade-in (2000ms) → Display (850ms) → Fade-out → Next panel
 * @timing_total ~3050ms total display time
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
class SplashPanel : public IPanel, public IActionService
{
public:
    // Constructors and Destructors
    SplashPanel(IGpioProvider *gpio, IDisplayProvider *display, IStyleService *styleService);
    ~SplashPanel();

    // Core Functionality Methods
    static constexpr const char* NAME = PanelNames::SPLASH;
    void Init() override;
    void Load(std::function<void()> callbackFunction) override;
    void Update(std::function<void()> callbackFunction) override;
    
    // Manager injection method
    void SetManagers(IPanelService* panelService, IStyleService* styleService) override;
    
    // Preference service injection
    void SetPreferenceService(IPreferenceService* preferenceService);
    
    // IActionService Interface Implementation - Action-based (no animation interruption)
    Action GetShortPressAction() override;
    Action GetLongPressAction() override;
    bool CanProcessInput() const override;
    
    // IPanel override to provide input service
    IActionService* GetInputService() override { return this; }

private:
    // Private Data Members
    // Panel specific constants
    static constexpr const int _ANIMATION_TIME = 1000;
    static constexpr const int _DELAY_TIME = 100;
    static constexpr const int _DISPLAY_TIME = 600;

    // Dependencies
    IGpioProvider *gpioProvider_;
    IDisplayProvider *displayProvider_;
    IStyleService *styleService_;
    IPanelService *panelService_;
    IPreferenceService *preferenceService_ = nullptr;

    // Components
    // screen_ is inherited from IPanel base class
    std::shared_ptr<IComponent> component_;
    lv_obj_t *blankScreen_;

    // Static Callback Methods
    static void animation_complete_timer_callback(lv_timer_t *timer);
    static void fade_in_timer_callback(lv_timer_t *timer);
    static void fade_out_timer_callback(lv_timer_t *timer);
    
    // Helper methods
    int GetSplashDuration() const;
};