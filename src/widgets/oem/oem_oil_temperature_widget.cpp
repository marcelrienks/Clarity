#include "widgets/oem/oem_oil_temperature_widget.h"

// Constructors and Destructors

OemOilTemperatureWidget::OemOilTemperatureWidget()
    : OemOilWidget()
{
    // Constructor delegates to base class
}

// Protected Methods

/// @brief Gets the icon for the oil temperature component.
/// @return Pointer to the icon image descriptor.
const lv_image_dsc_t* OemOilTemperatureWidget::get_icon() const
{
    return &oil_temp_regular;
}

/// @brief Gets the minimum scale value for the oil temperature component.
/// @return The minimum scale value.
int32_t OemOilTemperatureWidget::get_scale_min() const
{
    return 0;
}

/// @brief Gets the maximum scale value for the oil temperature component.
/// @return The maximum scale value.
int32_t OemOilTemperatureWidget::get_scale_max() const
{
    return 120;
}

/// @brief Gets the danger zone value for the oil temperature component.
/// @return The danger zone value.
int32_t OemOilTemperatureWidget::get_danger_zone() const
{
    return 100;
}

/// @brief Gets the scale mode for the oil temperature component.
/// @return The scale mode.
lv_scale_mode_t OemOilTemperatureWidget::get_scale_mode() const
{
    return LV_SCALE_MODE_ROUND_INNER;
}

/// @brief Gets the angle range for the oil temperature component.
/// @return The angle range.
int32_t OemOilTemperatureWidget::get_angle_range() const
{
    return 120;
}

/// @brief Checks if the given value is in the danger zone.
/// @param value The value to check.
/// @return True if the value is in the danger zone, false otherwise.
bool OemOilTemperatureWidget::is_danger_condition(int32_t value) const
{
    return value >= OemOilTemperatureWidget::get_danger_zone();
}

/// @brief Maps the value for display on the oil temperature component.
/// @param value The value to map.
/// @return The mapped value.
int32_t OemOilTemperatureWidget::map_value_for_display(int32_t value) const
{
    // Maps the value from the original scale (0-120) to the normal scale (120-0).
    // LVGL 9.3 has a bug that does allow setting a reverse scale 120-0, but it cannot animate the needle using that.
    // This method is used to map the value from the original scale (0-120) to the normal scale (120-0).
    
    log_d("original value is %i", value);
    
    // Map from [0,120] to [120,0] reverse the scale
    int32_t mapped_value = OemOilTemperatureWidget::get_scale_max() - value;
    
    log_d("mapped value is %i", mapped_value);
    
    return mapped_value;
}

/// @brief Sets up the danger zone section on the scale.
/// @param section The scale section to configure.
void OemOilTemperatureWidget::setup_danger_zone(lv_scale_section_t *section) const
{
    // Danger zone: map correct danger zone to reversed danger zone (hack to solve reversed scale in LVGL 9.3)
    lv_scale_section_set_range(section, map_value_for_display(OemOilTemperatureWidget::get_scale_max()), map_value_for_display(OemOilTemperatureWidget::get_danger_zone()));
}

/// @brief Gets the Y offset for the oil temperature icon.
/// @return The Y offset value.
int32_t OemOilTemperatureWidget::get_icon_y_offset() const
{
    return 55;
}

/// @brief Gets the label angles for L and H labels with swapped positioning.
/// @param l_angle Reference to store the L label angle.
/// @param h_angle Reference to store the H label angle.
void OemOilTemperatureWidget::get_label_angles(int32_t& l_angle, int32_t& h_angle) const
{
    // Swap L and H positioning due to reversed scale mapping
    // H label: At _scale_rotation angle (where L would normally be)
    h_angle = _scale_rotation;
    
    // L label: At _scale_rotation + angle_range (where H would normally be)
    l_angle = _scale_rotation + get_angle_range();
}