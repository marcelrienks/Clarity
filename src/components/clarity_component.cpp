#include "components/clarity_component.h"

void ClarityComponent::init(lv_obj_t *screen) {
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
  
    lv_obj_t *icon_three = lv_img_create(screen);
    lv_img_set_src(icon_three, LV_SYMBOL_DUMMY "Clarity");
    lv_obj_set_style_text_color(icon_three, lv_color_white(), LV_OPA_0);
    lv_obj_align(icon_three, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_img_recolor(icon_three, lv_color_white(), 0);
    lv_obj_set_style_img_recolor_opa(icon_three, LV_OPA_COVER, 0);
}

void ClarityComponent::update(Reading value)
{
    // Not needed but required to satisfy interface
}