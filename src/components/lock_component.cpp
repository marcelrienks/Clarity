#include "components/lock_component.h"
#include "managers/error_manager.h"
#include <esp32-hal-log.h>
#include <icons/lock_alt_solid.h>

// Constructors and Destructors
LockComponent::LockComponent(IStyleService *styleService) : lockIcon_(nullptr), styleService_(styleService)
{
    log_v("LockComponent constructor called");
}

LockComponent::~LockComponent()
{
    log_v("~LockComponent() destructor called");
    
    if (lockIcon_)
    {
        lv_obj_del(lockIcon_);
    }
}

// Core Functionality Methods

/// @brief This method initializes the lock status icon with location parameters
/// @param screen The screen object to render the component on.
/// @param location The location parameters for positioning the component.
void LockComponent::Render(lv_obj_t *screen, const ComponentLocation &location, IDisplayProvider *display)
{
    log_v("Render() called");
    
    if (!display)
    {
        log_e("LockComponent requires display provider");
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "LockComponent",
                                             "Cannot render - display provider is null");
        return;
    }


    lockIcon_ = display->CreateImage(screen);
    lv_image_set_src(lockIcon_, &lock_alt_solid);

    // Apply location settings
    lv_obj_align(lockIcon_, location.align, location.x_offset, location.y_offset);
    
    // Always show red (engaged) color for lock panel
    if (styleService_)
    {
        lv_color_t lock_colour = styleService_->GetThemeColors().lockEngaged;
        lv_obj_set_style_image_recolor(lockIcon_, lock_colour, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_image_recolor_opa(lockIcon_, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
}