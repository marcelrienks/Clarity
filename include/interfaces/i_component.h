#pragma once // preventing duplicate definitions, alternative to the traditional include guards

// System/Library Includes
#include <esp32-hal-log.h>
#include <functional>
#include <lvgl.h>

// Project Includes
#include "utilities/types.h"

/// @brief UI elements
class IComponent
{
public:
    // Core Interface Methods
    virtual void render(lv_obj_t *screen, const ComponentLocation& location) = 0;
    virtual void refresh(const Reading& reading) {};
    virtual void set_value(int32_t value) {};
};