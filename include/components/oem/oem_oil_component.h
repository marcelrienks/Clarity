#pragma once

#include "interfaces/i_component.h"
#include "utilities/types.h"
#include "managers/style_manager.h"

#include <lvgl.h>

class OemOilComponent : public IComponent
{
public:
    OemOilComponent();
    virtual ~OemOilComponent();

    void render_load(lv_obj_t *screen, const ComponentLocation& location) override;
    void render_update(lv_anim_t *animation, int32_t start, int32_t end) override;
    void set_value(int32_t value) override;

protected:
    // LVGL objects
    lv_obj_t *_scale;
    lv_obj_t *_needle_line;
    lv_obj_t *_oil_icon;
    
    // Style variables as member variables
    lv_style_t _indicator_part_style;
    lv_style_t _items_part_style;
    lv_style_t _main_part_style;
    lv_style_t _danger_section_items_part_style;

    // Common constants
    static constexpr int32_t _animation_duration = 1000;
    static constexpr int32_t _needle_length = 90;

    // Virtual methods for derived classes to override
    virtual const lv_image_dsc_t* get_icon() const = 0;
    virtual int32_t get_scale_min() const = 0;
    virtual int32_t get_scale_max() const = 0;
    virtual int32_t get_danger_zone() const = 0;
    virtual lv_align_t get_alignment() const = 0;
    virtual lv_scale_mode_t get_scale_mode() const = 0;
    virtual int32_t get_rotation() const = 0;
    virtual int32_t get_angle_range() const = 0;
    virtual bool is_danger_condition(int32_t value) const = 0;
    virtual int32_t map_value_for_display(int32_t value) const;
    virtual void setup_danger_zone(lv_scale_section_t *section) const = 0;
    virtual int32_t get_icon_y_offset() const = 0;

private:
    void initialize_styles();
    void cleanup_styles();
    void create_scale();
    void create_needle();
    void create_icon();
};