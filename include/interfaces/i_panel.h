#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_device.h"
#include "interfaces/i_component.h"
#include "interfaces/i_sensor.h"
#include "utilities/serial_logger.h"
#include "utilities/lv_tools.h"

#include <lvgl.h>
#include <functional>
#include <memory>
#include <typeinfo>

enum class PanelType
{
    Splash,
    Sensor,
    Config
};

class IPanel
{
public:
    virtual void init(IDevice *device) = 0;
    virtual void show(std::function<void()> callback_function) = 0;
    virtual void update() = 0;

    virtual PanelType get_type() const = 0;
    virtual std::string get_name() const = 0;

protected:
    IDevice *_device;
    lv_obj_t *_screen;
    std::shared_ptr<IComponent> _component;
    std::shared_ptr<ISensor> _sensor;
    std::function<void()> _callback_function;
};