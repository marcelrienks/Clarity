#include "components/clarity_component.h"

/// @brief Initialises the Clarity Component by rendering a splash screen
/// @param screen the screen on which to render the component
void ClarityComponent::render_show(lv_obj_t *screen) {
    log_d("...");
     
    lv_obj_t *splash = lv_img_create(screen);
    lv_img_set_src(splash, LV_SYMBOL_DUMMY "Clarity");
    lv_obj_set_style_text_color(splash, lv_color_white(), LV_OPA_0);
    lv_obj_align(splash, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_img_recolor(splash, lv_color_white(), 0);
    lv_obj_set_style_img_recolor_opa(splash, LV_OPA_COVER, 0);
}