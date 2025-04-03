#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include <esp32-hal-log.h>
#include <lvgl.h>
#include <LovyanGFX.hpp>

/// @brief Hardware abstraction
class IDevice : public lgfx::LGFX_Device
{
public:
    virtual void prepare() = 0;
};