#include "components/oem_oil_temperature_component.h"

OemOilTemperatureComponent::OemOilTemperatureComponent()
    : OemOilComponent()
{
    // Constructor delegates to base class
}

const lv_image_dsc_t* OemOilTemperatureComponent::get_icon() const
{
    return &oil_temp_regular;
}

int32_t OemOilTemperatureComponent::get_scale_min() const
{
    return _scale_min;
}

int32_t OemOilTemperatureComponent::get_scale_max() const
{
    return _scale_max;
}

int32_t OemOilTemperatureComponent::get_danger_zone() const
{
    return _danger_zone;
}

lv_align_t OemOilTemperatureComponent::get_alignment() const
{
    return LV_ALIGN_BOTTOM_MID;
}

lv_scale_mode_t OemOilTemperatureComponent::get_scale_mode() const
{
    return LV_SCALE_MODE_ROUND_INNER;
}

int32_t OemOilTemperatureComponent::get_rotation() const
{
    return 30; // starting angle (0 = 3 o'clock)
}

int32_t OemOilTemperatureComponent::get_angle_range() const
{
    return 120; // range in degrees for the span of the scale
}

bool OemOilTemperatureComponent::is_danger_condition(int32_t value) const
{
    return value >= _danger_zone; // High temperature is dangerous
}

int32_t OemOilTemperatureComponent::map_value_for_display(int32_t value) const
{
    // Maps the value from the original scale (0-120) to the normal scale (120-0).
    // LVGL 9.3 has a bug that does allow setting a reverse scale 120-0, but it cannot animate the needle using that.
    // This method is used to map the value from the original scale (0-120) to the normal scale (120-0).
    
    log_d("original value is %i", value);
    
    // Map from [0,120] to [120,0] reverse the scale
    int32_t mapped_value = _scale_max - value;
    
    log_d("mapped value is %i", mapped_value);
    
    return mapped_value;
}

void OemOilTemperatureComponent::setup_danger_zone(lv_scale_section_t *section) const
{
    // Danger zone: map correct danger zone to reversed danger zone (hack to solve reversed scale in LVGL 9.3)
    lv_scale_section_set_range(section, map_value_for_display(_scale_max), map_value_for_display(_danger_zone));
}

int32_t OemOilTemperatureComponent::get_icon_y_offset() const
{
    return 50; // Below center
}