#include "components/oem/oem_oil_component.h"

OemOilComponent::OemOilComponent()
    : _scale(nullptr), _needle_line(nullptr), _needle_base(nullptr), _oil_icon(nullptr), _style_manager(&StyleManager::get_instance())
{
    // Cache StyleManager reference for performance
}

OemOilComponent::~OemOilComponent()
{
    // Clean up LVGL objects
    if (_needle_line) {
        lv_obj_del(_needle_line);
    }
    
    if (_needle_base) {
        lv_obj_del(_needle_base);
    }

    if (_scale) {
        lv_obj_del(_scale);
    }

    if (_oil_icon) {
        lv_obj_del(_oil_icon);
    }

    // No style cleanup needed - styles are managed by StyleManager
}

/// @brief This method initializes the scale, needle, and icon for the oil component with location parameters.
/// @param screen The screen object to render the component on.
/// @param location The location parameters for positioning the component.
void OemOilComponent::render_load(lv_obj_t *screen, const ComponentLocation &location)
{
    log_d("...");

    // Create the scale
    _scale = lv_scale_create(screen);
    lv_obj_set_size(_scale, 240, 240);

    // Apply location settings
    lv_obj_align(_scale, location.align, location.x_offset, location.y_offset);

    // Setup scale properties based on derived class configuration
    create_scale(location.rotation);
    create_needle();
    create_icon();

    log_d("rendered load");
}

/// @brief Updates the rendered oil component.
/// @param animation The animation object for the update.
/// @param start The starting value for the animation.
/// @param end The ending value for the animation.
void OemOilComponent::render_update(lv_anim_t *animation, int32_t start, int32_t end)
{
    log_d("...");

    const ThemeColors &colours = _style_manager->get_colours(_style_manager->get_theme());
    lv_color_t colour = colours.gauge_normal;

    // Check danger condition based on derived class logic
    if (is_danger_condition(end))
        colour = colours.gauge_danger;

    // Update needle and icon colors (both needle parts)
    lv_obj_set_style_line_color(_needle_line, colour, MAIN_DEFAULT);
    lv_obj_set_style_line_color(_needle_base, colour, MAIN_DEFAULT);  // Fix: Update base section too
    lv_obj_set_style_image_recolor(_oil_icon, colour, MAIN_DEFAULT);
    lv_obj_set_style_image_recolor_opa(_oil_icon, LV_OPA_COVER, MAIN_DEFAULT);

    // Setup animation
    lv_anim_init(animation);
    lv_anim_set_duration(animation, _animation_duration);
    lv_anim_set_repeat_count(animation, 0);
    lv_anim_set_playback_duration(animation, 0);
    lv_anim_set_values(animation, start, end);

    log_d("rendered update");
}

