#ifndef I_COMPONENT_H
#define I_COMPONENT_H

#include <lvgl.h>
#include <memory>

class IComponent
{
public:    
    virtual void init(lv_obj_t *virtual_screen) = 0;
    virtual void update(uint32_t value) = 0;
};

#endif // I_COMPONENT_H