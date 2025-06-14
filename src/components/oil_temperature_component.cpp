#include "components/oil_temperature_component.h"
#include "icons/oil_can_regular.h"
#include "icons/oil_temp_regular.h"

OilTemperatureComponent::OilTemperatureComponent()
{
    // Initialize styles - DO NOT assign styles directly
    lv_style_init(&_indicator_part_style);
    lv_style_init(&_items_part_style);
    lv_style_init(&_main_part_style);
    lv_style_init(&_danger_section_items_part_style);
}

OilTemperatureComponent::~OilTemperatureComponent()
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

/// @brief Initialise an oil temperature component to show the engine oil pressure
/// @param screen the screen on which to render the component
void OilTemperatureComponent::render_load(lv_obj_t *screen)
{
    log_d("...");

    _scale = lv_scale_create(screen);

    const StyleManager &styleManager = StyleManager::get_instance();

    // Apply theme color to our styles
    const ThemeColors &colours = styleManager.get_colours(styleManager.get_theme());
    lv_style_set_line_color(&_indicator_part_style, colours.gauge_normal);
    lv_style_set_line_color(&_items_part_style, colours.gauge_normal);
    lv_style_set_line_color(&_danger_section_items_part_style, colours.gauge_danger);

    lv_obj_set_size(_scale, 240, 240);
    lv_obj_align(_scale, LV_ALIGN_BOTTOM_MID, 0, 0);

    // Set scale properties
    lv_scale_set_mode(_scale, LV_SCALE_MODE_ROUND_INNER);
    lv_scale_set_rotation(_scale, 30);     // starting angle (0 = 3 o'clock)
    lv_scale_set_angle_range(_scale, 120); // range in degrees for the span of the scale
    lv_scale_set_range(_scale, 120, 0);    // the range of the scale, setting minimum to higher number draws the scale in reverse

    // Adjust tick counts
    lv_scale_set_total_tick_count(_scale, 13);
    lv_scale_set_major_tick_every(_scale, 2);
    lv_scale_set_label_show(_scale, true);

    // Main style
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

    // Danger Zone
    lv_scale_section_t *section = lv_scale_add_section(_scale);
    lv_style_set_line_width(&_danger_section_items_part_style, 5);
    lv_scale_section_set_style(section, MAIN_DEFAULT, &_main_part_style);
    lv_scale_section_set_style(section, INDICATOR_DEFAULT, &_danger_section_items_part_style);
    lv_scale_section_set_style(section, ITEMS_DEFAULT, &_danger_section_items_part_style);
    lv_scale_section_set_range(section, _danger_zone, 120);

    // Add needle line - restore original needle length
    _needle_line = lv_line_create(_scale);
    lv_obj_set_style_line_color(_needle_line, colours.gauge_normal, MAIN_DEFAULT);
    lv_obj_set_style_line_width(_needle_line, 5, MAIN_DEFAULT);
    lv_obj_set_style_line_rounded(_needle_line, false, MAIN_DEFAULT);
    lv_obj_set_style_line_opa(_needle_line, LV_OPA_COVER, MAIN_DEFAULT);
    // lv_obj_set_style_shadow_width(_needle_line, 2, MAIN_DEFAULT);
    // lv_obj_set_style_shadow_opa(_needle_line, LV_OPA_20, MAIN_DEFAULT);

    // Circle at pivot point
    auto _pivot_circle = lv_obj_create(_scale);
    lv_obj_set_size(_pivot_circle, 35, 35);
    lv_obj_center(_pivot_circle);
    lv_obj_set_style_radius(_pivot_circle, LV_RADIUS_CIRCLE, MAIN_DEFAULT);
    lv_obj_set_style_bg_color(_pivot_circle, lv_color_darken(colours.background, 25), MAIN_DEFAULT);
    lv_obj_set_style_border_width(_pivot_circle, 2, MAIN_DEFAULT);
    lv_obj_set_style_border_color(_pivot_circle, lv_color_lighten(colours.background, 15), MAIN_DEFAULT);
    lv_obj_set_style_shadow_color(_pivot_circle, lv_color_darken(colours.gauge_normal, 3), MAIN_DEFAULT);
    lv_obj_set_style_shadow_width(_pivot_circle, 1, MAIN_DEFAULT);
    lv_obj_set_style_shadow_opa(_pivot_circle, LV_OPA_10, MAIN_DEFAULT);
    lv_obj_set_style_shadow_spread(_pivot_circle, 3, MAIN_DEFAULT);

    // Create and position the oil can icon
    _oil_can_icon = lv_image_create(_scale);
    lv_image_set_src(_oil_can_icon, &oil_temp_regular);
    lv_image_set_scale(_oil_can_icon, 50); // 50/256 ≈ 19% of original
    lv_obj_align(_oil_can_icon, LV_ALIGN_CENTER, 0, 50);
    lv_obj_set_style_opa(_oil_can_icon, LV_OPA_COVER, MAIN_DEFAULT);
    lv_obj_set_style_image_recolor(_oil_can_icon, colours.gauge_normal, MAIN_DEFAULT);
    lv_obj_set_style_image_recolor_opa(_oil_can_icon, LV_OPA_COVER, MAIN_DEFAULT);

    log_d("rendered load");
}

