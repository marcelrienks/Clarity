#include "components/oil_pressure_component.h"

OilPressureComponent::OilPressureComponent()
{
    // Set defaults
    _indicator_part_style = StyleManager::get_instance().gauge_normal_style;
    _items_part_style = StyleManager::get_instance().gauge_normal_style;
    _main_part_style = StyleManager::get_instance().gauge_normal_style;
    _danger_section_items_part_style = StyleManager::get_instance().gauge_danger_style;
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
    lv_obj_add_style(_scale, &StyleManager::get_instance().gauge_normal_style, MAIN_DEFAULT);
    lv_obj_set_pos(_scale, 0, 0);
    lv_obj_set_size(_scale, 240, 240);
    lv_obj_set_align(_scale, LV_ALIGN_TOP_MID);
    lv_scale_set_mode(_scale, LV_SCALE_MODE_ROUND_INNER);
    lv_scale_set_rotation(_scale, 210);
    lv_scale_set_angle_range(_scale, 120);
    lv_scale_set_range(_scale, 0, 6);
    lv_scale_set_total_tick_count(_scale, 13);
    lv_scale_set_major_tick_every(_scale, 2);
    lv_scale_set_label_show(_scale, false);
    lv_obj_set_style_align(_scale, LV_ALIGN_CENTER, MAIN_DEFAULT);

    // TODO: is there value in having separate styles, as these can be set inline as per above?
    //  Unless they could be re-used in updates, I don't see the point

    // Main
    lv_style_set_arc_width(&_main_part_style, 0U);
    lv_obj_add_style(_scale, &_main_part_style, MAIN_DEFAULT);

    // Indicator (Major ticks)
    log_d("...0");
    if (lv_style_is_empty(&_indicator_part_style))
        log_d("null");
    log_d("...1");
    lv_style_set_width(&_indicator_part_style, 30);
    log_d("...2");
    lv_style_set_line_width(&_indicator_part_style, 6);
    log_d("...3");
    lv_obj_add_style(_scale, &_indicator_part_style, INDICATOR_DEFAULT);
    log_d("...4");

    // // Items (Minor ticks)
    // lv_style_set_length(&_items_part_style, 20U);
    // lv_style_set_line_width(&_items_part_style, 3U);
    // lv_obj_add_style(_scale, &_items_part_style, LV_PART_ITEMS);

    // // Configure section styles
    // lv_scale_section_t *section = lv_scale_add_section(_scale);
    // lv_scale_section_set_style(section, LV_PART_ITEMS, &_danger_section_items_part_style);
    // lv_scale_section_set_range(section, 0U, 0.5);
}

/// @brief Update the component by rendering the new reading
/// @param animation the animation object that will render the updated value
/// @param start the start value, this represents the initial value of the gauge currently
/// @param end the final reading that is gauge must display
void OilPressureComponent::render_update(lv_anim_t *animation, int32_t start, int32_t end)
{
    log_d("...");

    // lv_color_t color = lv_palette_lighten(LV_PALETTE_INDIGO, 3);
    // if (end >= 75)
    //     color = lv_palette_darken(LV_PALETTE_RED, 3);

    // lv_obj_set_style_line_color(_needle_line, color, 0);

    // lv_anim_init(animation);
    // lv_anim_set_duration(animation, _animation_duration);
    // lv_anim_set_repeat_count(animation, 0);
    // lv_anim_set_playback_duration(animation, _playback_duration);
    // lv_anim_set_values(animation, start, end);
}

/// @brief Set the value of the line needle
/// @param value the value to set the line needle to
void OilPressureComponent::set_value(int32_t value)
{
    log_i("value is %i", value);
    // lv_scale_set_line_needle_value(_scale, _needle_line, 60, value);
}