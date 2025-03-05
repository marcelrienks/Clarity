#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_panel.h"
#include "components/demo_component.h"
#include "sensors/demo_sensor.h"
#include "utilities/serial_logger.h"

#include <lvgl.h>
#include <memory>

class DemoPanel : public IPanel
{
public:
    DemoPanel();
    ~DemoPanel() override;

    void init(IDevice *device) override;
    void show() override;
    void update() override;
    void set_completion_callback(std::function<void()> callback_function) override;
};