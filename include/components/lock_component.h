#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_component.h"
#include "utilities/types.h"
#include "managers/style_manager.h"

#include <lvgl.h>

/**
 * @class LockComponent
 * @brief Lock status indicator component
 * 
 * @details This component displays a lock icon to indicate the lock status
 * of the vehicle. It provides visual feedback for lock-related states and
 * conditions using the lock-alt-solid icon.
 * 
 * @view_role Renders lock status icon with theme-aware styling
 * @ui_elements Lock icon with conditional styling based on status
 * @positioning Supports all ComponentLocation alignment options
 * 
 * @data_source Designed to work with LockSensor for status updates
 * @visual_states Normal, active, warning states with color changes
 * @icon_resource Uses lock-alt-solid icon from icon resources
 * 
 * @context This component shows lock status. It's part of
 * the broader vehicle monitoring system and could be used in various
 * panels for status indication.
 */
class LockComponent : public IComponent
{
public:
    // Constructors and Destructors
    LockComponent();
    virtual ~LockComponent();

    // Core Functionality Methods
    void refresh(const Reading& reading) override;
    void render(lv_obj_t *screen, const ComponentLocation& location, IDisplayProvider* display) override;

protected:
    // Protected Data Members
    lv_obj_t *lockIcon_;

private:
    // Core Functionality Methods
    void CreateIcon();
};