#ifndef I_PANEL_H
#define I_PANEL_H

#include "interfaces/i_device.h"
#include "interfaces/i_component.h"
#include "interfaces/i_sensor.h"

#include <lvgl.h>

// Define a panel completion callback type
using PanelCompletionCallback = std::function<void()>;

class IPanel
{
private:
    IDevice *_device;
    lv_obj_t *_screen;

public:
    virtual void init(IDevice *device) = 0;
    virtual void show() = 0;
    virtual void update() = 0;
    virtual void set_completion_callback(PanelCompletionCallback callback) = 0;
};

#endif // I_PANEL_H
