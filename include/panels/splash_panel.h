#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_panel.h"
#include "components/clarity_component.h"
#include "utilities/lv_tools.h"

#define SPLASH_ANIMATION_TIME 2000
#define SPLASH_DELAY_TIME 0
#define SPLASH_DISPLAY_TIME 850

class SplashPanel : public IPanel
{
public:
    SplashPanel(PanelIteration panel_iteration);
    ~SplashPanel();

    std::string get_name() const {return "SplashPanel";};
    PanelType get_type() const {return PanelType::Splash;};

    void init(IDevice *device) override;
    void show(std::function<void()> callback_function) override;
    void update() override;

private:
    lv_obj_t *_blank_screen;

    static void fade_in_timer_callback(lv_timer_t *timer);
    static void fade_out_timer_callback(lv_timer_t *timer);
    static void animation_complete_timer_callback(lv_timer_t *timer);
};