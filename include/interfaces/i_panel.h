#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_device.h"
#include "interfaces/i_component.h"
#include "interfaces/i_sensor.h"

#include <esp32-hal-log.h>
#include <lvgl.h>
#include <functional>
#include <memory>

class IPanel
{
public:
    // Destructor
    virtual ~IPanel() = default;

    // Pure Virtual Methods
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
    virtual void update(std::function<void()> callback_function) = 0; //TODO: rename this to refresh

protected:
    // Instance Data Members
    std::function<void()> _callback_function;
};