#include "components/oem/oem_oil_component.h"

OemOilComponent::OemOilComponent()
    : _scale(nullptr), _needle_line(nullptr), _oil_icon(nullptr)
{
    // Initialize styles - DO NOT assign styles directly
    initialize_styles();
}

OemOilComponent::~OemOilComponent()
{
    // Clean up LVGL objects
    if (_needle_line)
        lv_obj_del(_needle_line);

    if (_scale)
        lv_obj_del(_scale);

    if (_oil_icon)
        lv_obj_del(_oil_icon);

    // Clean up styles
    cleanup_styles();
}

/// @brief This method initializes the scale, needle, and icon for the oil component.
/// @param screen The screen object to render the component on.
void OemOilComponent::render_load(lv_obj_t *screen)
{
    log_d("...");

    // Create the scale
    _scale = lv_scale_create(screen);
    lv_obj_set_size(_scale, 240, 240);
    lv_obj_align(_scale, get_alignment(), 0, 0);

    // Setup scale properties based on derived class configuration
    setup_scale_properties();

    // Create needle and icon
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

    const ThemeColors &colours = StyleManager::get_instance().get_colours(StyleManager::get_instance().get_theme());
    lv_color_t colour = colours.gauge_normal;

    // Check danger condition based on derived class logic
    if (is_danger_condition(end))
        colour = colours.gauge_danger;

    // Update needle and icon colors
    lv_obj_set_style_line_color(_needle_line, colour, MAIN_DEFAULT);
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
    lv_scale_set_line_needle_value(_scale, _needle_line, _needle_length, mapped_value);
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

/// @brief Sets up the danger zone section on the scale.
void OemOilComponent::initialize_styles()
{
    lv_style_init(&_indicator_part_style);
    lv_style_init(&_items_part_style);
    lv_style_init(&_main_part_style);
    lv_style_init(&_danger_section_items_part_style);
}

/// @brief Cleans up the styles used by the oil component.
void OemOilComponent::cleanup_styles()
{
    lv_style_reset(&_indicator_part_style);
    lv_style_reset(&_items_part_style);
    lv_style_reset(&_main_part_style);
    lv_style_reset(&_danger_section_items_part_style);
}

/// @brief Sets up the scale properties for the oil component.
void OemOilComponent::setup_scale_properties()
{
    const StyleManager &styleManager = StyleManager::get_instance();
    const ThemeColors &colours = styleManager.get_colours(styleManager.get_theme());

    // Apply theme colors to styles
    lv_style_set_line_color(&_indicator_part_style, colours.gauge_normal);
    lv_style_set_line_color(&_items_part_style, colours.gauge_normal);
    lv_style_set_line_color(&_danger_section_items_part_style, colours.gauge_danger);

    // Set scale properties from derived class
    lv_scale_set_mode(_scale, get_scale_mode());
    lv_scale_set_rotation(_scale, get_rotation());
    lv_scale_set_angle_range(_scale, get_angle_range());
    lv_scale_set_range(_scale, get_scale_min(), get_scale_max());

    // Set tick properties
    lv_scale_set_total_tick_count(_scale, 13);
    lv_scale_set_major_tick_every(_scale, 2);
    lv_scale_set_label_show(_scale, false);

    // Apply styles to scale parts
    lv_style_set_arc_width(&_main_part_style, 0);
    lv_obj_add_style(_scale, &_main_part_style, MAIN_DEFAULT);

    // Indicator (Major ticks)
    lv_style_set_length(&_indicator_part_style, 25);
    lv_style_set_line_width(&_indicator_part_style, 7);
    lv_obj_add_style(_scale, &_indicator_part_style, INDICATOR_DEFAULT);

    // Items (Minor ticks)
    lv_style_set_length(&_items_part_style, 18);
    lv_style_set_line_width(&_items_part_style, 2);
    lv_obj_add_style(_scale, &_items_part_style, ITEMS_DEFAULT);

    // Create danger zone section
    lv_scale_section_t *section = lv_scale_add_section(_scale);
    lv_style_set_line_width(&_danger_section_items_part_style, 5);
    lv_scale_section_set_style(section, MAIN_DEFAULT, &_main_part_style);
    lv_scale_section_set_style(section, INDICATOR_DEFAULT, &_danger_section_items_part_style);
    lv_scale_section_set_style(section, ITEMS_DEFAULT, &_danger_section_items_part_style);
    
    // Set danger zone range - derived classes will handle specific ranges
    setup_danger_zone(section);
}

/// @brief Creates the needle line for the oil component.
void OemOilComponent::create_needle()
{
    const ThemeColors &colours = StyleManager::get_instance().get_colours(StyleManager::get_instance().get_theme());
    
    // Create needle line
    _needle_line = lv_line_create(_scale);
    lv_obj_set_style_line_color(_needle_line, colours.gauge_normal, MAIN_DEFAULT);
    lv_obj_set_style_line_width(_needle_line, 5, MAIN_DEFAULT);
    lv_obj_set_style_line_rounded(_needle_line, false, MAIN_DEFAULT);
    lv_obj_set_style_line_opa(_needle_line, LV_OPA_COVER, MAIN_DEFAULT);
}

/// @brief Creates the oil icon for the oil component.
void OemOilComponent::create_icon()
{
    const ThemeColors &colours = StyleManager::get_instance().get_colours(StyleManager::get_instance().get_theme());
    
    _oil_icon = lv_image_create(_scale);
    lv_image_set_src(_oil_icon, get_icon());
    lv_image_set_scale(_oil_icon, 50);
    lv_obj_align(_oil_icon, LV_ALIGN_CENTER, 0, get_icon_y_offset());
    lv_obj_set_style_opa(_oil_icon, LV_OPA_COVER, MAIN_DEFAULT);
    lv_obj_set_style_image_recolor(_oil_icon, colours.gauge_normal, MAIN_DEFAULT);
    lv_obj_set_style_image_recolor_opa(_oil_icon, LV_OPA_COVER, MAIN_DEFAULT);
}