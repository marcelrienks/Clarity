#ifndef CLARITY_COMPONENT_H
#define CLARITY_COMPONENT_H

#include "interfaces/i_component.h"

class ClarityComponent : public IComponent
{
public:
    void init(lv_obj_t *virtual_screen);
    void update(std::shared_ptr<void> ptr);
};

#endif // CLARITY_COMPONENT_H