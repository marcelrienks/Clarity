#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include <lvgl.h>
#include <LovyanGFX.hpp>

class IDevice : public lgfx::LGFX_Device
{
private:
    lgfx::Panel_GC9A01 _panel_instance;
    lgfx::Light_PWM _light_instance;
    lgfx::Bus_SPI _bus_instance;

    virtual void display_flush_callback(lv_display_t *display, const lv_area_t *area, unsigned char *data) = 0;

public:
    lv_obj_t *screen;

    virtual void prepare() = 0;
};