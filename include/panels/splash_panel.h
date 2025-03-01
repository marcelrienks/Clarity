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
    int _animation_sequence;

    static void animation_callback(lv_anim_t * animation);
    void run_animation_workflow_handler();

public:
    SplashPanel();
    ~SplashPanel();

    void init(IDevice *device) override;
    void show() override;
    void update() override;
    void set_callback(std::function<void()> callback_function) override;
    
    // New method to advance animation sequence
    void advance_animation();
};

#endif // SPLASH_PANEL_H