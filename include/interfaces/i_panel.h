#pragma once // preventing duplicate definitions, alternative to the traditional include guards

// System/Library Includes
#include <functional>
#include <lvgl.h>
#include <memory>

// Project Includes
#include "interfaces/i_sensor.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_display_provider.h"

class IPanel
{
public:
    // Destructors
    virtual ~IPanel() = default;

    // Core Interface Methods
    /// @brief Initialize the panel and its components
    /// @param gpio GPIO provider for hardware access
    /// @param display Display provider for UI operations
    virtual void init(IGpioProvider* gpio, IDisplayProvider* display) = 0;

    /// @brief Load the panel with asynchronous completion callback
    /// @param callbackFunction Function to call when loading is complete
    /// @param gpio GPIO provider for hardware access
    /// @param display Display provider for UI operations
    virtual void load(std::function<void()> callbackFunction, IGpioProvider* gpio, IDisplayProvider* display) = 0;

    /// @brief Update the panel data with asynchronous completion callback
    /// @param callbackFunction Function to call when update is complete
    /// @param gpio GPIO provider for hardware access
    /// @param display Display provider for UI operations
    virtual void update(std::function<void()> callbackFunction, IGpioProvider* gpio, IDisplayProvider* display) = 0;

    /// @brief Show the panel in the display
    virtual void show() {
        if (screen_) {
            display_->loadScreen(screen_);
        }
    }

protected:
    lv_obj_t* screen_ = nullptr;
    IDisplayProvider* display_ = nullptr;

protected:
    // Protected Data Members
    std::function<void()> callbackFunction_;
};