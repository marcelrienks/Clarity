#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_component.h"
#include "utilities/serial_logger.h"
#include "utilities/types.h"

#include <lvgl.h>

struct NeedleAnimationContext
{
    IComponent *component;
    lv_obj_t *needle_line;
    lv_obj_t *scale;
};

class DemoComponent : public IComponent
{
public:
    DemoComponent();
    ~DemoComponent();

    void render_show(lv_obj_t *screen) override;
    void render_update(lv_anim_t *animation, int32_t start, int32_t end) override;
    void set_value(int32_t value) override;

private:
    lv_obj_t *_scale;
    lv_obj_t *_needle_line;

    // Component specific constants
    static constexpr const int32_t _animation_duration = 1000;
    static constexpr const int32_t _playback_duration = 0;
};