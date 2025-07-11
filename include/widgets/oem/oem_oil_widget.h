#pragma once

// System/Library Includes
#include <lvgl.h>

// Project Includes
#include "interfaces/i_widget.h"
#include "managers/style_manager.h"
#include "utilities/types.h"

/**
 * @class OemOilWidget
 * @brief Abstract base class for oem styled oil monitoring gauge widgets
 * 
 * @details This abstract class provides the common functionality for oem styled oil-related
 * gauge widgets (pressure, temperature). It implements the Template Method
 * design pattern, allowing derived classes to customize specific behaviors
 * while sharing common gauge rendering logic.
 * 
 * @design_pattern Template Method - defines gauge creation algorithm
 * @view_role Renders circular gauges with needles, scales, and danger zones
 * @ui_elements Scale, needle, center icon, danger zone sections
 * @positioning Supports all WidgetLocation alignment options
 * 
 * @gauge_specifications:
 * - Size: 240x240 pixels (configurable via WidgetLocation)
 * - Needle length: 90 pixels
 * - Scale ticks: 13 total, major every 2
 * - Danger zone: Red highlighting for critical values
 * 
 * @derived_classes:
 * - OemOilPressureWidget: Oil pressure monitoring (0-100 PSI)
 * - OemOilTemperatureWidget: Oil temperature monitoring (mapped range)
 * 
 * @virtual_methods Subclasses must implement:
 * - get_icon(): Widget-specific icon
 * - get_scale_min/max(): Value range
 * - get_danger_zone(): Critical threshold
 * - setup_danger_zone(): Configure danger highlighting
 * - is_danger_condition(): Determine if value is critical
 * 
 * @context This is the base class for oem styled oil gauges. Currently used by
 * pressure and temperature widgets. The widgets are positioned on opposite sides
 * on the screen to maintain a consistent appearance with OEM styling.
 */
class OemOilWidget : public IWidget
{
public:
    // Constructors and Destructors
    OemOilWidget();
    virtual ~OemOilWidget();

    // Core Functionality Methods
    void render(lv_obj_t *screen, const WidgetLocation& location) override;
    void refresh(const Reading& reading) override;
    void set_value(int32_t value) override;

protected:
    // Protected Methods
    virtual const lv_image_dsc_t* get_icon() const = 0;
    virtual int32_t get_scale_min() const = 0;
    virtual int32_t get_scale_max() const = 0;
    virtual int32_t get_danger_zone() const = 0;
    virtual lv_scale_mode_t get_scale_mode() const = 0;
    virtual int32_t get_angle_range() const = 0;
    virtual bool is_danger_condition(int32_t value) const = 0;
    virtual int32_t map_value_for_display(int32_t value) const;
    virtual void setup_danger_zone(lv_scale_section_t *section) const = 0;
    virtual int32_t get_icon_y_offset() const = 0;
    virtual void get_label_angles(int32_t& l_angle, int32_t& h_angle) const = 0;

    // Protected Data Members
    // LVGL objects
    lv_obj_t *_scale;
    lv_obj_t *_needle_line;      // Tip section - thinnest
    lv_obj_t *_needle_middle;    // Middle section - medium thickness
    lv_obj_t *_needle_base;      // Base section - thickest for smooth tapered appearance
    lv_obj_t *_needle_highlight_line;    // Highlight line for 3D effect - tip
    lv_obj_t *_needle_highlight_middle;  // Highlight line for 3D effect - middle
    lv_obj_t *_needle_highlight_base;    // Highlight line for 3D effect - base
    lv_obj_t *_oil_icon;
    lv_obj_t *_low_label;        // "L" label for low end
    lv_obj_t *_high_label;       // "H" label for high end
    
    // Cached StyleManager reference (optimization)
    StyleManager* _style_manager;

    // Common constants
    static constexpr int32_t _needle_length = 90;
    
    // Scale rotation tracking for label positioning
    int32_t _scale_rotation;

private:
    // Private Methods
    void create_icon();
    void create_labels();
    void create_needle();
    void create_scale(int32_t rotation);
};