#pragma once

#include "components/oem_oil_component.h"
#include "icons/oil_can_regular.h"

class OemOilPressureComponent : public OemOilComponent
{
public:
    OemOilPressureComponent();
    ~OemOilPressureComponent() = default;

protected:
    // Override virtual methods from base class
    const lv_image_dsc_t* get_icon() const override;
    int32_t get_scale_min() const override;
    int32_t get_scale_max() const override;
    int32_t get_danger_zone() const override;
    lv_align_t get_alignment() const override;
    lv_scale_mode_t get_scale_mode() const override;
    int32_t get_rotation() const override;
    int32_t get_angle_range() const override;
    bool is_danger_condition(int32_t value) const override;
    void setup_danger_zone(lv_scale_section_t *section) const override;
    int32_t get_icon_y_offset() const override;
};