#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "utilities/types.h"

#include <lvgl.h>
#include <functional>

/// @brief UI elements
class IComponent
{
public:    
    virtual void init(lv_obj_t *screen) = 0;
    virtual void render_reading(Reading reading, std::function<void()> render_completion_callback = nullptr) = 0;
};