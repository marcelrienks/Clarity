#include "components/oem_oil_pressure_component.h"

OemOilPressureComponent::OemOilPressureComponent()
    : OemOilComponent()
{
    // Constructor delegates to base class
}

const lv_image_dsc_t* OemOilPressureComponent::get_icon() const
{
    return &oil_can_regular;
}

int32_t OemOilPressureComponent::get_scale_min() const
{
    return 0;
}

int32_t OemOilPressureComponent::get_scale_max() const
{
    return 60;
}

int32_t OemOilPressureComponent::get_danger_zone() const
{
    return 5;
}

lv_align_t OemOilPressureComponent::get_alignment() const
{
    return LV_ALIGN_TOP_MID;
}

lv_scale_mode_t OemOilPressureComponent::get_scale_mode() const
{
    return LV_SCALE_MODE_ROUND_INNER;
}

int32_t OemOilPressureComponent::get_rotation() const
{
    return 210;
}

int32_t OemOilPressureComponent::get_angle_range() const
{
    return 120;
}

bool OemOilPressureComponent::is_danger_condition(int32_t value) const
{
    return value <= OemOilPressureComponent::get_danger_zone();
}

void OemOilPressureComponent::setup_danger_zone(lv_scale_section_t *section) const
{
    lv_scale_section_set_range(section, OemOilPressureComponent::get_scale_min(), OemOilPressureComponent::get_danger_zone());
}

int32_t OemOilPressureComponent::get_icon_y_offset() const
{
    return -50;
}