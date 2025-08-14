#pragma once

// System/Library Includes
#include <lvgl.h>

// Project Includes
#include "interfaces/i_component.h"
#include "interfaces/i_style_service.h"
#include "utilities/types.h"

/**
 * @class OemOilComponent
 * @brief Abstract base class for oem styled oil monitoring gauge components
 *
 * @details This abstract class provides the common functionality for oem styled oil-related
 * gauge components (pressure, temperature). It implements the Template Method
 * design pattern, allowing derived classes to customize specific behaviors
 * while sharing common gauge rendering logic.
 *
 * @design_pattern Template Method - defines gauge creation algorithm
 * @view_role Renders circular gauges with needles, scales, and danger zones
 * @ui_elements Scale, needle, center icon, danger zone sections
 * @positioning Supports all ComponentLocation alignment options
 *
 * @gauge_specifications:
 * - Size: 240x240 pixels (configurable via ComponentLocation)
 * - Needle length: 90 pixels
 * - Scale ticks: 13 total, major every 2
 * - Danger zone: Red highlighting for critical values
 *
 * @derived_classes:
 * - OemOilPressureComponent: Oil pressure monitoring (0-100 PSI)
 * - OemOilTemperatureComponent: Oil temperature monitoring (mapped range)
 *
 * @virtual_methods Subclasses must implement:
 * - get_icon(): Component-specific icon
 * - get_scale_min/max(): Value range
 * - get_danger_zone(): Critical threshold
 * - setup_danger_zone(): Configure danger highlighting
 * - is_danger_condition(): Determine if value is critical
 *
 * @context This is the base class for oem styled oil gauges. Currently used by
 * pressure and temperature components. The components are positioned on opposite sides
 * on the screen to maintain a consistent appearance with OEM styling.
 */
class OemOilComponent : public IComponent
{
  public:
    // Constructors and Destructors
    OemOilComponent(IStyleService *styleService);
    virtual ~OemOilComponent();

    // Core Functionality Methods
    void Render(lv_obj_t *screen, const ComponentLocation &location, IDisplayProvider *display) override;
    void Refresh(const Reading &reading) override;
    void SetValue(int32_t value) override;

  protected:
    // Protected Data Members
    IStyleService *styleService_;

    // Protected Methods
    virtual const lv_image_dsc_t *get_icon() const = 0;
    virtual int32_t get_scale_min() const = 0;
    virtual int32_t get_scale_max() const = 0;
    virtual int32_t get_danger_zone() const = 0;
    virtual lv_scale_mode_t get_scale_mode() const = 0;
    virtual int32_t get_angle_range() const = 0;
    virtual bool is_danger_condition(int32_t value) const = 0;
    virtual int32_t map_value_for_display(int32_t value) const;
    virtual void setup_danger_zone(lv_scale_section_t *section) const = 0;
    virtual int32_t get_icon_y_offset() const = 0;
    virtual void get_label_angles(int32_t &lAngle, int32_t &hAngle) const = 0;

    // Protected Data Members
    // LVGL objects
    lv_obj_t *scale_;
    lv_obj_t *needleLine_;            // Tip section - thinnest
    lv_obj_t *needleMiddle_;          // Middle section - medium thickness
    lv_obj_t *needleBase_;            // Base section - thickest for smooth tapered appearance
    lv_obj_t *needleHighlightLine_;   // Highlight line for 3D effect - tip
    lv_obj_t *needleHighlightMiddle_; // Highlight line for 3D effect - middle
    lv_obj_t *needleHighlightBase_;   // Highlight line for 3D effect - base
    lv_obj_t *oilIcon_;
    lv_obj_t *lowLabel_;       // "L" label for low end
    lv_obj_t *highLabel_;      // "H" label for high end
    lv_obj_t *pivotCircle_;    // Main pivot circle
    lv_obj_t *pivotHighlight_; // Highlight on pivot for 3D effect

    // Common constants
    static constexpr int32_t NEEDLE_LENGTH = 90;

    // Scale rotation tracking for label positioning
    int32_t scaleRotation_;

  private:
    // Private Methods
    void create_icon();
    void create_labels();
    void create_needle();
    void create_scale(int32_t rotation);
    void update_pivot_styling();
};