#ifndef I_DEVICE_H
#define I_DEVICE_H

#include <lvgl.h>
#include <LovyanGFX.hpp>

class IDevice : public lgfx::LGFX_Device
{
public:
    volatile bool _is_splash_complete = false;

    virtual void prepare() = 0;
    virtual void display_flush_callback(lv_display_t *display, const lv_area_t *area, unsigned char *data) = 0;
};

#endif // I_DEVICE_H