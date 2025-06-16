#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_panel.h"
#include "components/oem_oil_pressure_component.h"
#include "components/oem_oil_temperature_component.h"
#include "sensors/oil_pressure_sensor.h"
#include "sensors/oil_temperature_sensor.h"
#include "utilities/types.h"

#include <utilities/lv_tools.h>

class OilPanel : public IPanel
{
public:
    OilPanel();
    ~OilPanel();

    const char *get_name() const { return PanelNames::Oil; };

    void init() override;
    void load(std::function<void()> callback_function) override;
    void update(std::function<void()> callback_function = nullptr) override;

private:
    // Components
    lv_obj_t *_screen;
    std::shared_ptr<IComponent> _oem_oil_pressure_component;
    std::shared_ptr<IComponent> _oil_temperature_component;
    std::shared_ptr<ISensor> _oil_pressure_sensor;
    std::shared_ptr<ISensor> _oil_temperature_sensor;
    int32_t _current_oil_pressure_value;
    int32_t _current_oil_temperature_value;
    bool _is_pressure_animation_running = false;
    bool _is_temperature_animation_running = false;

    void update_oil_pressure();
    void update_oil_temperature();
    static void show_panel_completion_callback(lv_event_t *event);
    static void update_panel_completion_callback(lv_anim_t *animation);
    static void execute_pressure_animation_callback(void *target, int32_t value);
    static void execute_temperature_animation_callback(void *target, int32_t value);
};