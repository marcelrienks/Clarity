#include "components/oem/oem_oil_component.h"
#include <math.h>

// Constructors and Destructors

OemOilComponent::OemOilComponent()
    : scale_(nullptr),
      needleLine_(nullptr),
      needleMiddle_(nullptr),
      needleBase_(nullptr),
      needleHighlightLine_(nullptr),
      needleHighlightMiddle_(nullptr),
      needleHighlightBase_(nullptr),
      oilIcon_(nullptr),
      lowLabel_(nullptr),
      highLabel_(nullptr),
      scaleRotation_(0),
      styleManager_(&StyleManager::get_instance())
{
    // Cache StyleManager reference for performance
}

OemOilComponent::~OemOilComponent()
{
    // Clean up LVGL objects
    if (needleLine_)
    {
        lv_obj_del(needleLine_);
    }

    if (needleMiddle_)
    {
        lv_obj_del(needleMiddle_);
    }

    if (needleBase_)
    {
        lv_obj_del(needleBase_);
    }

    if (needleHighlightLine_)
    {
        lv_obj_del(needleHighlightLine_);
    }

    if (needleHighlightMiddle_)
    {
        lv_obj_del(needleHighlightMiddle_);
    }

    if (needleHighlightBase_)
    {
        lv_obj_del(needleHighlightBase_);
    }

    if (scale_)
    {
        lv_obj_del(scale_);
    }

    if (oilIcon_)
    {
        lv_obj_del(oilIcon_);
    }

    if (lowLabel_)
    {
        lv_obj_del(lowLabel_);
    }

    if (highLabel_)
    {
        lv_obj_del(highLabel_);
    }

    // No style cleanup needed - styles are managed by StyleManager
}

// Core Functionality Methods

/// @brief This method initializes the scale, needle, and icon for the oil component with location parameters.
/// @param screen The screen object to render the component on.
/// @param location The location parameters for positioning the component.
void OemOilComponent::render(lv_obj_t *screen, const ComponentLocation &location)
{
    log_d("...");

    // Create the scale
    scale_ = lv_scale_create(screen);
    lv_obj_set_size(scale_, 240, 240);

    // Apply location settings
    lv_obj_align(scale_, location.align, location.x_offset, location.y_offset);

    // Setup scale properties based on derived class configuration
    create_scale(location.rotation);
    create_needle();
    create_icon();
    create_labels();

    log_d("rendered load");
}

/// @brief Updates the rendered oil component.
/// @param reading The Reading value to update the component with.
void OemOilComponent::refresh(const Reading& reading)
{
    log_d("...");

    int32_t value = std::get<int32_t>(reading);
    const ThemeColors &colours = styleManager_->get_colours(styleManager_->THEME);
    lv_color_t colour = colours.gaugeNormal;

    // Check danger condition based on derived class logic
    if (is_danger_condition(value))
        colour = colours.gaugeDanger;

    // Update needle and icon colors (all three needle sections with 3D gradient)
    if (is_danger_condition(value))
    {
        // Danger mode - bright danger color with gradient
        lv_obj_set_style_line_color(needleLine_, colours.needleDanger, MAIN_DEFAULT);                        // Bright danger tip
        lv_obj_set_style_line_color(needleMiddle_, lv_color_darken(colours.needleDanger, 10), MAIN_DEFAULT); // Medium danger middle
        lv_obj_set_style_line_color(needleBase_, lv_color_darken(colours.needleDanger, 20), MAIN_DEFAULT);   // Darker danger base

        // Highlight lines in danger mode - bright danger highlights
        lv_obj_set_style_line_color(needleHighlightLine_, lv_color_lighten(colours.needleDanger, 30), MAIN_DEFAULT);
        lv_obj_set_style_line_color(needleHighlightMiddle_, lv_color_lighten(colours.needleDanger, 20), MAIN_DEFAULT);
        lv_obj_set_style_line_color(needleHighlightBase_, lv_color_lighten(colours.needleDanger, 10), MAIN_DEFAULT);
    }
    else
    {
        // Normal mode - bright white gradient for 3D effect
        lv_obj_set_style_line_color(needleLine_, colours.needleNormal, MAIN_DEFAULT);                        // Bright white tip
        lv_obj_set_style_line_color(needleMiddle_, lv_color_darken(colours.needleNormal, 10), MAIN_DEFAULT); // Medium white middle
        lv_obj_set_style_line_color(needleBase_, lv_color_darken(colours.needleNormal, 20), MAIN_DEFAULT);   // Darker white base

        // Highlight lines in normal mode - bright white highlights
        lv_obj_set_style_line_color(needleHighlightLine_, colours.needleNormal, MAIN_DEFAULT);
        lv_obj_set_style_line_color(needleHighlightMiddle_, colours.needleNormal, MAIN_DEFAULT);
        lv_obj_set_style_line_color(needleHighlightBase_, colours.needleNormal, MAIN_DEFAULT);
    }
    lv_obj_set_style_image_recolor(oilIcon_, colour, MAIN_DEFAULT);
    lv_obj_set_style_image_recolor_opa(oilIcon_, LV_OPA_COVER, MAIN_DEFAULT);

    log_d("rendered update");
}

