#pragma once // preventing duplicate definitions, alternative to the traditional include guards

// Project Includes
#include "interfaces/i_panel.h"
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
    SplashPanel();
    ~SplashPanel();

    // Core Functionality Methods
    const char *get_name() const { return PanelNames::Splash; };
    void init() override;
    void load(std::function<void()> show_panel_completion_callback) override;
    void update(std::function<void()> update_panel_completion_callback = nullptr) override;

private:
    // Private Data Members
    // Panel specific constants
    static constexpr const int _animation_time = 2000;
    static constexpr const int _delay_time = 200;
    static constexpr const int _display_time = 850;

    // Components
    lv_obj_t *_screen; // All panels should always have their own screens
    std::shared_ptr<IComponent> _component;
    lv_obj_t *_blank_screen;

    // Static Callback Methods
    static void animation_complete_timer_callback(lv_timer_t *timer);
    static void fade_in_timer_callback(lv_timer_t *timer);
    static void fade_out_timer_callback(lv_timer_t *timer);
};