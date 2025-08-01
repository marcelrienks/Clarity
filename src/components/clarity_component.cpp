#include "components/clarity_component.h"
#include <esp32-hal-log.h>

// Constructors and Destructors
ClarityComponent::ClarityComponent(IStyleService* styleService)
    : styleService_(styleService)
{
}

// Core Functionality Methods

/// @brief Initialises the Clarity Component by rendering a splash screen with location parameters
/// @param screen the screen on which to render the component
/// @param location the location parameters for positioning the component
void ClarityComponent::render(lv_obj_t *screen, const ComponentLocation& location, IDisplayProvider* display) {
    log_d("Rendering Clarity splash text component");
    
    // Using a label (recommended for text display)
    if (!display) {
        log_e("ClarityComponent requires display provider");
        return;
    }
    
    lv_obj_t *splash = display->createLabel(screen);
    lv_label_set_text(splash, UIConstants::APP_NAME);
    
    // Apply the current theme's text style
    lv_obj_add_style(splash, &styleService_->getTextStyle(), LV_PART_MAIN | LV_STATE_DEFAULT);
    
    // Apply location settings
    lv_obj_align(splash, location.align, location.x_offset, location.y_offset);
    
    // Set font size
    lv_obj_set_style_text_font(splash, &lv_font_montserrat_20, 0);
}