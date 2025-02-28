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
    PanelCompletionCallback _completion_callback;

    static void fade_in_timer_callback(lv_timer_t *timer);
    static void fade_out_timer_callback(lv_timer_t *timer);
    static void animation_completion_callback(lv_timer_t *timer);

public:
    SplashPanel();
    ~SplashPanel();

    void init(IDevice *device);
    void show();
    void update();

    // Implementation of the callback setter
    void set_completion_callback(PanelCompletionCallback callback) override;
};

#endif // SPLASH_PANEL_H