#pragma once // preventing duplicate definitions, alternative to the traditional include guards

// Project Includes
#include "interfaces/i_panel.h"
#include "interfaces/i_component_factory.h"
#include "components/clarity_component.h"
#include "utilities/lv_tools.h"

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
class SplashPanel : public IPanel
{
public:
    // Constructors and Destructors
    SplashPanel(IComponentFactory* componentFactory);
    ~SplashPanel();

    // Core Functionality Methods
    static constexpr const char* NAME = PanelNames::SPLASH;
    void init(IGpioProvider* gpio, IDisplayProvider* display) override;
    void load(std::function<void()> callbackFunction, IGpioProvider* gpio, IDisplayProvider* display) override;
    void update(std::function<void()> callbackFunction, IGpioProvider* gpio, IDisplayProvider* display) override;

private:
    // Private Data Members
    // Panel specific constants
    static constexpr const int _ANIMATION_TIME = 2000;
    static constexpr const int _DELAY_TIME = 200;
    static constexpr const int _DISPLAY_TIME = 850;

    // Dependencies
    IComponentFactory* componentFactory_;

    // Components
    lv_obj_t *screen_; // All panels should always have their own screens
    std::shared_ptr<IComponent> component_;
    lv_obj_t *blankScreen_;

    // Static Callback Methods
    static void animation_complete_timer_callback(lv_timer_t *timer);
    static void fade_in_timer_callback(lv_timer_t *timer);
    static void fade_out_timer_callback(lv_timer_t *timer);
};