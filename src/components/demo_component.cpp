#include "components/demo_component.h"

DemoComponent *g_demo_component_instance = nullptr;

/// @brief DemoScreen constructor, generates a _scale with a needle line
DemoComponent::DemoComponent()
{
    g_demo_component_instance = this;
}

/// @brief Initialize the component
void DemoComponent::init(lv_obj_t *virtual_screen)
{
    _scale = lv_scale_create(virtual_screen);
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

    static lv_style_t indicator_style;
    lv_style_init(&indicator_style);

    /* Label style properties */
    lv_style_set_text_font(&indicator_style, LV_FONT_DEFAULT);
    lv_style_set_text_color(&indicator_style, lv_palette_darken(LV_PALETTE_BLUE, 3));

    /* Major tick properties */
    lv_style_set_line_color(&indicator_style, lv_palette_darken(LV_PALETTE_BLUE, 3));
    lv_style_set_width(&indicator_style, 10U);     /*Tick length*/
    lv_style_set_line_width(&indicator_style, 2U); /*Tick width*/
    lv_obj_add_style(_scale, &indicator_style, LV_PART_INDICATOR);

    static lv_style_t minor_ticks_style;
    lv_style_init(&minor_ticks_style);
    lv_style_set_line_color(&minor_ticks_style, lv_palette_lighten(LV_PALETTE_BLUE, 2));
    lv_style_set_width(&minor_ticks_style, 5U);      /*Tick length*/
    lv_style_set_line_width(&minor_ticks_style, 2U); /*Tick width*/
    lv_obj_add_style(_scale, &minor_ticks_style, LV_PART_ITEMS);

    static lv_style_t main_line_style;
    lv_style_init(&main_line_style);
    /* Main line properties */
    lv_style_set_arc_color(&main_line_style, lv_palette_darken(LV_PALETTE_BLUE, 3));
    lv_style_set_arc_width(&main_line_style, 2U); /*Tick width*/
    lv_obj_add_style(_scale, &main_line_style, LV_PART_MAIN);

    /* Add a section */
    static lv_style_t section_minor_tick_style;
    static lv_style_t section_label_style;
    static lv_style_t section_main_line_style;

    lv_style_init(&section_label_style);
    lv_style_init(&section_minor_tick_style);
    lv_style_init(&section_main_line_style);

    /* Label style properties */
    lv_style_set_text_font(&section_label_style, LV_FONT_DEFAULT);
    lv_style_set_text_color(&section_label_style, lv_palette_darken(LV_PALETTE_RED, 3));

    lv_style_set_line_color(&section_label_style, lv_palette_darken(LV_PALETTE_RED, 3));
    lv_style_set_line_width(&section_label_style, 5U); /*Tick width*/

    lv_style_set_line_color(&section_minor_tick_style, lv_palette_lighten(LV_PALETTE_RED, 2));
    lv_style_set_line_width(&section_minor_tick_style, 4U); /*Tick width*/

    /* Main line properties */
    lv_style_set_arc_color(&section_main_line_style, lv_palette_darken(LV_PALETTE_RED, 3));
    lv_style_set_arc_width(&section_main_line_style, 4U); /*Tick width*/

    /* Configure section styles */
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

    // Clock check animation
    DemoComponent::animate_needle(1000, 1000, _current_reading, 100);

    this->_start_time = millis();
}

/// @brief Change the value of the needle line
/// @param value the value to set the needle line to
void DemoComponent::update(std::shared_ptr<void> ptr)
{
    int value = *std::static_pointer_cast<int>(ptr);  // Cast to int

    if (millis() - _start_time < 3000)
    {
        this->_current_reading = 0;
        return;
    }

    if (this->_current_reading == value)
        return;

    if (value >= 75)
        lv_obj_set_style_line_color(_needle_line, lv_palette_darken(LV_PALETTE_RED, 3), 0);

    else
        lv_obj_set_style_line_color(_needle_line, lv_palette_lighten(LV_PALETTE_INDIGO, 3), 0);

    DemoComponent::animate_needle(1000, 0, _current_reading, value);

    this->_current_reading = value;
}

/// @brief Animate the needle line smoothly
/// @param animation_duration the duration of the animation/transition
/// @param playback_duration the duration of the playback
/// @param start the starting value of the needle line
/// @param end the ending value of the needle line
void DemoComponent::animate_needle(int16_t animation_duration, int16_t playback_duration, int32_t start, int32_t end)
{
    static lv_anim_t animate_scale_line;

    lv_anim_init(&animate_scale_line);
    lv_anim_set_var(&animate_scale_line, this->_needle_line);
    lv_anim_set_exec_cb(&animate_scale_line, DemoComponent::set_needle_line_value_callback_wrapper);
    lv_anim_set_duration(&animate_scale_line, animation_duration);
    lv_anim_set_repeat_count(&animate_scale_line, 0);
    lv_anim_set_playback_duration(&animate_scale_line, playback_duration);
    lv_anim_set_values(&animate_scale_line, start, end);
    lv_anim_start(&animate_scale_line);
}

/// @brief Wrapper for the callback function to set the needle line value
/// @param obj
/// @param v
void DemoComponent::set_needle_line_value_callback_wrapper(void *object, int32_t value)
{
    g_demo_component_instance->set_needle_line_value_callback(object, value);
}

/// @brief Callback function to set the needle line value
void DemoComponent::set_needle_line_value_callback(void *object, int32_t value)
{
    lv_scale_set_line_needle_value(this->_scale, this->_needle_line, 60, value);
}

/// @brief DemoComponent destructor to clean up dynamically allocated objects
DemoComponent::~DemoComponent()
{
    if (g_demo_component_instance)
        delete g_demo_component_instance;

    if (_needle_line)
        lv_obj_del(_needle_line);

    if (_scale)
        lv_obj_del(_scale);
}