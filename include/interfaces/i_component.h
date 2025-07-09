#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "utilities/types.h"

#include <esp32-hal-log.h>
#include <lvgl.h>
#include <functional>

/// @brief UI elements
class IComponent //TODO: rename all components to widgets
{
public:
    // Overridable methods
    virtual void render_load(lv_obj_t *screen, const ComponentLocation& location) = 0;
    virtual void render_update(bool is_key_present) {};
    virtual void render_update(int32_t value) {};
    virtual void set_value(int32_t value) {};
};