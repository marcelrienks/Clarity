#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_panel.h"
#include "components/demo_component.h"
#include "sensors/demo_sensor.h"

class DemoPanel : public IPanel
{
public:
    DemoPanel();
    ~DemoPanel();

    void init(IDevice *device) override;
    void show(std::function<void()> callback_function) override;
    void update() override;

    PanelType get_type() const {return PanelType::Sensor;};
    std::string get_name() const {return "DemoPanel";};
};