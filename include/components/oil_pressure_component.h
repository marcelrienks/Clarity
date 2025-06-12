#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_component.h"
#include "utilities/types.h"
#include "managers/style_manager.h"
#include "icons/oil_can_icon_data.h"

#include <lvgl.h>

class OilPressureComponent : public IComponent
{
    public:
    OilPressureComponent();
    ~OilPressureComponent();

    void render_load(lv_obj_t *screen) override;
    void render_update(lv_anim_t *animation, int32_t start, int32_t end) override;
    void set_value(int32_t value) override;

private:
    lv_obj_t *_scale;
    lv_obj_t *_needle_line;
    lv_obj_t *_oil_can_icon;
    
    // Style variables as member variables
    lv_style_t _indicator_part_style;
    lv_style_t _items_part_style;
    lv_style_t _main_part_style;
    lv_style_t _danger_section_items_part_style;

    // Component specific constants
    static constexpr int32_t _animation_duration = 1000U;
    static constexpr int32_t _needle_length = 100U;
    static constexpr int32_t _danger_zone = 5U;
};