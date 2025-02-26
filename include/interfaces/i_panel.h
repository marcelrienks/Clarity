#ifndef I_PANEL_H
#define I_PANEL_H

#include "interfaces/i_device.h"
#include "interfaces/i_component.h"
#include "interfaces/i_sensor.h"

#include <lvgl.h>

class IPanel
{
private:
    IDevice *_device;
    lv_obj_t *_screen;

public:
    virtual void init(IDevice *device) = 0;
    virtual void show() = 0;
    virtual void update() = 0;
};

#endif // I_PANEL_H
