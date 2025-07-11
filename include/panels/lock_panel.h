#pragma once

#include "interfaces/i_panel.h"
#include "widgets/lock_widget.h"
#include "sensors/lock_sensor.h"
#include "utilities/types.h"

#include <utilities/lv_tools.h>

/**
 * @class LockPanel
 * @brief Lock status monitoring panel
 *
 * @details This panel displays the lock status using a centered lock icon.
 * It provides a simple, clean interface for monitoring lock state.
 *
 * @presenter_role Coordinates LockWidget with LockSensor data
 * @data_source LockSensor providing boolean lock status
 * @update_strategy Simple boolean state updates without animation
 *
 * @ui_layout:
 * - Lock Icon: Center of screen with lock-alt-solid icon
 * - Icon changes color based on lock status (white=engaged, red=disengaged)
 *
 * @visual_feedback:
 * - Normal: White lock icon when lock is engaged
 * - Alert: Red lock icon when lock is disengaged
 * - Smooth color transitions for status changes
 *
 * @context This panel provides a dedicated view for lock status monitoring.
 * It's designed to be simple and clear, focusing on the lock status indication.
 */
class LockPanel : public IPanel
{
public:
    // Constructors and Destructors
    LockPanel();
    ~LockPanel();

    // Core Functionality Methods
    const char *get_name() const { return PanelNames::Lock; };
    void init() override;
    void load(std::function<void()> callback_function = nullptr) override;
    void update(std::function<void()> callback_function = nullptr) override;

private:
    // Static Methods
    static void show_panel_completion_callback(lv_event_t *event);

    // Instance Data Members
    lv_obj_t *_screen; // All panels should always have their own screens
    std::shared_ptr<LockWidget> _lock_widget;
    std::shared_ptr<LockSensor> _lock_sensor;
    WidgetLocation _center_location;
    bool _is_lock_engaged = false;
};