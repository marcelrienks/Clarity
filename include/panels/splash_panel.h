#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_panel.h"
#include "components/clarity_component.h"
#include "utilities/lv_tools.h"

class SplashPanel : public IPanel
{
public:
    SplashPanel();
    ~SplashPanel();

    std::string get_name() const { return PanelNames::Splash; };

    void init() override;
    void show(std::function<void()> show_panel_completion_callback) override;
    void update(std::function<void()> update_panel_completion_callback = nullptr) override;

private:
    // Panel specific constants
    static constexpr const int _animation_time = 2000;
    static constexpr const int _delay_time = 0;
    static constexpr const int _display_time = 850;

    // Components
    lv_obj_t *_screen;
    std::shared_ptr<IComponent> _component;
    lv_obj_t *_blank_screen;

    static void fade_in_timer_callback(lv_timer_t *timer);
    static void fade_out_timer_callback(lv_timer_t *timer);
    static void animation_complete_timer_callback(lv_timer_t *timer);
};