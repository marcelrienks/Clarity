#pragma once // preventing duplicate definitions, alternative to the traditional include guards

// System/Library Includes
#include <esp32-hal-log.h>
#include <LovyanGFX.hpp>
#include <lvgl.h>

/// @brief Hardware abstraction
class IDevice : public lgfx::LGFX_Device
{
public:
    // Core Interface Methods
    virtual void prepare() = 0;
};