/// @brief Sets the value of the oil component.
/// This method updates the needle position based on the provided value.
/// @param value
void OemOilComponent::set_value(int32_t value)
{
    log_i("value is %i", value);

    // Allow derived classes to map values if needed (e.g., for reversed scales)
    int32_t mapped_value = map_value_for_display(value);
    
    // Update both needle parts for tapered appearance
    lv_scale_set_line_needle_value(_scale, _needle_line, _needle_length, mapped_value);
    lv_scale_set_line_needle_value(_scale, _needle_base, _needle_length / 2, mapped_value);  // Base is 1/2 length for better taper
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


/// @brief Sets up the scale properties for the oil component.
void OemOilComponent::create_scale(int32_t rotation)
{
    // Set scale properties from derived class
    lv_scale_set_mode(_scale, get_scale_mode());
    lv_scale_set_rotation(_scale, rotation);
    lv_scale_set_angle_range(_scale, get_angle_range());
    lv_scale_set_range(_scale, get_scale_min(), get_scale_max());

    // Set tick properties
    lv_scale_set_total_tick_count(_scale, 13);
    lv_scale_set_major_tick_every(_scale, 2);
    lv_scale_set_label_show(_scale, false);

    // Apply shared styles to scale parts
    lv_obj_add_style(_scale, _style_manager->get_gauge_main_style(), MAIN_DEFAULT);
    lv_obj_add_style(_scale, _style_manager->get_gauge_indicator_style(), INDICATOR_DEFAULT);
    lv_obj_add_style(_scale, _style_manager->get_gauge_items_style(), ITEMS_DEFAULT);

    // Create danger zone section
    lv_scale_section_t *section = lv_scale_add_section(_scale);
    lv_scale_section_set_style(section, MAIN_DEFAULT, _style_manager->get_gauge_main_style());
    lv_scale_section_set_style(section, INDICATOR_DEFAULT, _style_manager->get_gauge_danger_section_style());
    lv_scale_section_set_style(section, ITEMS_DEFAULT, _style_manager->get_gauge_danger_section_style());

    // Set danger zone range - derived classes will handle specific ranges
    setup_danger_zone(section);
}

/// @brief Creates the needle line for the oil component.
void OemOilComponent::create_needle()
{
    const ThemeColors &colours = _style_manager->get_colours(_style_manager->get_theme());

    // Create realistic white/silver needle with tapered appearance (based on actual car dashboard reference)
    _needle_line = lv_line_create(_scale);
    
    // Clean white/silver appearance (no shadows to prevent tearing)
    lv_obj_set_style_line_color(_needle_line, lv_color_hex(0xF0F0F0), MAIN_DEFAULT);  // Light silver
    lv_obj_set_style_line_width(_needle_line, 4, MAIN_DEFAULT);  // Good visibility
    lv_obj_set_style_line_rounded(_needle_line, true, MAIN_DEFAULT);  // Rounded ends for elegance
    lv_obj_set_style_line_opa(_needle_line, LV_OPA_COVER, MAIN_DEFAULT);
    
    // Create a thicker base section near pivot to simulate tapered appearance
    _needle_base = lv_line_create(_scale);
    lv_obj_set_style_line_color(_needle_base, lv_color_hex(0xF0F0F0), MAIN_DEFAULT);  // Same color
    lv_obj_set_style_line_width(_needle_base, 6, MAIN_DEFAULT);  // Thicker base
    lv_obj_set_style_line_rounded(_needle_base, true, MAIN_DEFAULT);
    lv_obj_set_style_line_opa(_needle_base, LV_OPA_COVER, MAIN_DEFAULT);

    // Realistic dark plastic pivot point (based on actual car dashboard reference)
    auto _pivot_circle = lv_obj_create(_scale);
    lv_obj_set_size(_pivot_circle, 32U, 32U);  // Slightly larger for better visibility
    lv_obj_center(_pivot_circle);
    lv_obj_set_style_radius(_pivot_circle, LV_RADIUS_CIRCLE, MAIN_DEFAULT);
    
    // Dark plastic appearance with radial gradient (light center to dark edge)
    lv_obj_set_style_bg_color(_pivot_circle, lv_color_hex(0x505050), MAIN_DEFAULT);  // Medium gray center
    lv_obj_set_style_bg_grad_color(_pivot_circle, lv_color_hex(0x2A2A2A), MAIN_DEFAULT);  // Dark gray edge
    lv_obj_set_style_bg_grad_dir(_pivot_circle, LV_GRAD_DIR_HOR, MAIN_DEFAULT);  // Horizontal for radial-like effect
    lv_obj_set_style_bg_grad_stop(_pivot_circle, 180, MAIN_DEFAULT);  // Gradient more toward edge
    
    // Dark beveled border (darker than main body)
    lv_obj_set_style_border_width(_pivot_circle, 2, MAIN_DEFAULT);
    lv_obj_set_style_border_color(_pivot_circle, lv_color_hex(0x1A1A1A), MAIN_DEFAULT);  // Very dark border
    
    // Subtle shadow for depth (less pronounced than metallic)
    lv_obj_set_style_shadow_color(_pivot_circle, lv_color_hex(0x000000), MAIN_DEFAULT);
    lv_obj_set_style_shadow_width(_pivot_circle, 3U, MAIN_DEFAULT);  // Moderate shadow
    lv_obj_set_style_shadow_opa(_pivot_circle, LV_OPA_20, MAIN_DEFAULT);  // Subtle
    lv_obj_set_style_shadow_spread(_pivot_circle, 1, MAIN_DEFAULT);
    lv_obj_set_style_shadow_offset_x(_pivot_circle, 1, MAIN_DEFAULT);
    lv_obj_set_style_shadow_offset_y(_pivot_circle, 1, MAIN_DEFAULT);
    
    // Center light pickup highlight (where light hits the plastic)
    auto _pivot_highlight = lv_obj_create(_pivot_circle);
    lv_obj_set_size(_pivot_highlight, 12U, 12U);  // Proportional to larger pivot
    lv_obj_center(_pivot_highlight);
    lv_obj_set_style_radius(_pivot_highlight, LV_RADIUS_CIRCLE, MAIN_DEFAULT);
    lv_obj_set_style_bg_color(_pivot_highlight, lv_color_hex(0x707070), MAIN_DEFAULT);  // Light gray highlight
    lv_obj_set_style_bg_opa(_pivot_highlight, LV_OPA_80, MAIN_DEFAULT);  // More opaque for plastic look
    lv_obj_set_style_border_width(_pivot_highlight, 0, MAIN_DEFAULT);
}

/// @brief Creates the oil icon for the oil component.
void OemOilComponent::create_icon()
{
    const ThemeColors &colours = _style_manager->get_colours(_style_manager->get_theme());

    _oil_icon = lv_image_create(_scale);
    lv_image_set_src(_oil_icon, get_icon());
    lv_image_set_scale(_oil_icon, 50);
    lv_obj_align(_oil_icon, LV_ALIGN_CENTER, 0, get_icon_y_offset());
    lv_obj_set_style_opa(_oil_icon, LV_OPA_COVER, MAIN_DEFAULT);
    lv_obj_set_style_image_recolor(_oil_icon, colours.gauge_normal, MAIN_DEFAULT);
    lv_obj_set_style_image_recolor_opa(_oil_icon, LV_OPA_COVER, MAIN_DEFAULT);
}