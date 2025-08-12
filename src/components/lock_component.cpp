#include "components/lock_component.h"
#include "managers/error_manager.h"
#include <icons/lock_alt_solid.h>
#include <esp32-hal-log.h>

// Constructors and Destructors
LockComponent::LockComponent(IStyleService* styleService) 
    : lockIcon_(nullptr), styleService_(styleService)
{
}

LockComponent::~LockComponent()
{
    if (lockIcon_)
    {
        lv_obj_del(lockIcon_);
    }
}

// Core Functionality Methods
void LockComponent::Refresh(const Reading& reading)
{
    log_d("Refreshing lock component display with new state");
    
    bool is_lock_engaged = std::get<bool>(reading);
    lv_color_t colour = styleService_->GetThemeColors().keyPresent;
    if (!is_lock_engaged)
    {
        colour = styleService_->GetThemeColors().keyNotPresent;
    }

    lv_obj_set_style_image_recolor(lockIcon_, colour, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_image_recolor_opa(lockIcon_, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
}

/// @brief This method initializes the lock status icon with location parameters
/// @param screen The screen object to render the component on.
/// @param location The location parameters for positioning the component.
void LockComponent::Render(lv_obj_t *screen, const ComponentLocation &location, IDisplayProvider* display)
{
    log_d("Rendering lock component icon at specified location");

    if (!display) {
        log_e("LockComponent requires display provider");
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "LockComponent", 
            "Cannot render - display provider is null");
        return;
    }
    
    lockIcon_ = display->CreateImage(screen);
    lv_image_set_src(lockIcon_, &lock_alt_solid);

    // Apply location settings
    lv_obj_align(lockIcon_, location.align, location.x_offset, location.y_offset);
}