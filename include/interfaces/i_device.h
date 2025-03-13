#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include <lvgl.h>
#include <LovyanGFX.hpp>

/// @brief Hardware abstraction
class IDevice : public lgfx::LGFX_Device
{
public:
    lv_obj_t *screen;

    virtual void prepare() = 0;

private:
    lgfx::Panel_GC9A01 _panel_instance;
    lgfx::Light_PWM _light_instance;
    lgfx::Bus_SPI _bus_instance;
};