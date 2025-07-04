#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_component.h"
#include "utilities/types.h"
#include "managers/style_manager.h"

#include <lvgl.h>

/**
 * @class KeyComponent
 * @brief Key/ignition status indicator component
 * 
 * @details This component displays a key icon to indicate the ignition or
 * key status of the vehicle. It provides visual feedback for key-related
 * states and conditions.
 * 
 * @view_role Renders key status icon with theme-aware styling
 * @ui_elements Key icon with conditional styling based on status
 * @positioning Supports all ComponentLocation alignment options
 * 
 * @data_source Designed to work with KeySensor for status updates
 * @visual_states Normal, active, warning states with color changes
 * @icon_resource Uses key-solid icon from icon resources
 * 
 * @context This component shows key/ignition status. It's part of
 * the broader vehicle monitoring system and could be used in various
 * panels for status indication.
 */
class KeyComponent : public IComponent
{
public:
    KeyComponent();
    virtual ~KeyComponent();

    void render_load(lv_obj_t *screen, const ComponentLocation& location) override;

protected:
    // LVGL objects
    lv_obj_t *_key_icon;

private:
    void create_icon();
};