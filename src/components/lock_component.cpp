#include "components/lock_component.h"
#include <icons/lock_alt_solid.h>

// Constructors and Destructors
LockComponent::LockComponent() : lockIcon_(nullptr)
{
}

LockComponent::~LockComponent()
{
    // Clean up LVGL objects
    if (lockIcon_)
    {
        lv_obj_del(lockIcon_);
    }
}

// Core Functionality Methods
void LockComponent::refresh(const Reading& reading)
{
    log_d("Refreshing lock component display with new state");
    
    bool is_lock_engaged = std::get<bool>(reading);
    lv_color_t colour = StyleManager::GetInstance().get_colours(StyleManager::GetInstance().THEME).keyPresent;
    if (!is_lock_engaged)
    {
        colour = StyleManager::GetInstance().get_colours(StyleManager::GetInstance().THEME).keyNotPresent;
    }

    lv_obj_set_style_image_recolor(lockIcon_, colour, MAIN_DEFAULT);
    lv_obj_set_style_image_recolor_opa(lockIcon_, LV_OPA_COVER, MAIN_DEFAULT);
}

/// @brief This method initializes the lock status icon with location parameters
/// @param screen The screen object to render the component on.
/// @param location The location parameters for positioning the component.
void LockComponent::render(lv_obj_t *screen, const ComponentLocation &location)
{
    log_d("Rendering lock component icon at specified location");

    // Create the lock icon
    lockIcon_ = lv_image_create(screen);
    lv_image_set_src(lockIcon_, &lock_alt_solid);

    // Apply location settings
    lv_obj_align(lockIcon_, location.align, location.x_offset, location.y_offset);
}