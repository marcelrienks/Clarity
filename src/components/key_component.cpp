#include "components/key_component.h"
#include <icons/key-solid.h>

// Constructors and Destructors
KeyComponent::KeyComponent() : keyIcon_(nullptr)
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
    log_d("...");
    
    KeyState key_state = static_cast<KeyState>(std::get<int32_t>(reading));
    lv_color_t colour;
    
    if (key_state == KeyState::Present)
    {
        colour = StyleManager::GetInstance().get_colours(StyleManager::GetInstance().THEME).keyPresent;
    }
    else // KeyState::NotPresent or KeyState::Inactive
    {
        colour = StyleManager::GetInstance().get_colours(StyleManager::GetInstance().THEME).keyNotPresent;
    }

    lv_obj_set_style_image_recolor(keyIcon_, colour, MAIN_DEFAULT);
    lv_obj_set_style_image_recolor_opa(keyIcon_, LV_OPA_COVER, MAIN_DEFAULT);
    log_d("rendered update with colour: R=%d G=%d B=%d", colour.red, colour.green, colour.blue);
}

/// @brief This method initializes the key present icon with location parameters
/// @param screen The screen object to render the component on.
/// @param location The location parameters for positioning the component.
void KeyComponent::render(lv_obj_t *screen, const ComponentLocation &location)
{
    log_d("...");

    // Create the key icon
    keyIcon_ = lv_image_create(screen);
    lv_image_set_src(keyIcon_, &key_solid);

    // Apply location settings
    lv_obj_align(keyIcon_, location.align, location.x_offset, location.y_offset);
    log_d("rendered load with location");
}