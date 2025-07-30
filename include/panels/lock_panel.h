#pragma once

#include "interfaces/i_panel.h"
#include "components/lock_component.h"
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
 * @presenter_role Coordinates LockComponent with LockSensor data
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
    static constexpr const char* NAME = PanelNames::LOCK;
    void init(IGpioProvider* gpio, IDisplayProvider* display) override;
    void load(std::function<void()> callbackFunction, IGpioProvider* gpio, IDisplayProvider* display) override;
    void update(std::function<void()> callbackFunction, IGpioProvider* gpio, IDisplayProvider* display) override;

private:
    // Static Methods
    static void ShowPanelCompletionCallback(lv_event_t *event);

    // Instance Data Members
    lv_obj_t *screen_; // All panels should always have their own screens
    std::shared_ptr<LockComponent> lockComponent_;
    std::shared_ptr<LockSensor> lockSensor_;
    ComponentLocation centerLocation_;
    bool isLockEngaged_ = false;
};