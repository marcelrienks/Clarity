#include "widgets/lock_widget.h"
#include <icons/lock-alt-solid.h>

// Constructors and Destructors
LockWidget::LockWidget() : _lock_icon(nullptr)
{
}

LockWidget::~LockWidget()
{
    // Clean up LVGL objects
    if (_lock_icon)
    {
        lv_obj_del(_lock_icon);
    }
}

// Core Functionality Methods
void LockWidget::refresh(const Reading& reading)
{
    log_d("...");
    
    bool is_lock_engaged = std::get<bool>(reading);
    lv_color_t colour = StyleManager::get_instance().get_colours(StyleManager::get_instance().get_theme()).key_present;
    if (!is_lock_engaged)
    {
        colour = StyleManager::get_instance().get_colours(StyleManager::get_instance().get_theme()).key_not_present;
    }

    lv_obj_set_style_image_recolor(_lock_icon, colour, MAIN_DEFAULT);
    lv_obj_set_style_image_recolor_opa(_lock_icon, LV_OPA_COVER, MAIN_DEFAULT);
    log_d("rendered update with colour: R=%d G=%d B=%d", colour.red, colour.green, colour.blue);
}

/// @brief This method initializes the lock status icon with location parameters
/// @param screen The screen object to render the widget on.
/// @param location The location parameters for positioning the widget.
void LockWidget::render(lv_obj_t *screen, const WidgetLocation &location)
{
    log_d("...");

    // Create the lock icon
    _lock_icon = lv_image_create(screen);
    lv_image_set_src(_lock_icon, &lock_alt_solid);

    // Apply location settings
    lv_obj_align(_lock_icon, location.align, location.x_offset, location.y_offset);
    log_d("rendered load with location");
}