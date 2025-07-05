#pragma once

#include "components/oem/oem_oil_component.h"
#include "icons/oil_can_regular.h"

/**
 * @class OemOilPressureComponent
 * @brief Oem styled Oil pressure gauge component with danger zone monitoring
 * 
 * @details This component specializes the OemOilComponent for oil pressure
 * monitoring. It displays a half circular gauge with pressure readings and
 * highlights dangerous low-pressure conditions. Due to it being Oem styled
 * it should take up the whole of the half of the screen to match the existing oem styled gauges.
 * 
 * @specialization Oil Pressure Monitoring
 * @measurement_unit Bar (100 Kpa)
 * @range 0-6 Bar
 * @scale 0-60 mapping values for decimal precision
 * @danger_zone Below 2 Bar (low pressure warning)
 * @icon Oil can icon (oil_can_regular.h)
 * 
* @position Intention: Only one half of the screen
 * @size 240x240 pixels: to ensure the arc of the scale occupies the total half of the display, while the other half has no fill and can be overlayed with other components.
 * @data_source OilPressureSensor with delta-based updates
 * 
 * @gauge_configuration:
 * - Scale: 0-60 mapping values for decimal precision
 * - Rotation: Standard circular gauge
 * - Angle range: Full circle coverage
 * - Danger zone: 0-2 Bar highlighted in red
 * 
 * @visual_feedback:
 * - Normal: white needle and icon
 * - Danger: Red needle and icon for low pressure
 * - Smooth needle animations between values
 *
* @context While the scale only occupies one half of the screen, it's size is equal to that of the screen to ensure
 * it fills the whole half of the rounded screen to maintain a consistent appearance with other OEM components.
 */
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
    lv_scale_mode_t get_scale_mode() const override;
    int32_t get_angle_range() const override;
    bool is_danger_condition(int32_t value) const override;
    void setup_danger_zone(lv_scale_section_t *section) const override;
    int32_t get_icon_y_offset() const override;
    void get_label_angles(int32_t& l_angle, int32_t& h_angle) const override;
};