/// @brief Update the component by rendering the new reading
/// @param animation the animation object that will render the updated value
/// @param start the start value, this represents the initial value of the gauge currently
/// @param end the final reading that is gauge must display
void OilTemperatureComponent::render_update(lv_anim_t *animation, int32_t start, int32_t end)
{
    log_d("...");

    const ThemeColors &colours = StyleManager::get_instance().get_colours(StyleManager::get_instance().get_theme());
    lv_color_t colour = colours.gauge_normal;

    // Change color based on value
    if (end >= _danger_zone)
        colour = colours.gauge_danger;

    lv_obj_set_style_line_color(_needle_line, colour, MAIN_DEFAULT);

    // Also update the oil can icon color to match the needle
    lv_obj_set_style_image_recolor(_oil_can_icon, colour, MAIN_DEFAULT);
    lv_obj_set_style_image_recolor_opa(_oil_can_icon, LV_OPA_COVER, MAIN_DEFAULT);

    lv_anim_init(animation);
    lv_anim_set_duration(animation, _animation_duration);
    lv_anim_set_repeat_count(animation, 0);
    lv_anim_set_playback_duration(animation, 0);

    // Due to a bug in LVGL 9.3, the scale needle does not support reversed values directly.
    // We need to map the value from [120,0] to [0,120] for proper needle positioning.
    int32_t mapped_start = map_reverse_value(start);
    int32_t mapped_end = map_reverse_value(end);

    log_d("original start is %i", start);
    log_d("original end is %i", end);
    log_d("mapped start is %i", mapped_start);
    log_d("mapped end is %i", mapped_end);
    
    lv_anim_set_values(animation, mapped_start, mapped_end);

    log_d("rendered update");
}

/// @brief Set the value of the line needle
/// @param value the value to set the line needle to
void OilTemperatureComponent::set_value(int32_t value)
{
    log_d("...");

    // Due to a bug in LVGL 9.3, the scale needle does not support reversed values directly.
    // We need to map the value from [120,0] to [0,120] for proper needle positioning.
    int32_t mapped_value = map_reverse_value(value);

    log_d("original value is %i", value);
    log_d("mapped value is %i", mapped_value);
    lv_scale_set_line_needle_value(_scale, _needle_line, _needle_length, mapped_value);
}

int32_t OilTemperatureComponent::map_reverse_value(int32_t value) const
{
    log_d("...");

    // Due to a bug in LVGL 9.3, the scale needle does not support reversed values directly.
    // We need to map the value from [120,0] to [0,120] for proper needle positioning.

    // Clamp input value to valid range
    if (value > _scale_min) value = _scale_min;
    if (value < _scale_max) value = _scale_max;
    
    // Map from [120,0] to [0,120] proportionally
    log_d("original value is %i", value);
    log_d("mapped value is %i", _scale_min - value);
    return _scale_min - value;
}