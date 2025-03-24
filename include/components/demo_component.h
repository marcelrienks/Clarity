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
    std::function<void()> render_completion_callback;
};

class DemoComponent : public IComponent
{
public:
    DemoComponent();
    ~DemoComponent();

    void init(lv_obj_t *screen) override;
    void render_reading(Reading reading, std::function<void()> render_completion_callback = nullptr) override;

private:
    lv_obj_t *_scale;
    lv_obj_t *_needle_line;
    int32_t _current_value;

    // Component specific constants
    static constexpr const int32_t _animation_duration = 1000;
    static constexpr const int32_t _playback_duration = 0;
};