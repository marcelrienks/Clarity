#include "widgets/clarity_widget.h"

// Core Functionality Methods

/// @brief Initialises the Clarity Widget by rendering a splash screen with location parameters
/// @param screen the screen on which to render the widget
/// @param location the location parameters for positioning the widget
void ClarityWidget::render(lv_obj_t *screen, const WidgetLocation& location) {
    log_d("...");
     
    // Using a label (recommended for text display)
    lv_obj_t *splash = lv_label_create(screen);
    lv_label_set_text(splash, "Clarity");
    
    // Set text color with full opacity
    lv_obj_set_style_text_color(splash, lv_color_white(), LV_OPA_COVER);
    
    // Apply location settings
    lv_obj_align(splash, location.align, location.x_offset, location.y_offset);
    
    // Set font size
    lv_obj_set_style_text_font(splash, &lv_font_montserrat_20, 0);
}