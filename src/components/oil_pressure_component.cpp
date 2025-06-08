#include "components/oil_pressure_component.h"
#include "icons/oil_can_icon_data.h"

OilPressureComponent::OilPressureComponent()
{
    StyleManager &_styleManager = StyleManager::get_instance();

    // Initialize styles - DO NOT assign styles directly
    lv_style_init(&_indicator_part_style);
    lv_style_init(&_items_part_style);
    lv_style_init(&_main_part_style);
    lv_style_init(&_danger_section_items_part_style);
}

OilPressureComponent::~OilPressureComponent()
{
    // Clean up LVGL objects
    if (_needle_line)
        lv_obj_del(_needle_line);

    if (_scale)
        lv_obj_del(_scale);

    if (_oil_can_icon)
        lv_obj_del(_oil_can_icon);

    // Clean up styles
    lv_style_reset(&_indicator_part_style);
    lv_style_reset(&_items_part_style);
    lv_style_reset(&_main_part_style);
    lv_style_reset(&_danger_section_items_part_style);
}

/// @brief Initialise an oil pressure component to show the engine oil pressure
/// @param screen the screen on which to render the component
void OilPressureComponent::render_load(lv_obj_t *screen)
{
    log_d("...");

    _scale = lv_scale_create(screen);

    const StyleManager &styleManager = StyleManager::get_instance();

    // Apply theme color to our styles
    const ThemeColors &colours = styleManager.get_colours(styleManager.get_theme());
    lv_style_set_line_color(&_indicator_part_style, colours.gauge_normal);
    lv_style_set_line_color(&_items_part_style, colours.gauge_normal);
    lv_style_set_line_color(&_danger_section_items_part_style, colours.gauge_danger);

    // Now use our initialized styles
    lv_obj_add_style(_scale, &styleManager.background_style, MAIN_DEFAULT);

    lv_obj_set_pos(_scale, 0U, 0U);
    lv_obj_set_size(_scale, 240U, 240U);
    lv_obj_set_align(_scale, LV_ALIGN_TOP_MID);
    lv_scale_set_mode(_scale, LV_SCALE_MODE_ROUND_INNER);
    lv_scale_set_rotation(_scale, 200U);
    lv_scale_set_angle_range(_scale, 140U);
    lv_scale_set_range(_scale, 0U, 60U);

    // Adjust tick counts to match our new scale - still showing major ticks at whole numbers
    lv_scale_set_total_tick_count(_scale, 13U);
    lv_scale_set_major_tick_every(_scale, 2U);
    lv_scale_set_label_show(_scale, false);
    lv_obj_set_style_align(_scale, LV_ALIGN_CENTER, MAIN_DEFAULT);

    // Main style
    lv_style_set_arc_width(&_main_part_style, 0U);
    lv_obj_add_style(_scale, &_main_part_style, MAIN_DEFAULT);

    // Indicator (Major ticks)
    lv_style_set_length(&_indicator_part_style, 25U);
    lv_style_set_line_width(&_indicator_part_style, 7U);
    lv_obj_add_style(_scale, &_indicator_part_style, INDICATOR_DEFAULT);

    // Items (Minor ticks)
    lv_style_set_length(&_items_part_style, 18U);
    lv_style_set_line_width(&_items_part_style, 2U);
    lv_obj_add_style(_scale, &_items_part_style, ITEMS_DEFAULT);

    // Configure section styles
    lv_scale_section_t *section = lv_scale_add_section(_scale);
    lv_style_set_line_width(&_danger_section_items_part_style, 5U);
    lv_scale_section_set_style(section, MAIN_DEFAULT, &_main_part_style); // Apply the same 0 arc width to the section
    lv_scale_section_set_style(section, ITEMS_DEFAULT, &_danger_section_items_part_style);
    lv_scale_section_set_range(section, 0U, _danger_zone);

    // Add needle line
    _needle_line = lv_line_create(_scale);
    lv_obj_set_style_line_color(_needle_line, colours.gauge_normal, 0U);
    lv_obj_set_style_line_width(_needle_line, 5U, MAIN_DEFAULT);
    lv_obj_set_style_line_rounded(_needle_line, false, MAIN_DEFAULT);
    lv_scale_set_line_needle_value(_scale, _needle_line, _needle_length, 2U);

    // Create and position the oil can icon
    _oil_can_icon = lv_image_create(screen);
    lv_image_set_src(_oil_can_icon, &oil_can_icon_data);
    lv_obj_center(_oil_can_icon);
    lv_obj_set_pos(_oil_can_icon, 0, -55); // Adjust position relative to center
    lv_obj_set_style_opa(_oil_can_icon, LV_OPA_COVER, MAIN_DEFAULT);
    lv_obj_set_style_image_recolor(_oil_can_icon, colours.gauge_normal, MAIN_DEFAULT);
}

/// @brief Update the component by rendering the new reading
/// @param animation the animation object that will render the updated value
/// @param start the start value, this represents the initial value of the gauge currently
/// @param end the final reading that is gauge must display
void OilPressureComponent::render_update(lv_anim_t *animation, int32_t start, int32_t end)
{
    log_d("...");

    const ThemeColors &colours = StyleManager::get_instance().get_colours(StyleManager::get_instance().get_theme());
    lv_color_t colour = colours.gauge_normal;

    // Change color based on value
    if (end <= _danger_zone)
        colour = colours.gauge_danger;

    lv_obj_set_style_line_color(_needle_line, colour, MAIN_DEFAULT);

    // Also update the oil can icon color to match the needle
    lv_obj_set_style_image_recolor(_oil_can_icon, colour, MAIN_DEFAULT);
    lv_obj_set_style_image_recolor_opa(_oil_can_icon, LV_OPA_COVER, MAIN_DEFAULT);

    lv_anim_init(animation);
    lv_anim_set_duration(animation, _animation_duration);
    lv_anim_set_repeat_count(animation, 0U);
    lv_anim_set_playback_duration(animation, 0U);
    lv_anim_set_values(animation, start, end);
}

/// @brief Set the value of the line needle
/// @param value the value to set the line needle to
void OilPressureComponent::set_value(int32_t value)
{
    log_i("value is %i", value);
    lv_scale_set_line_needle_value(_scale, _needle_line, _needle_length, value);
}