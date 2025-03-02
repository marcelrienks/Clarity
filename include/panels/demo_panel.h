#ifndef DEMO_PANEL_H
#define DEMO_PANEL_H

#include "interfaces/i_panel.h"
#include "components/demo_component.h"
#include "sensors/demo_sensor.h"
#include "utilities/serial_logger.h"

#include <lvgl.h>

class DemoPanel : public IPanel
{
private:
    IDevice *_device;
    lv_obj_t *_screen;
    IComponent *_component;
    ISensor *_sensor;
    std::function<void()> _callback_function;

public:
    DemoPanel();
    ~DemoPanel();

    void init(IDevice *device) override;
    void show() override;
    void update() override;
    void set_completion_callback(std::function<void()> callback_function) override;
};

#endif // DEMO_PANEL_H