/// @brief Sets the value of the oil component.
/// This method updates the needle position based on the provided value.
/// @param value
void OemOilComponent::set_value(int32_t value)
{
    log_i("value is %i", value);

    // Allow derived classes to map values if needed (e.g., for reversed scales)
    int32_t mappedValue = map_value_for_display(value);

    // Update all three needle sections for smooth tapered appearance
    lv_scale_set_line_needle_value(scale_, needleLine_, NEEDLE_LENGTH, mappedValue);             // Full length (tip)
    lv_scale_set_line_needle_value(scale_, needleMiddle_, (NEEDLE_LENGTH * 2) / 3, mappedValue); // 2/3 length (middle)
    lv_scale_set_line_needle_value(scale_, needleBase_, NEEDLE_LENGTH / 3, mappedValue);         // 1/3 length (base)

    // Update highlight lines for 3D effect
    lv_scale_set_line_needle_value(scale_, needleHighlightLine_, NEEDLE_LENGTH - 2, mappedValue);               // Slightly shorter for highlight effect
    lv_scale_set_line_needle_value(scale_, needleHighlightMiddle_, ((NEEDLE_LENGTH * 2) / 3) - 2, mappedValue); // 2/3 length highlight
    lv_scale_set_line_needle_value(scale_, needleHighlightBase_, (NEEDLE_LENGTH / 3) - 2, mappedValue);         // 1/3 length highlight
}

/// @brief Maps the value for display on the oil component.
/// @param value The original value to map.
/// @return The mapped value for display.
int32_t OemOilComponent::map_value_for_display(int32_t value) const
{
    // Default implementation - no mapping
    // Derived classes can override for special mapping (e.g., temperature component)
    return value;
}

// Private Methods

/// @brief Creates the oil icon for the oil component.
void OemOilComponent::create_icon()
{
    const ThemeColors &colours = styleManager_->get_colours(styleManager_->THEME);

    oilIcon_ = lv_image_create(scale_);
    lv_image_set_src(oilIcon_, get_icon());
    lv_image_set_scale(oilIcon_, 50);
    lv_obj_align(oilIcon_, LV_ALIGN_CENTER, 0, get_icon_y_offset());
    lv_obj_set_style_opa(oilIcon_, LV_OPA_COVER, MAIN_DEFAULT);
    lv_obj_set_style_image_recolor(oilIcon_, colours.gaugeNormal, MAIN_DEFAULT);
    lv_obj_set_style_image_recolor_opa(oilIcon_, LV_OPA_COVER, MAIN_DEFAULT);
}

/// @brief Creates L and H labels positioned relative to the scale rotation and angle range.
/// Labels automatically follow when scale rotation changes.
void OemOilComponent::create_labels()
{
    const ThemeColors &colours = styleManager_->get_colours(styleManager_->THEME);

    // Create "L" label for low end
    lowLabel_ = lv_label_create(scale_);
    lv_label_set_text(lowLabel_, "L");
    lv_obj_set_style_text_color(lowLabel_, colours.gaugeNormal, MAIN_DEFAULT);
    lv_obj_set_style_text_font(lowLabel_, &lv_font_montserrat_18, MAIN_DEFAULT);

    // Create "H" label for high end
    highLabel_ = lv_label_create(scale_);
    lv_label_set_text(highLabel_, "H");
    lv_obj_set_style_text_color(highLabel_, colours.gaugeNormal, MAIN_DEFAULT);
    lv_obj_set_style_text_font(highLabel_, &lv_font_montserrat_18, MAIN_DEFAULT);

    // Calculate label positions based on scale rotation and angle range
    // Allow derived classes to customize label positioning (e.g., for reversed scales)
    int32_t lAngle, hAngle;
    get_label_angles(lAngle, hAngle);

    // Use same radius for both labels - they should be equidistant from pivot
    int32_t radius = 78;

    // L label position - use center alignment without fixed offsets
    double lAngleRad = (lAngle * M_PI) / 180.0;
    int32_t lX = (int32_t)(radius * cos(lAngleRad));
    int32_t lY = (int32_t)(radius * sin(lAngleRad));
    lv_obj_align(lowLabel_, LV_ALIGN_CENTER, lX, lY);

    // H label position - use center alignment without fixed offsets
    double hAngleRad = (hAngle * M_PI) / 180.0;
    int32_t hX = (int32_t)(radius * cos(hAngleRad));
    int32_t hY = (int32_t)(radius * sin(hAngleRad));
    lv_obj_align(highLabel_, LV_ALIGN_CENTER, hX, hY);
}

