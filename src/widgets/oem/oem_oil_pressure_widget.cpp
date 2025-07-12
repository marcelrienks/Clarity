#include "widgets/oem/oem_oil_pressure_widget.h"

// Constructors and Destructors

OemOilPressureWidget::OemOilPressureWidget()
    : OemOilWidget()
{
    // Constructor delegates to base class
}

// Protected Methods

/// @brief Gets the icon for the oil pressure component.
/// @return Pointer to the icon image descriptor.
const lv_image_dsc_t* OemOilPressureWidget::get_icon() const
{
    return &oil_can_regular;
}

/// @brief Gets the minimum scale value for the oil pressure component.
/// @return The minimum scale value.
int32_t OemOilPressureWidget::get_scale_min() const
{
    return 0;
}

/// @brief Gets the maximum scale value for the oil pressure component.
/// @return The maximum scale value.
int32_t OemOilPressureWidget::get_scale_max() const
{
    return 60;
}

/// @brief Gets the danger zone value for the oil pressure component.
/// @return The danger zone value.
int32_t OemOilPressureWidget::get_danger_zone() const
{
    return 5;
}

/// @brief Gets the scale mode for the oil pressure component.
/// @return The scale mode.
lv_scale_mode_t OemOilPressureWidget::get_scale_mode() const
{
    return LV_SCALE_MODE_ROUND_INNER;
}

/// @brief Gets the angle range for the oil pressure component.
/// @return The angle range.
int32_t OemOilPressureWidget::get_angle_range() const
{
    return 120;
}

/// @brief Checks if the given value is in the danger zone.
/// @param value The value to check.
/// @return True if the value is in the danger zone, false otherwise.
bool OemOilPressureWidget::is_danger_condition(int32_t value) const
{
    return value <= OemOilPressureWidget::get_danger_zone();
}

/// @brief Sets up the danger zone section on the scale.
/// @param section The scale section to configure.
void OemOilPressureWidget::setup_danger_zone(lv_scale_section_t *section) const
{
    lv_scale_section_set_range(section, OemOilPressureWidget::get_scale_min(), OemOilPressureWidget::get_danger_zone());
}

/// @brief Gets the Y offset for the oil pressure icon.
/// @return The Y offset value.
int32_t OemOilPressureWidget::get_icon_y_offset() const
{
    return -55;
}

/// @brief Gets the label angles for L and H labels.
/// @param l_angle Reference to store the L label angle.
/// @param h_angle Reference to store the H label angle.
void OemOilPressureWidget::get_label_angles(int32_t& l_angle, int32_t& h_angle) const
{
    // Standard positioning for pressure component
    // L label: At _scale_rotation angle (low pressure start)
    l_angle = _scale_rotation;
    
    // H label: At _scale_rotation + angle_range (high pressure end)
    h_angle = _scale_rotation + get_angle_range();
}