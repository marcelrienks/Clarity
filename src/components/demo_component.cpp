#include "components/demo_component.h"

#include <memory>

DemoComponent::~DemoComponent()
{
    // Clean up LVGL objects
    if (_needle_line)
        lv_obj_del(_needle_line);

    if (_scale)
        lv_obj_del(_scale);
        
    // Clean up styles
    lv_style_reset(&_indicator_style);
    lv_style_reset(&_minor_ticks_style);
    lv_style_reset(&_main_line_style);
    lv_style_reset(&_section_label_style);
    lv_style_reset(&_section_minor_tick_style);
    lv_style_reset(&_section_main_line_style);
}

/// @brief Initialize a demo component to illustrate the use of a scale component
/// @param screen the screen on which to render the component
void DemoComponent::render_load(lv_obj_t *screen)
{
    log_d("...");

    _scale = lv_scale_create(screen);
    
    lv_obj_set_size(_scale, 150, 150);
    lv_scale_set_label_show(_scale, true);
    lv_scale_set_mode(_scale, LV_SCALE_MODE_ROUND_OUTER);
    lv_obj_center(_scale);

    lv_scale_set_total_tick_count(_scale, 21);
    lv_scale_set_major_tick_every(_scale, 5);

    lv_obj_set_style_length(_scale, 5, LV_PART_ITEMS);
    lv_obj_set_style_length(_scale, 10, LV_PART_INDICATOR);
    lv_scale_set_range(_scale, 0, 100);

    static const char *custom_labels[] = {"0 °C", "25 °C", "50 °C", "75 °C", "100 °C", NULL};
    lv_scale_set_text_src(_scale, custom_labels);

    // Initialize styles with error checking
    lv_style_init(&_indicator_style);
    lv_style_set_text_font(&_indicator_style, LV_FONT_DEFAULT);
    lv_style_set_text_color(&_indicator_style, lv_palette_darken(LV_PALETTE_BLUE, 3));
    lv_style_set_line_color(&_indicator_style, lv_palette_darken(LV_PALETTE_BLUE, 3));
    lv_style_set_width(&_indicator_style, 10U);
    lv_style_set_line_width(&_indicator_style, 2U);
    lv_obj_add_style(_scale, &_indicator_style, LV_PART_INDICATOR);

    lv_style_init(&_minor_ticks_style);
    lv_style_set_line_color(&_minor_ticks_style, lv_palette_lighten(LV_PALETTE_BLUE, 2));
    lv_style_set_width(&_minor_ticks_style, 5U);
    lv_style_set_line_width(&_minor_ticks_style, 2U);
    lv_obj_add_style(_scale, &_minor_ticks_style, LV_PART_ITEMS);

    lv_style_init(&_main_line_style);
    lv_style_set_arc_color(&_main_line_style, lv_palette_darken(LV_PALETTE_BLUE, 3));
    lv_style_set_arc_width(&_main_line_style, 2U);
    lv_obj_add_style(_scale, &_main_line_style, LV_PART_MAIN);

    // Initialize section styles
    lv_style_init(&_section_label_style);
    lv_style_init(&_section_minor_tick_style);
    lv_style_init(&_section_main_line_style);

    lv_style_set_text_font(&_section_label_style, LV_FONT_DEFAULT);
    lv_style_set_text_color(&_section_label_style, lv_palette_darken(LV_PALETTE_RED, 3));
    lv_style_set_line_color(&_section_label_style, lv_palette_darken(LV_PALETTE_RED, 3));
    lv_style_set_line_width(&_section_label_style, 5U);

    lv_style_set_line_color(&_section_minor_tick_style, lv_palette_lighten(LV_PALETTE_RED, 2));
    lv_style_set_line_width(&_section_minor_tick_style, 4U);

    lv_style_set_arc_color(&_section_main_line_style, lv_palette_darken(LV_PALETTE_RED, 3));
    lv_style_set_arc_width(&_section_main_line_style, 4U);

    // Configure section styles
    lv_scale_section_t *section = lv_scale_add_section(_scale);
    if (section != nullptr) {
        lv_scale_section_set_range(section, 75, 100);
        lv_scale_section_set_style(section, LV_PART_INDICATOR, &_section_label_style);
        lv_scale_section_set_style(section, LV_PART_ITEMS, &_section_minor_tick_style);
        lv_scale_section_set_style(section, LV_PART_MAIN, &_section_main_line_style);
    }

    // Create needle line
    _needle_line = lv_line_create(_scale);
    if (_needle_line != nullptr) {
        lv_obj_set_style_line_color(_needle_line, lv_palette_lighten(LV_PALETTE_INDIGO, 3), 0);
        lv_obj_set_style_line_width(_needle_line, 6, LV_PART_MAIN);
        lv_obj_set_style_line_rounded(_needle_line, true, LV_PART_MAIN);
        lv_scale_set_line_needle_value(_scale, _needle_line, 60, 0);
    }
}

/// @brief Update the component by rendering the new reading
/// @param animation the animation object that will render the updated value
/// @param start the start value, this represents the initial value of the gauge currently
/// @param end the final reading that is gauge must display
void DemoComponent::render_update(lv_anim_t *animation, int32_t start, int32_t end)
{
    log_d("...");

    lv_color_t colour = lv_palette_lighten(LV_PALETTE_INDIGO, 3);
    if (end >= 75)
        colour = lv_palette_darken(LV_PALETTE_RED, 3);

    lv_obj_set_style_line_color(_needle_line, colour, 0);

    lv_anim_init(animation);
    lv_anim_set_duration(animation, _animation_duration);
    lv_anim_set_repeat_count(animation, 0);
    lv_anim_set_playback_duration(animation, _playback_duration);
    lv_anim_set_values(animation, start, end);
}

/// @brief Set the value of the line needle
/// @param value the value to set the line needle to
void DemoComponent::set_value(int32_t value) {
    log_i("value is %i", value);
    lv_scale_set_line_needle_value(_scale, _needle_line, 60, value);
}