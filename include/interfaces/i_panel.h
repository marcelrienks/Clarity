#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_device.h"
#include "interfaces/i_component.h"
#include "interfaces/i_sensor.h"

#include <esp32-hal-log.h>
#include <lvgl.h>
#include <functional>
#include <memory>

class IPanel
{
public:
    virtual std::string get_name() const = 0;

    virtual void init() = 0;
    virtual void show(std::function<void()> callback_function) = 0;
    virtual void update(std::function<void()> callback_function) = 0;

    std::function<void()> _callback_function;
};