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

    // Set scale properties
    lv_scale_set_mode(_scale, LV_SCALE_MODE_ROUND_INNER);
    lv_scale_set_rotation(_scale, 30);     // starting angle (0 = 3 o'clock)
    lv_scale_set_angle_range(_scale, 120); // range in degrees for the span of the scale
    lv_scale_set_range(_scale, _scale_min, _scale_max);

    // Adjust tick counts
    lv_scale_set_total_tick_count(_scale, 13);
    lv_scale_set_major_tick_every(_scale, 2);
    lv_scale_set_label_show(_scale, false);

    // Set custom labels to show reversed temperature values
    // With 13 total ticks and major ticks every 2, we get labels at: 0, 20, 40, 60, 80, 100, 120
    // But we want to display them reversed: 120, 100, 80, 60, 40, 20, 0
    // static const char *custom_labels[] = {"120", "100", "80", "60", "40", "20", "0", NULL};
    // lv_scale_set_text_src(_scale, custom_labels);

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

    // Danger Zone
    lv_scale_section_t *section = lv_scale_add_section(_scale);
    lv_style_init(&_danger_section_items_part_style);
    lv_style_set_line_width(&_danger_section_items_part_style, 5);
    lv_style_set_line_color(&_danger_section_items_part_style, colours.gauge_danger);
    lv_scale_section_set_style(section, LV_PART_MAIN, &_main_part_style);
    lv_scale_section_set_style(section, LV_PART_INDICATOR, &_danger_section_items_part_style);
    lv_scale_section_set_style(section, LV_PART_ITEMS, &_danger_section_items_part_style);

    // Danger zone: map correct danger zone to reversed danger zone (hack to solve reversed scale in LVGL 9.3)
    lv_scale_section_set_range(section, map_reverse_value(_scale_max), map_reverse_value(_danger_zone));

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

    if (end >= _danger_zone)
        colour = colours.gauge_danger;

    lv_obj_set_style_line_color(_needle_line, colour, LV_PART_MAIN);
    lv_obj_set_style_image_recolor(_oil_can_icon, colour, LV_PART_MAIN);
    lv_obj_set_style_image_recolor_opa(_oil_can_icon, LV_OPA_COVER, LV_PART_MAIN);

    lv_anim_init(animation);
    lv_anim_set_duration(animation, _animation_duration);
    lv_anim_set_repeat_count(animation, 0);
    lv_anim_set_playback_duration(animation, 0);

    // map correct danger zone to reversed danger zone (hack to solve reversed scale in LVGL 9.3)
    lv_anim_set_values(animation, start, end);

    log_d("rendered update");
}

void OilTemperatureComponent::set_value(int32_t value)
{
    log_d("...");

    // map correct value to reversed value (hack to solve reversed scale animation issue in LVGL 9.3)
    auto mapped_value = map_reverse_value(value);
    log_d("original value is %i, the mapped value is %i", value, mapped_value);

    lv_scale_set_line_needle_value(_scale, _needle_line, _needle_length, mapped_value);
}

/// Maps the value from the original scale (0-120) to the normal scale (120-0).
/// LVGL 9.3 has a bug that does allow setting a reverse scale 120-0, but it cannot animate the needle using that.
/// This method is used to map the value from the original scale (0-120) to the normal scale (120-0).
int32_t OilTemperatureComponent::map_reverse_value(int32_t value) const
{
    // Map from [0,120] to [120,0] reverse the scale
    int32_t mapped_value = _scale_max - value;
    return mapped_value;
}