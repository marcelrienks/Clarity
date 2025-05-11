#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_panel.h"
#include "components/oil_pressure_component.h"
#include "sensors/oil_pressure_sensor.h"

#include <utilities/lv_tools.h>

class OilPanel : public IPanel
{
public:
    OilPanel(IDevice *device);
    ~OilPanel();

    std::string get_name() const { return _name; };
    PanelType get_type() const { return _type; };

    void init() override;
    void show(std::function<void()> callback_function) override;
    void update(std::function<void()> callback_function = nullptr) override;

private:
    // Panel specific constants
    static constexpr const char *_name = "Oil";
    static constexpr const PanelType _type = PanelType::Sensor;

    // Components
    IDevice *_device;
    lv_obj_t *_screen;
    std::shared_ptr<IComponent> _component;
    std::shared_ptr<ISensor> _sensor;
    int32_t _current_value;

    static void show_panel_completion_callback(lv_event_t *event);
    static void update_panel_completion_callback(lv_anim_t *animation);
    static void execute_animation_callback(void *target, int32_t value);
};