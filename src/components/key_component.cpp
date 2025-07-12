#include "components/key_component.h"
#include <icons/key-solid.h>

// Constructors and Destructors
KeyComponent::KeyComponent() : _key_icon(nullptr)
{
}

KeyComponent::~KeyComponent()
{
    // Clean up LVGL objects
    if (_key_icon)
    {
        lv_obj_del(_key_icon);
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
        colour = StyleManager::get_instance().get_colours(StyleManager::get_instance().get_theme()).key_present;
    }
    else if (key_state == KeyState::NotPresent)
    {
        colour = StyleManager::get_instance().get_colours(StyleManager::get_instance().get_theme()).key_not_present;
    }
    else // KeyState::Inactive
    {
        colour = StyleManager::get_instance().get_colours(StyleManager::get_instance().get_theme()).key_not_present;
    }

    lv_obj_set_style_image_recolor(_key_icon, colour, MAIN_DEFAULT);
    lv_obj_set_style_image_recolor_opa(_key_icon, LV_OPA_COVER, MAIN_DEFAULT);
    log_d("rendered update with colour: R=%d G=%d B=%d", colour.red, colour.green, colour.blue);
}

/// @brief This method initializes the key present icon with location parameters
/// @param screen The screen object to render the component on.
/// @param location The location parameters for positioning the component.
void KeyComponent::render(lv_obj_t *screen, const ComponentLocation &location)
{
    log_d("...");

    // Create the key icon
    _key_icon = lv_image_create(screen);
    lv_image_set_src(_key_icon, &key_solid);

    // Apply location settings
    lv_obj_align(_key_icon, location.align, location.x_offset, location.y_offset);
    log_d("rendered load with location");
}