#include "components/key_component.h"
#include "managers/error_manager.h"
#include "utilities/logging.h"
#include <esp32-hal-log.h>
#include <icons/key_solid.h>

// Constructors and Destructors
KeyComponent::KeyComponent(IStyleService *styleService) : keyIcon_(nullptr), styleService_(styleService)
{
    log_v("KeyComponent constructor called");
}

KeyComponent::~KeyComponent()
{
    log_v("~KeyComponent() destructor called");
    
    if (keyIcon_)
    {
        lv_obj_del(keyIcon_);
    }
}

// Core Functionality Methods

/// @brief This method initializes the key present icon with location parameters
/// @param screen The screen object to render the component on.
/// @param location The location parameters for positioning the component.
void KeyComponent::Render(lv_obj_t *screen, const ComponentLocation &location, IDisplayProvider *display)
{
    log_v("Render() called");
    
    if (!display)
    {
        log_e("KeyComponent requires display provider");
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "KeyComponent",
                                             "Cannot render - display provider is null");
        return;
    }


    keyIcon_ = display->CreateImage(screen);
    lv_image_set_src(keyIcon_, &key_solid);

    // Apply location settings
    lv_obj_align(keyIcon_, location.align, location.x_offset, location.y_offset);
}

/// @brief Set the key icon color based on key state
/// @param keyState The current key state to determine color
void KeyComponent::SetColor(KeyState keyState)
{
    log_v("SetColor() called with state: %d", static_cast<int>(keyState));
    
    if (!styleService_ || !keyIcon_)
    {
        return;
    }
    
    lv_color_t colour;
    if (keyState == KeyState::Present)
    {
        colour = styleService_->GetThemeColors().keyPresent;
        log_t("Key icon color set to GREEN (key present)");
    }
    else // KeyState::NotPresent or KeyState::Inactive
    {
        colour = styleService_->GetThemeColors().keyNotPresent;
        log_t("Key icon color set to RED (key not present)");
    }

    lv_obj_set_style_image_recolor(keyIcon_, colour, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_image_recolor_opa(keyIcon_, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
}