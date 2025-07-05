#include "components/key_component.h"
#include <icons/lock-alt-solid.h>

KeyComponent::KeyComponent() : _key_icon(nullptr)
{

}

KeyComponent::~KeyComponent()
{
    // Clean up LVGL objects
    if (_key_icon) {
        lv_obj_del(_key_icon);
    }
}

/// @brief This method initializes the key present icon with location parameters
/// @param screen The screen object to render the component on.
/// @param location The location parameters for positioning the component.
void KeyComponent::render_load(lv_obj_t *screen, const ComponentLocation& location)
{
    log_d("...");

    // Create the key icon
    _key_icon = lv_img_create(screen);
    lv_img_set_src(_key_icon, &lock_alt_solid);
    
    // Apply location settings
    lv_obj_align(_key_icon, location.align, location.x_offset, location.y_offset);
    
    lv_obj_set_style_img_recolor(_key_icon, lv_color_hex(0xFFFFFF), 0);

    log_d("rendered load with location");
}