/// @brief Creates the needle line for the oil component.
void OemOilComponent::create_needle()
{
    const ThemeColors &colours = styleManager_->get_colours(styleManager_->THEME);

    // Create realistic 3-section tapered needle (based on actual car dashboard reference)

    // Section 1: Tip section with enhanced 3D effect - thinnest (outer third)
    needleLine_ = lv_line_create(scale_);
    lv_obj_set_style_line_color(needleLine_, colours.needleNormal, MAIN_DEFAULT); // Bright white from theme
    lv_obj_set_style_line_width(needleLine_, 4, MAIN_DEFAULT);                     // Slightly thicker tip
    lv_obj_set_style_line_rounded(needleLine_, true, MAIN_DEFAULT);                // Rounded ends
    lv_obj_set_style_line_opa(needleLine_, LV_OPA_COVER, MAIN_DEFAULT);

    // Section 2: Middle section with enhanced 3D effect - medium thickness (middle third)
    needleMiddle_ = lv_line_create(scale_);
    lv_obj_set_style_line_color(needleMiddle_, lv_color_darken(colours.needleNormal, 10), MAIN_DEFAULT); // Slightly darker for gradient
    lv_obj_set_style_line_width(needleMiddle_, 5, MAIN_DEFAULT);                                          // Thicker medium section
    lv_obj_set_style_line_rounded(needleMiddle_, true, MAIN_DEFAULT);
    lv_obj_set_style_line_opa(needleMiddle_, LV_OPA_COVER, MAIN_DEFAULT);

    // Section 3: Base section with enhanced 3D effect - thickest (inner third near pivot)
    needleBase_ = lv_line_create(scale_);
    lv_obj_set_style_line_color(needleBase_, lv_color_darken(colours.needleNormal, 20), MAIN_DEFAULT); // Darkest for gradient
    lv_obj_set_style_line_width(needleBase_, 7, MAIN_DEFAULT);                                          // Thickest base section
    lv_obj_set_style_line_rounded(needleBase_, true, MAIN_DEFAULT);
    lv_obj_set_style_line_opa(needleBase_, LV_OPA_COVER, MAIN_DEFAULT);

    // Add subtle highlight lines for enhanced 3D effect (very subtle to avoid artifacts)

    // Highlight for tip section - very subtle white highlight
    needleHighlightLine_ = lv_line_create(scale_);
    lv_obj_set_style_line_color(needleHighlightLine_, lv_color_hex(0xFFFFFF), MAIN_DEFAULT); // Pure white highlight
    lv_obj_set_style_line_width(needleHighlightLine_, 1, MAIN_DEFAULT);                      // Thin highlight line
    lv_obj_set_style_line_rounded(needleHighlightLine_, true, MAIN_DEFAULT);
    lv_obj_set_style_line_opa(needleHighlightLine_, LV_OPA_20, MAIN_DEFAULT); // Very subtle

    // Highlight for middle section
    needleHighlightMiddle_ = lv_line_create(scale_);
    lv_obj_set_style_line_color(needleHighlightMiddle_, lv_color_hex(0xFFFFFF), MAIN_DEFAULT); // Pure white highlight
    lv_obj_set_style_line_width(needleHighlightMiddle_, 1, MAIN_DEFAULT);                      // Thin highlight line
    lv_obj_set_style_line_rounded(needleHighlightMiddle_, true, MAIN_DEFAULT);
    lv_obj_set_style_line_opa(needleHighlightMiddle_, LV_OPA_20, MAIN_DEFAULT); // Very subtle

    // Highlight for base section
    needleHighlightBase_ = lv_line_create(scale_);
    lv_obj_set_style_line_color(needleHighlightBase_, lv_color_hex(0xFFFFFF), MAIN_DEFAULT); // Pure white highlight
    lv_obj_set_style_line_width(needleHighlightBase_, 1, MAIN_DEFAULT);                      // Thin highlight line
    lv_obj_set_style_line_rounded(needleHighlightBase_, true, MAIN_DEFAULT);
    lv_obj_set_style_line_opa(needleHighlightBase_, LV_OPA_20, MAIN_DEFAULT); // Very subtle

    // Realistic dark plastic pivot point (based on actual car dashboard reference)
    auto _pivot_circle = lv_obj_create(scale_);
    lv_obj_set_size(_pivot_circle, 40U, 40U); // Even larger for better visibility and proportion
    lv_obj_center(_pivot_circle);
    lv_obj_set_style_radius(_pivot_circle, LV_RADIUS_CIRCLE, MAIN_DEFAULT);

    // Dark plastic appearance with radial gradient (light center to dark edge)
    lv_obj_set_style_bg_color(_pivot_circle, lv_color_hex(0x505050), MAIN_DEFAULT);      // Medium gray center
    lv_obj_set_style_bg_grad_color(_pivot_circle, lv_color_hex(0x2A2A2A), MAIN_DEFAULT); // Dark gray edge
    lv_obj_set_style_bg_grad_dir(_pivot_circle, LV_GRAD_DIR_HOR, MAIN_DEFAULT);          // Horizontal for radial-like effect
    lv_obj_set_style_bg_grad_stop(_pivot_circle, 180, MAIN_DEFAULT);                     // Gradient more toward edge

    // Dark beveled border (darker than main body)
    lv_obj_set_style_border_width(_pivot_circle, 2, MAIN_DEFAULT);
    lv_obj_set_style_border_color(_pivot_circle, lv_color_hex(0x1A1A1A), MAIN_DEFAULT); // Very dark border

    // Subtle shadow for depth (less pronounced than metallic)
    lv_obj_set_style_shadow_color(_pivot_circle, lv_color_hex(0x000000), MAIN_DEFAULT);
    lv_obj_set_style_shadow_width(_pivot_circle, 3U, MAIN_DEFAULT);      // Moderate shadow
    lv_obj_set_style_shadow_opa(_pivot_circle, LV_OPA_20, MAIN_DEFAULT); // Subtle
    lv_obj_set_style_shadow_spread(_pivot_circle, 1, MAIN_DEFAULT);
    lv_obj_set_style_shadow_offset_x(_pivot_circle, 1, MAIN_DEFAULT);
    lv_obj_set_style_shadow_offset_y(_pivot_circle, 1, MAIN_DEFAULT);

    // Center light pickup highlight (where light hits the plastic)
    auto _pivot_highlight = lv_obj_create(_pivot_circle);
    lv_obj_set_size(_pivot_highlight, 16U, 16U); // Proportional to larger pivot
    lv_obj_center(_pivot_highlight);
    lv_obj_set_style_radius(_pivot_highlight, LV_RADIUS_CIRCLE, MAIN_DEFAULT);
    lv_obj_set_style_bg_color(_pivot_highlight, lv_color_hex(0x707070), MAIN_DEFAULT); // Light gray highlight
    lv_obj_set_style_bg_opa(_pivot_highlight, LV_OPA_80, MAIN_DEFAULT);                // More opaque for plastic look
    lv_obj_set_style_border_width(_pivot_highlight, 0, MAIN_DEFAULT);
}

