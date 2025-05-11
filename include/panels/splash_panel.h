#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_panel.h"
#include "components/clarity_component.h"
#include "utilities/lv_tools.h"

class SplashPanel : public IPanel
{
public:
    SplashPanel(IDevice *device);
    ~SplashPanel();

    std::string get_name() const { return _name; };
    PanelType get_type() const { return _type; };

    void init() override;
    void show(std::function<void()> show_panel_completion_callback) override;
    void update(std::function<void()> update_panel_completion_callback = nullptr) override;

private:
    // Panel specific constants
    static constexpr const char *_name = "Splash";
    static constexpr const PanelType _type = PanelType::Splash;
    static constexpr const int _animation_time = 2000;
    static constexpr const int _delay_time = 0;
    static constexpr const int _display_time = 850;

    // Components
    IDevice *_device;
    lv_obj_t *_screen;
    std::shared_ptr<IComponent> _component;
    lv_obj_t *_blank_screen;

    static void fade_in_timer_callback(lv_timer_t *timer);
    static void fade_out_timer_callback(lv_timer_t *timer);
    static void animation_complete_timer_callback(lv_timer_t *timer);
};