#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_component.h"
#include "utilities/serial_logger.h"

#include <lvgl.h>
#include "utilities/types.h"

struct NeedleAnimationContext
{
    IComponent *component;
    lv_obj_t *needle_line;
    lv_obj_t *scale;
    std::function<void()> component_animation_completion_callback;
};

class DemoComponent : public IComponent
{
public:
    DemoComponent();
    ~DemoComponent();

    void init(lv_obj_t *screen) override;
    void update(Reading reading, std::function<void()> update_component_completion_callback = nullptr) override;

private:
    lv_obj_t *_scale;
    lv_obj_t *_needle_line;
    int32_t _start_time;
    int32_t _current_reading;

    void animate_needle(int32_t animation_duration, int32_t playback_duration, int32_t start, int32_t end, std::function<void()> component_animation_completion_callback);
};