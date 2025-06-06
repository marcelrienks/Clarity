#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_panel.h"
#include "components/demo_component.h"
#include "sensors/demo_sensor.h"

class DemoPanel : public IPanel
{
public:
    DemoPanel();
    ~DemoPanel();

    const char *get_name() const { return PanelNames::Demo; };

    void init() override;
    void load(std::function<void()> callback_function) override;
    void update(std::function<void()> callback_function = nullptr) override;

private:
    // Components
    lv_obj_t *_screen;
    std::shared_ptr<IComponent> _component;
    std::shared_ptr<ISensor> _sensor;
    int32_t _current_value;

    static void show_panel_completion_callback(lv_event_t* event);
    static void update_panel_completion_callback(lv_anim_t *animation);
    static void execute_animation_callback(void *target, int32_t value);
};