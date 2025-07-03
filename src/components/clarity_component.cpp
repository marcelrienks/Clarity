#include "components/clarity_component.h"

/// @brief Initialises the Clarity Component by rendering a splash screen with location parameters
/// @param screen the screen on which to render the component
/// @param location the location parameters for positioning the component
void ClarityComponent::render_load(lv_obj_t *screen, const ComponentLocation& location) {
    log_d("...");
     
    // Using a label (recommended for text display)
    lv_obj_t *splash = lv_label_create(screen);
    lv_label_set_text(splash, "Clarity");
    
    // Set text color with full opacity
    lv_obj_set_style_text_color(splash, lv_color_white(), LV_OPA_COVER);
    
    // Apply location settings
    if (location.align != LV_ALIGN_CENTER || location.x_offset != 0 || location.y_offset != 0)
        lv_obj_align(splash, location.align, location.x_offset, location.y_offset);

    else if (location.x != 0 || location.y != 0)
        lv_obj_set_pos(splash, location.x, location.y);

    else
        lv_obj_align(splash, LV_ALIGN_CENTER, 0, 0);
    
    // Set font size
    lv_obj_set_style_text_font(splash, &lv_font_montserrat_20, 0);
}