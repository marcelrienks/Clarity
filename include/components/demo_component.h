#ifndef DEMO_COMPONENT_H
#define DEMO_COMPONENT_H

#include "interfaces/i_component.h"

#include <lvgl.h>
#include <LovyanGFX.hpp>

class DemoComponent : public IComponent
{
private:
    lv_obj_t *_scale;
    lv_obj_t *_needle_line;
    uint32_t _start_time;
    uint32_t _current_reading;

    static void set_needle_line_value_callback_wrapper(void *object, int32_t value);

    void animate_needle(int16_t animation_duration, int16_t playback_duration, int32_t start, int32_t end);
    void set_needle_line_value_callback(void *object, int32_t value);

public:
    DemoComponent();
    ~DemoComponent();

    void init(lv_obj_t *virtual_screen);
    void update(uint32_t value);
};

extern DemoComponent *g_demo_component_instance;

#endif // DEMO_COMPONENT_H