#pragma once // preventing duplicate definitions, alternative to the traditional include guards

// System/Library Includes
#include <lvgl.h>

// Project Includes
#include "utilities/types.h"
#include "interfaces/i_display_provider.h"

/// @brief UI elements
class IComponent
{
public:
    // Core Interface Methods
    virtual void render(lv_obj_t *screen, const ComponentLocation& location, IDisplayProvider* display) = 0;
    virtual void refresh(const Reading& reading) {};
    virtual void setValue(int32_t value) {};
};