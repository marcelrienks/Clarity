#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_component.h"


class ClarityComponent : public IComponent
{
public:
    void render_show(lv_obj_t *screen) override;
};