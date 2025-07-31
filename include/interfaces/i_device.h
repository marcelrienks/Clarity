#pragma once // preventing duplicate definitions, alternative to the traditional include guards

// System/Library Includes
#include <LovyanGFX.hpp>
#include <lvgl.h>

// Forward declarations
class IGpioProvider;
class IDisplayProvider;

/// @brief Hardware abstraction
class IDevice : public lgfx::LGFX_Device
{
public:
    // Core Interface Methods
    virtual void prepare() = 0;
};