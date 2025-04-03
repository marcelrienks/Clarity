#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "utilities/types.h"

#include <esp32-hal-log.h>
#include <lvgl.h>
#include <functional>

/// @brief UI elements
class IComponent
{
public:
    // Overridable methods
    virtual void render_show(lv_obj_t *screen) {};
    virtual void render_update(lv_anim_t *animation, int32_t start, int32_t end) {};
    virtual void set_value(int32_t value) {};
};