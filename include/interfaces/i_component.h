#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "utilities/types.h"

#include <lvgl.h>
#include <functional>

/// @brief UI elements
class IComponent
{
public:
    // Pure Virtual
    virtual void render_show(lv_obj_t *screen) = 0;

    // Overridable methods
    virtual void render_update(lv_anim_t *animation, int32_t start, int32_t end) {};//TODO: feel like this methods name is not accurate?
    virtual void set_value(int32_t value) {};
};