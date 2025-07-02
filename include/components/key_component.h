#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_component.h"
#include "utilities/types.h"
#include "managers/style_manager.h"

#include <lvgl.h>

class KeyComponent : public IComponent
{
public:
    KeyComponent();
    virtual ~KeyComponent();

    void render_load(lv_obj_t *screen) override;

protected:
    // LVGL objects
    lv_obj_t *_key_icon;

private:
    void create_icon();
};