#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_component.h"
#include "interfaces/i_style_service.h"
#include "utilities/types.h"

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
 * @icon_resource Uses lock-alt-solid icon from icon resources
 * 
 * @context This component shows key/ignition status. It's part of
 * the broader vehicle monitoring system and could be used in various
 * panels for status indication.
 */
class KeyComponent : public IComponent
{
public:
    // Constructors and Destructors
    explicit KeyComponent(IStyleService *styleService);
    virtual ~KeyComponent();

    // Core Functionality Methods
    void Render(lv_obj_t *screen, const ComponentLocation& location, IDisplayProvider *display) override;
    void Refresh(const Reading& reading) override;

protected:
    // Protected Data Members
    lv_obj_t *keyIcon_;
    IStyleService *styleService_;

private:
    // Core Functionality Methods
    void create_icon();
};