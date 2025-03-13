#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_panel.h"
#include "components/clarity_component.h"
#include "utilities/lv_tools.h"

#define SPLASH_ANIMATION_TIME 1000
#define SPLASH_DELAY_TIME 0
#define SPLASH_DISPLAY_TIME 500

class SplashPanel : public IPanel
{
public:
    SplashPanel();
    ~SplashPanel();

    void init(IDevice *device) override;
    void show(std::function<void()> callback_function) override;
    void update() override;

    PanelType get_type() const {return PanelType::Splash;};
    std::string get_name() const {return "SplashPanel";};

private:
    lv_obj_t *_blank_screen;
    std::shared_ptr<ClarityComponent> _clarity_component;

    static void fade_in_timer_callback(lv_timer_t *timer);
    static void fade_out_timer_callback(lv_timer_t *timer);
    static void animation_complete_timer_callback(lv_timer_t *timer);
};