#include "components/oil_temperature_component.h"

OilTemperatureComponent::OilTemperatureComponent() {}

OilTemperatureComponent::~OilTemperatureComponent()
{
    if (_scale)
        lv_obj_delete(_scale);
}

void OilTemperatureComponent::render_load(lv_obj_t *screen)
{
    log_d("...");

    const ThemeColors &colours = StyleManager::get_instance().get_colours(StyleManager::get_instance().get_theme());

    // Create the scale with NORMAL (non-reversed) range
    _scale = lv_scale_create(screen);
    lv_obj_set_size(_scale, 240, 240);
    lv_obj_align(_scale, LV_ALIGN_BOTTOM_MID, 0, 0);

    // Set scale properties - USE NORMAL RANGE, NOT REVERSED
    lv_scale_set_mode(_scale, LV_SCALE_MODE_ROUND_INNER);
    lv_scale_set_rotation(_scale, 30);     // starting angle (0 = 3 o'clock)
    lv_scale_set_angle_range(_scale, 120); // range in degrees for the span of the scale
    lv_scale_set_range(_scale, 0, 120);    // NORMAL range - we'll handle reverse mapping ourselves

    // Adjust tick counts
    lv_scale_set_total_tick_count(_scale, 13);
    lv_scale_set_major_tick_every(_scale, 2);
    lv_scale_set_label_show(_scale, true);

    // Set custom labels to show reversed temperature values
    // With 13 total ticks and major ticks every 2, we get labels at: 0, 20, 40, 60, 80, 100, 120
    // But we want to display them reversed: 120, 100, 80, 60, 40, 20, 0
    static const char *custom_labels[] = {"120°F", "100°F", "80°F", "60°F", "40°F", "20°F", "0°F", NULL};
    lv_scale_set_text_src(_scale, custom_labels);

    // Main style
    lv_style_init(&_main_part_style);
    lv_style_set_arc_width(&_main_part_style, 0);
    lv_obj_add_style(_scale, &_main_part_style, LV_PART_MAIN);

    // Indicator (Major ticks)
    lv_style_init(&_indicator_part_style);
    lv_style_set_length(&_indicator_part_style, 25);
    lv_style_set_line_width(&_indicator_part_style, 7);
    lv_style_set_line_color(&_indicator_part_style, colours.gauge_normal);
    lv_obj_add_style(_scale, &_indicator_part_style, LV_PART_INDICATOR);

    // Items (Minor ticks)
    lv_style_init(&_items_part_style);
    lv_style_set_length(&_items_part_style, 18);
    lv_style_set_line_width(&_items_part_style, 2);
    lv_style_set_line_color(&_items_part_style, colours.gauge_normal);
    lv_obj_add_style(_scale, &_items_part_style, LV_PART_ITEMS);

    // Danger Zone - original danger zone is 100-120°F, which maps to 0-20 in our normal scale
    lv_scale_section_t *section = lv_scale_add_section(_scale);
    lv_style_init(&_danger_section_items_part_style);
    lv_style_set_line_width(&_danger_section_items_part_style, 5);
    lv_style_set_line_color(&_danger_section_items_part_style, colours.gauge_danger);
    lv_scale_section_set_style(section, LV_PART_MAIN, &_main_part_style);
    lv_scale_section_set_style(section, LV_PART_INDICATOR, &_danger_section_items_part_style);
    lv_scale_section_set_style(section, LV_PART_ITEMS, &_danger_section_items_part_style);
    
    // Danger zone: original 100-120°F maps to 0-20 in our 0-120 scale
    lv_scale_section_set_range(section, 0, 20);

    // Add needle line
    _needle_line = lv_line_create(_scale);
    lv_obj_set_style_line_color(_needle_line, colours.gauge_normal, LV_PART_MAIN);
    lv_obj_set_style_line_width(_needle_line, 5, LV_PART_MAIN);
    lv_obj_set_style_line_rounded(_needle_line, false, LV_PART_MAIN);
    lv_obj_set_style_line_opa(_needle_line, LV_OPA_COVER, LV_PART_MAIN);

    // Circle at pivot point
    lv_obj_t *pivot_circle = lv_obj_create(_scale);
    lv_obj_set_size(pivot_circle, 35, 35);
    lv_obj_center(pivot_circle);
    lv_obj_set_style_radius(pivot_circle, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(pivot_circle, lv_color_darken(colours.background, 25), LV_PART_MAIN);
    lv_obj_set_style_border_width(pivot_circle, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(pivot_circle, lv_color_lighten(colours.background, 15), LV_PART_MAIN);

    // Create and position the oil can icon
    _oil_can_icon = lv_image_create(_scale);
    lv_image_set_src(_oil_can_icon, &oil_temp_regular);
    lv_image_set_scale(_oil_can_icon, 50);
    lv_obj_align(_oil_can_icon, LV_ALIGN_CENTER, 0, 50);
    lv_obj_set_style_opa(_oil_can_icon, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_image_recolor(_oil_can_icon, colours.gauge_normal, LV_PART_MAIN);
    lv_obj_set_style_image_recolor_opa(_oil_can_icon, LV_OPA_COVER, LV_PART_MAIN);

    log_d("rendered load");
}

void OilTemperatureComponent::render_update(lv_anim_t *animation, int32_t start, int32_t end)
{
    log_d("...");

    const ThemeColors &colours = StyleManager::get_instance().get_colours(StyleManager::get_instance().get_theme());
    lv_color_t colour = colours.gauge_normal;

    // Change color based on ORIGINAL value (not mapped)
    if (end >= _danger_zone)
        colour = colours.gauge_danger;

    lv_obj_set_style_line_color(_needle_line, colour, LV_PART_MAIN);
    lv_obj_set_style_image_recolor(_oil_can_icon, colour, LV_PART_MAIN);
    lv_obj_set_style_image_recolor_opa(_oil_can_icon, LV_OPA_COVER, LV_PART_MAIN);

    lv_anim_init(animation);
    lv_anim_set_duration(animation, _animation_duration);
    lv_anim_set_repeat_count(animation, 0);
    lv_anim_set_playback_duration(animation, 0);

    // Map the values to the normal scale (0-120 display as 120-0)
    int32_t mapped_start = map_reverse_value(start);
    int32_t mapped_end = map_reverse_value(end);

    log_d("original start is %i", start);
    log_d("original end is %i", end);
    log_d("mapped start is %i", mapped_start);
    log_d("mapped end is %i", mapped_end);
    
    lv_anim_set_values(animation, mapped_start, mapped_end);

    log_d("rendered update");
}

void OilTemperatureComponent::set_value(int32_t value)
{
    log_d("...");

    // Map the value for the normal scale
    int32_t mapped_value = map_reverse_value(value);

    log_d("original value is %i", value);
    log_d("mapped value is %i", mapped_value);
    
    // Use the mapped value directly with the normal scale
    lv_scale_set_line_needle_value(_scale, _needle_line, _needle_length, mapped_value);
}

int32_t OilTemperatureComponent::map_reverse_value(int32_t value) const
{
    log_d("...");

    // Clamp input value to valid range [0, 120]
    if (value > _scale_max_original) value = _scale_max_original;
    if (value < _scale_min_original) value = _scale_min_original;
    
    // Map from [120,0] to [0,120] - reverse the scale
    int32_t mapped_value = _scale_max_original - value;
    
    log_d("original value is %i", value);
    log_d("mapped value is %i", mapped_value);
    
    return mapped_value;
}