#pragma once // preventing duplicate definitions, alternative to the traditional include guards

// System/Library Includes
#include <esp32-hal-log.h>
#include <functional>
#include <lvgl.h>
#include <memory>

// Project Includes
#include "interfaces/i_device.h"
#include "interfaces/i_sensor.h"

class IPanel
{
public:
    // Destructors
    virtual ~IPanel() = default;

    // Core Interface Methods
    /// @brief Get the panel's name identifier
    /// @return String name of the panel
    virtual const char *get_name() const = 0;

    /// @brief Initialize the panel and its components
    virtual void init() = 0;

    /// @brief Load the panel with asynchronous completion callback
    /// @param callback_function Function to call when loading is complete
    virtual void load(std::function<void()> callback_function) = 0;

    /// @brief Update the panel data with asynchronous completion callback
    /// @param callback_function Function to call when update is complete
    virtual void update(std::function<void()> callback_function) = 0;

protected:
    // Protected Data Members
    std::function<void()> _callback_function;
};