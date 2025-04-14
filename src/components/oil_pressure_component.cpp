#include "components/oil_pressure_component.h"

OilPressureComponent::~OilPressureComponent()
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

/// @brief Initialise an oil pressure component to show the engine oil pressure
/// @param screen the screen on which to render the component
void OilPressureComponent::render_show(lv_obj_t *screen)
{
    log_d("...");

    _scale = lv_scale_create(screen);

    lv_obj_set_size(_scale, 240, 240);
    lv_scale_set_mode(_scale, LV_SCALE_MODE_ROUND_INNER);
    lv_obj_set_style_bg_opa(_scale, LV_OPA_COVER, DEFAULT_SELECTOR);
    lv_obj_set_style_bg_color(_scale, lv_palette_lighten(LV_PALETTE_RED, 5), DEFAULT_SELECTOR);
    lv_obj_set_style_radius(_scale, LV_RADIUS_CIRCLE, DEFAULT_SELECTOR);
    lv_obj_set_style_pad_all(_scale, 10, _default_selector); // Set padding all around scale
    lv_obj_set_style_clip_corner(_scale, true, DEFAULT_SELECTOR);
    //lv_obj_align(_scale, LV_ALIGN_LEFT_MID, LV_PCT(2), 0);

    lv_scale_set_label_show(_scale, true);

    lv_scale_set_total_tick_count(_scale, 6);
    lv_scale_set_major_tick_every(_scale, 1);

    lv_obj_set_style_length(_scale, 5, LV_PART_ITEMS);
    lv_obj_set_style_length(_scale, 10, LV_PART_INDICATOR);
    lv_scale_set_range(_scale, 0, 5);

    lv_scale_set_angle_range(_scale, 210);
    lv_scale_set_rotation(_scale, 330);

    auto needle_line = lv_line_create(_scale);
    lv_obj_set_style_line_width(needle_line, 6, LV_PART_MAIN);
    lv_obj_set_style_line_rounded(needle_line, true, LV_PART_MAIN);
    
}

/// @brief Update the component by rendering the new reading
/// @param animation the animation object that will render the updated value
/// @param start the start value, this represents the initial value of the gauge currently
/// @param end the final reading that is gauge must display
void OilPressureComponent::render_update(lv_anim_t *animation, int32_t start, int32_t end)
{
    log_d("...");
}

/// @brief Set the value of the line needle
/// @param value the value to set the line needle to
void OilPressureComponent::set_value(int32_t value)
{
    log_i("value is %i", value);
}