/// @brief Sets up the scale properties for the oil component.
void OemOilComponent::create_scale(int32_t rotation)
{
    // Store rotation for label positioning
    scaleRotation_ = rotation;

    // Set scale properties from derived class
    lv_scale_set_mode(scale_, get_scale_mode());
    lv_scale_set_rotation(scale_, rotation);
    lv_scale_set_angle_range(scale_, get_angle_range());
    lv_scale_set_range(scale_, get_scale_min(), get_scale_max());

    // Set tick properties to match fuel gauge pattern
    lv_scale_set_total_tick_count(scale_, 13);
    lv_scale_set_major_tick_every(scale_, 3);
    lv_scale_set_label_show(scale_, false); // Disable built-in labels, use custom L/H positioning

    // Apply shared styles to scale parts
    lv_obj_add_style(scale_, &styleManager_->gaugeMainStyle, MAIN_DEFAULT);
    lv_obj_add_style(scale_, &styleManager_->gaugeIndicatorStyle, INDICATOR_DEFAULT);
    lv_obj_add_style(scale_, &styleManager_->gaugeItemsStyle, ITEMS_DEFAULT);

    // Create danger zone section
    lv_scale_section_t *section = lv_scale_add_section(scale_);
    lv_scale_section_set_style(section, MAIN_DEFAULT, &styleManager_->gaugeMainStyle);
    lv_scale_section_set_style(section, INDICATOR_DEFAULT, &styleManager_->gaugeDangerSectionStyle);
    lv_scale_section_set_style(section, ITEMS_DEFAULT, &styleManager_->gaugeDangerSectionStyle);

    // Set danger zone range - derived classes will handle specific ranges
    setup_danger_zone(section);
}