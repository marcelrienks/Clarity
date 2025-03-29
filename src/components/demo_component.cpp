#include "components/demo_component.h"

#include <memory>

DemoComponent::DemoComponent() {}

DemoComponent::~DemoComponent()
{
    if (_needle_line)
        lv_obj_del(_needle_line);

    if (_scale)
        lv_obj_del(_scale);
}

/// @brief Initialize a demo component to illustrate the use of a scale component
/// @param screen the screen on which to render the component
void DemoComponent::render_show(lv_obj_t *screen)
{
    SerialLogger().log_point("DemoComponent::render_show()", "...");

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

    static lv_style_t indicator_style;//TODO: where and how should these static types be cleaned up?
    lv_style_init(&indicator_style);

    // Label style properties
    lv_style_set_text_font(&indicator_style, LV_FONT_DEFAULT);
    lv_style_set_text_color(&indicator_style, lv_palette_darken(LV_PALETTE_BLUE, 3));

    // Major tick properties
    lv_style_set_line_color(&indicator_style, lv_palette_darken(LV_PALETTE_BLUE, 3));
    lv_style_set_width(&indicator_style, 10U);     // Tick length
    lv_style_set_line_width(&indicator_style, 2U); // Tick width
    lv_obj_add_style(_scale, &indicator_style, LV_PART_INDICATOR);

    static lv_style_t minor_ticks_style;
    lv_style_init(&minor_ticks_style);
    lv_style_set_line_color(&minor_ticks_style, lv_palette_lighten(LV_PALETTE_BLUE, 2));
    lv_style_set_width(&minor_ticks_style, 5U);      // tick width
    lv_style_set_line_width(&minor_ticks_style, 2U); // tick width
    lv_obj_add_style(_scale, &minor_ticks_style, LV_PART_ITEMS);

    static lv_style_t main_line_style;
    lv_style_init(&main_line_style);
    // Main line properties
    lv_style_set_arc_color(&main_line_style, lv_palette_darken(LV_PALETTE_BLUE, 3));
    lv_style_set_arc_width(&main_line_style, 2U); // tick width
    lv_obj_add_style(_scale, &main_line_style, LV_PART_MAIN);

    // Add a section
    static lv_style_t section_minor_tick_style;
    static lv_style_t section_label_style;
    static lv_style_t section_main_line_style;

    lv_style_init(&section_label_style);
    lv_style_init(&section_minor_tick_style);
    lv_style_init(&section_main_line_style);

    // Label style properties
    lv_style_set_text_font(&section_label_style, LV_FONT_DEFAULT);
    lv_style_set_text_color(&section_label_style, lv_palette_darken(LV_PALETTE_RED, 3));

    lv_style_set_line_color(&section_label_style, lv_palette_darken(LV_PALETTE_RED, 3));
    lv_style_set_line_width(&section_label_style, 5U); // tick width

    lv_style_set_line_color(&section_minor_tick_style, lv_palette_lighten(LV_PALETTE_RED, 2));
    lv_style_set_line_width(&section_minor_tick_style, 4U); // tick width

    // Main line properties
    lv_style_set_arc_color(&section_main_line_style, lv_palette_darken(LV_PALETTE_RED, 3));
    lv_style_set_arc_width(&section_main_line_style, 4U); // tick width

    // Configure section styles
    lv_scale_section_t *section = lv_scale_add_section(_scale);
    lv_scale_section_set_range(section, 75, 100);
    lv_scale_section_set_style(section, LV_PART_INDICATOR, &section_label_style);
    lv_scale_section_set_style(section, LV_PART_ITEMS, &section_minor_tick_style);
    lv_scale_section_set_style(section, LV_PART_MAIN, &section_main_line_style);

    this->_needle_line = lv_line_create(_scale);
    lv_obj_set_style_line_color(_needle_line, lv_palette_lighten(LV_PALETTE_INDIGO, 3), 0);
    lv_obj_set_style_line_width(_needle_line, 6, LV_PART_MAIN);
    lv_obj_set_style_line_rounded(_needle_line, true, LV_PART_MAIN);
    lv_scale_set_line_needle_value(_scale, _needle_line, 60, 0);
}

/// @brief Update the component by rendering the new reading
/// @param reading the reading that was received from the sensor
/// @param render_completion_callback the call back to be executed once rendering is complete
void DemoComponent::render_update(lv_anim_t *animation, int32_t start, int32_t end)
{
    SerialLogger().log_point("DemoComponent::render_update()", "...");

    lv_color_t color = lv_palette_lighten(LV_PALETTE_INDIGO, 3);
    if (start >= 75)
        color = lv_palette_lighten(LV_PALETTE_INDIGO, 3);

    lv_obj_set_style_line_color(_needle_line, color, 0);

    lv_anim_init(animation);
    lv_anim_set_duration(animation, _animation_duration);
    lv_anim_set_repeat_count(animation, 0);
    lv_anim_set_playback_duration(animation, _playback_duration);
    lv_anim_set_values(animation, start, end);
}

/// @brief Set the value of the line needle
/// @param value the value to set the line needle to
void DemoComponent::set_value(int32_t value) {
    SerialLogger().log_value("DemoComponent::set_value()", "value", std::to_string(value));
    lv_scale_set_line_needle_value(_scale, _needle_line, 60, value);
}