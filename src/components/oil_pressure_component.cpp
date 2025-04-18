#include "components/oil_pressure_component.h"

OilPressureComponent::OilPressureComponent()
{
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

    // Clean up styles
    lv_style_reset(&_indicator_part_style);
    lv_style_reset(&_items_part_style);
    lv_style_reset(&_main_part_style);
    lv_style_reset(&_danger_section_items_part_style);
}

/// @brief Initialise an oil pressure component to show the engine oil pressure
/// @param screen the screen on which to render the component
void OilPressureComponent::render_show(lv_obj_t *screen)
{
    log_d("...");

    // Scale
    _scale = lv_scale_create(screen);

    // Apply theme styles
    const StyleManager &styleManager = StyleManager::get_instance();

    // Apply theme color to our styles
    const ThemeColors &colors = styleManager.get_current_colors();
    lv_style_set_line_color(&_indicator_part_style, colors.gauge_normal);
    lv_style_set_line_color(&_items_part_style, colors.gauge_normal);
    lv_style_set_line_color(&_danger_section_items_part_style, colors.gauge_danger);

    // Now use our initialized styles
    lv_obj_add_style(_scale, &styleManager.background_style, MAIN_DEFAULT);

    lv_obj_set_pos(_scale, 0, 0);
    lv_obj_set_size(_scale, 240, 240);
    lv_obj_set_align(_scale, LV_ALIGN_TOP_MID);
    lv_scale_set_mode(_scale, LV_SCALE_MODE_ROUND_INNER);
    lv_scale_set_rotation(_scale, 200);
    lv_scale_set_angle_range(_scale, 140);
    lv_scale_set_range(_scale, 0, 6);
    lv_scale_set_total_tick_count(_scale, 13);
    lv_scale_set_major_tick_every(_scale, 2);
    lv_scale_set_label_show(_scale, false);
    lv_obj_set_style_align(_scale, LV_ALIGN_CENTER, MAIN_DEFAULT);

    // Main style
    lv_style_set_arc_width(&_main_part_style, 0U);
    lv_obj_add_style(_scale, &_main_part_style, MAIN_DEFAULT);

    // Indicator (Major ticks)
    lv_style_set_length(&_indicator_part_style, 30U);
    lv_style_set_line_width(&_indicator_part_style, 6U);
    lv_obj_add_style(_scale, &_indicator_part_style, INDICATOR_DEFAULT);

    // Items (Minor ticks)
    lv_style_set_length(&_items_part_style, 18U);
    lv_style_set_line_width(&_items_part_style, 3U);
    lv_obj_add_style(_scale, &_items_part_style, ITEMS_DEFAULT);

    // Configure section styles
    lv_scale_section_t *danger_section = lv_scale_add_section(_scale);
    lv_scale_section_set_style(danger_section, ITEMS_DEFAULT, &_danger_section_items_part_style);
    lv_scale_section_set_range(danger_section, 0U, 0.5);

    _needle_line = lv_line_create(_scale);
    lv_obj_set_style_line_color(_needle_line, colors.gauge_normal, 0);
    lv_obj_set_style_line_width(_needle_line, 6, MAIN_DEFAULT);
    lv_obj_set_style_line_rounded(_needle_line, true, MAIN_DEFAULT);
    lv_scale_set_line_needle_value(_scale, _needle_line, 60, 0);
}

/// @brief Update the component by rendering the new reading
/// @param animation the animation object that will render the updated value
/// @param start the start value, this represents the initial value of the gauge currently
/// @param end the final reading that is gauge must display
void OilPressureComponent::render_update(lv_anim_t *animation, int32_t start, int32_t end)
{
    log_d("...");

    const ThemeColors &colors = StyleManager::get_instance().get_current_colors();
    lv_color_t color = colors.gauge_normal;

    // Change color based on value
    if (end <= 0.5)
        color = colors.gauge_danger;

    else if (end >= 5.5)
        color = colors.gauge_warning;

    lv_obj_set_style_line_color(_needle_line, color, 0);

    lv_anim_init(animation);
    lv_anim_set_duration(animation, _animation_duration);
    lv_anim_set_repeat_count(animation, 0);
    lv_anim_set_playback_duration(animation, _playback_duration);
    lv_anim_set_values(animation, start, end);
}

/// @brief Set the value of the line needle
/// @param value the value to set the line needle to
void OilPressureComponent::set_value(int32_t value)
{
    log_i("value is %i", value);

    // Convert integer value to oil pressure range (assuming 0-100 input maps to 0-6 bar)
    float oil_pressure = (value / 100.0f) * 6.0f;
    lv_scale_set_line_needle_value(_scale, _needle_line, 60, oil_pressure);
}