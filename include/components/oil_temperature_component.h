#pragma once

#include "interfaces/i_component.h"
#include "utilities/types.h"
#include "managers/style_manager.h"
#include "icons/oil_temp_regular.h"

#include <lvgl.h>

class OilTemperatureComponent : public IComponent
{
public:
    OilTemperatureComponent();
    ~OilTemperatureComponent();

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
    static constexpr int32_t _animation_duration = 1000;
    static constexpr int32_t _needle_length = 90;
    
    
    // Use normal scale internally, but map values for display
    static constexpr int32_t _scale_min = 0;
    static constexpr int32_t _scale_max = 120;
    static constexpr int32_t _danger_zone = 100;

    int32_t map_reverse_value(int32_t value) const;
};