#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include <lvgl.h>
#include <utilities/common_types.h>

/// @brief UI elements
class IComponent
{
public:    
    virtual void init(lv_obj_t *virtual_screen) = 0;
    virtual void update(Reading value) = 0;
};