#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_component.h"

class ClarityComponent : public IComponent
{
public:
    void init(lv_obj_t *virtual_screen) override;
    void update(Reading value) override;
};