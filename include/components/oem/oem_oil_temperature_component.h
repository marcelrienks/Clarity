#pragma once

// Project Includes
#include "components/oem/oem_oil_component.h"
#include "icons/oil_temp_regular.h"

/**
 * @class OemOilTemperatureComponent
 * @brief Oem styled Oil temperature gauge component with overheating protection
 *
 * @details This component specializes the OemOilComponent for oil temperature
 * monitoring. It displays a half circular gauge with temperature readings and
 * highlights dangerous overheating conditions. Due to it being Oem styled
 * it should take up the whole of the half of the screen to match the existing oem styled gauges.
 *
 * @specialization Oil Temperature Monitoring
 * @measurement_unit Degrees Celsius
 * @range 0-120°C (mapped range)
 * @scale Requires reverse value mapping in order to allow for counter clockwise needle rotation in lvgl.
 * @danger_zone Above 100°C (overheating warning)
 * @icon Oil temperature icon (oil_temp_regular.h)
 *
 * @position Intention: Only one half of the screen
 * @size 240x240 pixels: to ensure the arc of the scale occupies the total half of the display, while the other half has
 * no fill and can be overlayed with other components.
 * @data_source OilTemperatureSensor with delta-based updates
 *
 * @gauge_configuration:
 * - Scale: 120-0°C display range (mapped from 0-120°C actual)
 * - Rotation: Standard circular gauge
 * - Angle range: Full circle coverage
 * - Danger zone: 120-100°C display range (100-120°C actual)
 * - Value mapping: Implements reverse mapping for temperature display
 *
 * @visual_feedback:
 * - Normal: white needle and icon
 * - Danger: Red needle and icon for overheating
 * - Smooth needle animations between values
 *
 * @special_features:
 * - map_value_for_display(): Converts actual temperature to display range
 * - Reverse mapping: This is to accomodate lvgl limitation where reverse mapping is required for counter-clockwise
 * needle rotation.
 *
 * @context While the scale only occupies one half of the screen, it's size is equal to that of the screen to ensure
 * it fills the whole half of the rounded screen to maintain a consistent appearance with other OEM components.
 */
class OemOilTemperatureComponent : public OemOilComponent
{
  public:
    // ========== Constructors and Destructor ==========
    OemOilTemperatureComponent(IStyleService *styleService);
    ~OemOilTemperatureComponent() = default;

  protected:
    // ========== Protected Methods ==========
    const lv_image_dsc_t *get_icon() const override;
    int32_t get_scale_min() const override;
    int32_t get_scale_max() const override;
    int32_t get_danger_zone() const override;
    lv_scale_mode_t get_scale_mode() const override;
    int32_t get_angle_range() const override;
    bool is_danger_condition(int32_t value) const override;
    int32_t map_value_for_display(int32_t value) const override;
    void setup_danger_zone(lv_scale_section_t *section) const override;
    int32_t get_icon_y_offset() const override;
    void get_label_angles(int32_t &lAngle, int32_t &hAngle) const override;
};