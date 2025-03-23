#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_panel.h"
#include "components/demo_component.h"
#include "sensors/demo_sensor.h"

class DemoPanel : public IPanel
{
public:
    DemoPanel(PanelIteration panel_iteration) : _component(std::make_shared<DemoComponent>()),
                                                _sensor(std::make_shared<DemoSensor>()),
                                                _iteration(panel_iteration) {};

    ~DemoPanel();

    std::string get_name() const { return _name; };
    PanelType get_type() const { return _type; };
    PanelIteration get_iteration() const { return _iteration; };
    void set_iteration(PanelIteration panel_iteration) { _iteration = panel_iteration; };

    void init(IDevice *device) override;
    void show(std::function<void()> show_panel_completion_callback) override;
    void update(std::function<void()> update_panel_completion_callback = nullptr) override;

private:
    // Panel specific constants
    static constexpr const char *_name = "Demo";
    static constexpr const PanelType _type = PanelType::Sensor;
    PanelIteration _iteration = PanelIteration::Infinite; // this must be modifiable to allow for personalization

    // Components
    IDevice *_device;
    lv_obj_t *_screen;
    std::shared_ptr<IComponent> _component;
    std::shared_ptr<ISensor> _sensor;

    static void show_panel_completion_callback(lv_event_t* event);
    static void update_panel_completion_callback(lv_event_t* event);
};