#pragma once // preventing duplicate definitions, alternative to the traditional include guards

// System/Library Includes
#include <esp32-hal-log.h>
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
    
    // Provider Access Methods
    /// @brief Get GPIO provider for hardware I/O operations
    /// @return Pointer to GPIO provider instance
    virtual IGpioProvider* getGpioProvider() = 0;
    
    /// @brief Get display provider for LVGL operations
    /// @return Pointer to display provider instance
    virtual IDisplayProvider* getDisplayProvider() = 0;
};