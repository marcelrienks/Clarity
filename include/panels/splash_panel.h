#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_panel.h"
#include "components/clarity_component.h"
#include "utilities/serial_logger.h"
#include "utilities/lv_tools.h"

#include <lvgl.h>
#include <memory>

#define ANIMATION_TIME 1000
#define DELAY_TIME 0
#define DISPLAY_TIME 500

class SplashPanel : public IPanel
{
private:
    lv_obj_t *_blank_screen;
    std::shared_ptr<ClarityComponent> _clarity_component;

    static void fade_in_timer_callback(lv_timer_t *timer);
    static void fade_out_timer_callback(lv_timer_t *timer);
    static void animation_complete_timer_callback(lv_timer_t *timer);

public:
    SplashPanel();
    ~SplashPanel() override;

    void init(IDevice *device) override;
    void show() override;
    void update() override;
    void set_completion_callback(std::function<void()> callback_function) override;
};