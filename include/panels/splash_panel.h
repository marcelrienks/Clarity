#ifndef SPLASH_PANEL_H
#define SPLASH_PANEL_H

#include "interfaces/i_panel.h"
#include "components/clarity_component.h"
#include "utilities/serial_logger.h"
#include "utilities/lv_tools.h"

#include <lvgl.h>

#define ANIMATION_TIME 1000
#define DELAY_TIME 0
#define DISPLAY_TIME 500

class SplashPanel : public IPanel
{
private:
    IDevice *_device;
    lv_obj_t *_screen;
    lv_obj_t *_blank_screen;
    ClarityComponent *_component;
    std::function<void()> _callback_function;

    static void animation_complete_callback(lv_anim_t * animation);

    void register_animation(lv_obj_t *screen, lv_screen_load_anim_t screen_load, uint32_t animation_time, uint32_t delay_time, lv_anim_completed_cb_t _callback_function);

public:
    SplashPanel();
    ~SplashPanel();

    void init(IDevice *device) override;
    void show() override;
    void update() override;
    void set_callback(std::function<void()> callback_function) override;
};

#endif // SPLASH_PANEL_H