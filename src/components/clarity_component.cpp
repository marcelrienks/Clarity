#include "components/clarity_component.h"

/// @brief Initialises the Clarity Component by rendering a splash screen
/// @param screen the screen on which to render the component
void ClarityComponent::render_show(lv_obj_t *screen) {
    log_d("...");
     
    // Using a label (recommended for text display)
    lv_obj_t *splash = lv_label_create(screen);
    lv_label_set_text(splash, "Clarity");
    
    // Set text color with full opacity
    lv_obj_set_style_text_color(splash, lv_color_white(), LV_OPA_COVER);
    
    // Center align the text
    lv_obj_align(splash, LV_ALIGN_CENTER, 0, 0);
    
    // Set font size
    lv_obj_set_style_text_font(splash, &lv_font_montserrat_20, 0);
}