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
    DemoComponent *_component;
    DemoSensor *_sensor;

public:
    DemoPanel();
    ~DemoPanel();

    void init(IDevice *device);
    void show();
    void update();
};

#endif // DEMO_PANEL_H