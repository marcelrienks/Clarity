#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_component.h"

#include <lvgl.h>
#include <LovyanGFX.hpp>
#include "utilities/common_types.h"

class DemoComponent : public IComponent
{
private:
    lv_obj_t *_scale;
    lv_obj_t *_needle_line;
    uint32_t _start_time;
    Reading _current_reading;

    static void set_needle_line_value_callback_wrapper(void *object, int32_t value);

    void animate_needle(int32_t animation_duration, int32_t playback_duration, int32_t start, int32_t end);
    void set_needle_line_value_callback(void *object, int32_t value);

public:
    DemoComponent();
    ~DemoComponent();

    void init(lv_obj_t *virtual_screen);
    void update(Reading value);
};

extern DemoComponent *g_demo_component_instance;