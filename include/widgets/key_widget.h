#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_widget.h"
#include "utilities/types.h"
#include "managers/style_manager.h"

#include <lvgl.h>

/**
 * @class KeyWidget
 * @brief Key/ignition status indicator widget
 * 
 * @details This widget displays a key icon to indicate the ignition or
 * key status of the vehicle. It provides visual feedback for key-related
 * states and conditions.
 * 
 * @view_role Renders key status icon with theme-aware styling
 * @ui_elements Key icon with conditional styling based on status
 * @positioning Supports all WidgetLocation alignment options
 * 
 * @data_source Designed to work with KeySensor for status updates
 * @visual_states Normal, active, warning states with color changes
 * @icon_resource Uses lock-alt-solid icon from icon resources
 * 
 * @context This widget shows key/ignition status. It's part of
 * the broader vehicle monitoring system and could be used in various
 * panels for status indication.
 */
class KeyWidget : public IWidget
{
public:
    // Constructors and Destructors
    KeyWidget();
    virtual ~KeyWidget();

    // Core Functionality Methods
    void render(lv_obj_t *screen, const WidgetLocation& location) override;
    void refresh(const Reading& reading) override;

protected:
    // Protected Data Members
    lv_obj_t *_key_icon;

private:
    // Core Functionality Methods
    void create_icon();
};