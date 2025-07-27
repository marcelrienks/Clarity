#pragma once

#include "interfaces/i_panel.h"
#include "components/key_component.h"
#include "utilities/types.h"

#include <utilities/lv_tools.h>

/**
 * @class KeyPanel
 * @brief Key status monitoring panel
 *
 * @details This panel displays the key/ignition status using a centered key icon.
 * It provides a simple, clean interface for monitoring key presence or ignition state.
 *
 * @presenter_role Coordinates KeyComponent with KeySensor data
 * @data_source KeySensor providing boolean key status
 * @update_strategy Simple boolean state updates without animation
 *
 * @ui_layout:
 * - Key Icon: Center of screen with key-solid icon
 * - Icon changes color based on key status (white=present, red=absent)
 *
 * @visual_feedback:
 * - Normal: White key icon when key is present/ignition on
 * - Alert: Red key icon when key is absent/ignition off
 * - Smooth color transitions for status changes
 *
 * @context This panel provides a dedicated view for key status monitoring.
 * It's designed to be simple and clear, focusing on the key status indication.
 */
class KeyPanel : public IPanel
{
public:
    // Constructors and Destructors
    KeyPanel();
    ~KeyPanel();

    // Core Functionality Methods
    static constexpr const char* NAME = PanelNames::KEY;
    void init() override;
    void load(std::function<void()> callbackFunction = nullptr) override;
    void update(std::function<void()> callbackFunction = nullptr) override;

private:
    // Static Methods
    static void ShowPanelCompletionCallback(lv_event_t *event);

    // Instance Data Members
    lv_obj_t *screen_; // All panels should always have their own screens
    std::shared_ptr<KeyComponent> keyComponent_;
    ComponentLocation centerLocation_;
    KeyState currentKeyState_ = KeyState::Inactive;
};