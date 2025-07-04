#include "components/oem/oem_oil_pressure_component.h"

OemOilPressureComponent::OemOilPressureComponent()
    : OemOilComponent()
{
    // Constructor delegates to base class
}

/// @brief Gets the icon for the oil pressure component.
/// @return Pointer to the icon image descriptor.
const lv_image_dsc_t* OemOilPressureComponent::get_icon() const
{
    return &oil_can_regular;
}

/// @brief Gets the minimum scale value for the oil pressure component.
/// @return The minimum scale value.
int32_t OemOilPressureComponent::get_scale_min() const
{
    return 0;
}

/// @brief Gets the maximum scale value for the oil pressure component.
/// @return The maximum scale value.
int32_t OemOilPressureComponent::get_scale_max() const
{
    return 60;
}

/// @brief Gets the danger zone value for the oil pressure component.
/// @return The danger zone value.
int32_t OemOilPressureComponent::get_danger_zone() const
{
    return 5;
}

/// @brief Gets the alignment for the oil pressure component.
/// @return The alignment value.
lv_align_t OemOilPressureComponent::get_alignment() const
{
    return LV_ALIGN_TOP_MID;
}

/// @brief Gets the scale mode for the oil pressure component.
/// @return The scale mode.
lv_scale_mode_t OemOilPressureComponent::get_scale_mode() const
{
    return LV_SCALE_MODE_ROUND_INNER;
}

/// @brief Gets the angle range for the oil pressure component.
/// @return The angle range.
int32_t OemOilPressureComponent::get_angle_range() const
{
    return 120;
}

/// @brief Checks if the given value is in the danger zone.
/// @param value The value to check.
/// @return True if the value is in the danger zone, false otherwise.
bool OemOilPressureComponent::is_danger_condition(int32_t value) const
{
    return value <= OemOilPressureComponent::get_danger_zone();
}

/// @brief Sets up the danger zone section on the scale.
/// @param section The scale section to configure.
void OemOilPressureComponent::setup_danger_zone(lv_scale_section_t *section) const
{
    lv_scale_section_set_range(section, OemOilPressureComponent::get_scale_min(), OemOilPressureComponent::get_danger_zone());
}

/// @brief Gets the Y offset for the oil pressure icon.
/// @return The Y offset value.
int32_t OemOilPressureComponent::get_icon_y_offset() const
{
    return -55;
}