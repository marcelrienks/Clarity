#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_widget.h"
#include "utilities/types.h"
#include "managers/style_manager.h"

#include <lvgl.h>

/**
 * @class LockWidget
 * @brief Lock status indicator widget
 * 
 * @details This widget displays a lock icon to indicate the lock status
 * of the vehicle. It provides visual feedback for lock-related states and
 * conditions using the lock-alt-solid icon.
 * 
 * @view_role Renders lock status icon with theme-aware styling
 * @ui_elements Lock icon with conditional styling based on status
 * @positioning Supports all WidgetLocation alignment options
 * 
 * @data_source Designed to work with LockSensor for status updates
 * @visual_states Normal, active, warning states with color changes
 * @icon_resource Uses lock-alt-solid icon from icon resources
 * 
 * @context This widget shows lock status. It's part of
 * the broader vehicle monitoring system and could be used in various
 * panels for status indication.
 */
class LockWidget : public IWidget
{
public:
    // Constructors and Destructors
    LockWidget();
    virtual ~LockWidget();

    // Core Functionality Methods
    void refresh(const Reading& reading) override;
    void render(lv_obj_t *screen, const WidgetLocation& location) override;

protected:
    // Protected Data Members
    lv_obj_t *_lock_icon;

private:
    // Core Functionality Methods
    void create_icon();
};