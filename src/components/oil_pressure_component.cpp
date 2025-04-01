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
    SerialLogger().log_point("OilPressureComponent::render_show()", "...");

    _scale = lv_scale_create(screen);
    lv_obj_set_style_pad_all(_scale, 10, _default_selector); // Set padding all around scale
    //lv_obj_set_style_bg_color(_scale, palette, _default_selector);
}

/// @brief Update the component by rendering the new reading
/// @param animation the animation object that will render the updated value
/// @param start the start value, this represents the initial value of the gauge currently
/// @param end the final reading that is gauge must display
void OilPressureComponent::render_update(lv_anim_t *animation, int32_t start, int32_t end)
{
    SerialLogger().log_point("OilPressureComponent::render_update()", "...");
}

/// @brief Set the value of the line needle
/// @param value the value to set the line needle to
void OilPressureComponent::set_value(int32_t value)
{
    SerialLogger().log_value("OilPressureComponent::set_value()", "value", std::to_string(value));
}