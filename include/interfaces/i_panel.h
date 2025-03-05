#pragma once // preventing duplicate definitions, alternative to the traditional include guards

#include "interfaces/i_device.h"
#include "interfaces/i_component.h"
#include "interfaces/i_sensor.h"

#include <lvgl.h>
#include <functional>
#include <memory>

class IPanel
{
protected:
    IDevice *_device;
    lv_obj_t *_screen;
    std::shared_ptr<IComponent> _component;
    std::shared_ptr<ISensor> _sensor;
    std::function<void()> _callback_function;

public:
    /**
     * @brief Initialize the panel with the device
     * @param device The device to initialize with
     */
    virtual void init(IDevice *device) = 0;
    
    /**
     * @brief Show the panel, applying any transitions or animations
     */
    virtual void show() = 0;
    
    /**
     * @brief Update the panel state
     * This should be called regularly from the main loop
     */
    virtual void update() = 0;
    
    /**
     * @brief Set a callback to be executed when the panel animation completes
     * @param callback_function The function to call when animation is complete
     */
    virtual void set_completion_callback(std::function<void()> callback_function) = 0;
    
    /**
     * @brief Virtual destructor
     */
    virtual ~IPanel() = default;
};