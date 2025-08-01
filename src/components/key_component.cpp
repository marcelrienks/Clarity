#include "components/key_component.h"
#include <icons/key_solid.h>
#include <esp32-hal-log.h>

// Constructors and Destructors
KeyComponent::KeyComponent(IStyleService* styleService) 
    : keyIcon_(nullptr), styleService_(styleService)
{
}

KeyComponent::~KeyComponent()
{
    // Clean up LVGL objects
    if (keyIcon_)
    {
        lv_obj_del(keyIcon_);
    }
}

// Core Functionality Methods
void KeyComponent::refresh(const Reading& reading)
{
    log_d("Refreshing key component display with new state");
    
    KeyState key_state = static_cast<KeyState>(std::get<int32_t>(reading));
    lv_color_t colour;
    
    if (key_state == KeyState::Present)
    {
        colour = styleService_->getThemeColors().keyPresent;
    }
    else // KeyState::NotPresent or KeyState::Inactive
    {
        colour = styleService_->getThemeColors().keyNotPresent;
    }

    lv_obj_set_style_image_recolor(keyIcon_, colour, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_image_recolor_opa(keyIcon_, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
}

/// @brief This method initializes the key present icon with location parameters
/// @param screen The screen object to render the component on.
/// @param location The location parameters for positioning the component.
void KeyComponent::render(lv_obj_t *screen, const ComponentLocation &location, IDisplayProvider* display)
{
    log_d("Rendering key component icon at specified location");

    // Create the key icon
    // Note: LVGL doesn't have image creation in IDisplayProvider yet, keeping direct call
    keyIcon_ = lv_image_create(screen);
    lv_image_set_src(keyIcon_, &key_solid);

    // Apply location settings
    lv_obj_align(keyIcon_, location.align, location.x_offset, location.y_offset);
}