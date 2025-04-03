#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_component.h"
#include "utilities/types.h"

#include <lvgl.h>

class DemoComponent : public IComponent
{
public:
    ~DemoComponent();

    void render_show(lv_obj_t *screen) override;
    void render_update(lv_anim_t *animation, int32_t start, int32_t end) override;
    void set_value(int32_t value) override;

private:
    lv_obj_t *_scale;
    lv_obj_t *_needle_line;

    // Style variables as member variables
    lv_style_t _indicator_style;
    lv_style_t _minor_ticks_style;
    lv_style_t _main_line_style;
    lv_style_t _section_label_style;
    lv_style_t _section_minor_tick_style;
    lv_style_t _section_main_line_style;

    // Component specific constants
    static constexpr int32_t _animation_duration = 1000;
    static constexpr int32_t _playback_duration = 0;
};