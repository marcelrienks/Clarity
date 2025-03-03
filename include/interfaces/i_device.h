#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include <lvgl.h>
#include <LovyanGFX.hpp>

//TODO: I don't think this is valuable, unless I create integration tests, there will never be multiple implementations of this

class IDevice : public lgfx::LGFX_Device
{
public:
    virtual void prepare() = 0;
    virtual void display_flush_callback(lv_display_t *display, const lv_area_t *area, unsigned char *data) = 0;
};