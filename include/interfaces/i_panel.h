#pragma once // preventing duplicate definitions, alternative to the traditional include guards

// System/Library Includes
#include <functional>
#include <lvgl.h>
#include <memory>

// Project Includes
#include "interfaces/i_sensor.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_display_provider.h"

// Forward declaration
class IInputService;

/**
 * @interface IPanel
 * @brief Base interface for all screen panels in the Clarity system
 * 
 * @details This interface defines the contract for panels that manage complete
 * screens in the MVP architecture. Panels act as Presenters, coordinating
 * between sensors (models) and components (views) to create cohesive displays.
 * 
 * @design_pattern Presenter in MVP - coordinates models and views
 * @lifecycle:
 * 1. init(): Initialize panel and create components
 * 2. load(): Setup UI and start async operations with callback
 * 3. update(): Periodic refresh of sensor data and UI
 * 4. show(): Make panel visible on screen
 * 
 * @async_handling:
 * - load() accepts completion callbacks for smooth transitions
 * - Panels can perform time-consuming initialization without blocking
 * - PanelManager uses callbacks to coordinate panel switching
 * 
 * @sensor_integration:
 * - Panels create and manage sensor instances
 * - Regular update() calls refresh sensor readings
 * - Sensor data passed to components via refresh() methods
 * 
 * @implementations:
 * - SplashPanel: Startup branding screen
 * - OemOilPanel: Main oil monitoring dashboard
 * - KeyPanel: Key status display
 * - LockPanel: Lock status display
 * 
 * @context This is the base interface for all screen panels.
 * Panels coordinate the entire screen experience, managing sensors
 * and components to create complete user interfaces.
 */
class IPanel
{
public:
    // Destructors
    virtual ~IPanel() = default;

    // Core Interface Methods
    /// @brief Initialize the panel and its components
    /// @param gpio GPIO provider for hardware access
    /// @param display Display provider for UI operations
    virtual void Init(IGpioProvider *gpio, IDisplayProvider *display) = 0;

    /// @brief Load the panel with asynchronous completion callback
    /// @param callbackFunction Function to call when loading is complete
    /// @param gpio GPIO provider for hardware access
    /// @param display Display provider for UI operations
    virtual void Load(std::function<void()> callbackFunction, IGpioProvider *gpio, IDisplayProvider *display) = 0;

    /// @brief Update the panel data with asynchronous completion callback
    /// @param callbackFunction Function to call when update is complete
    /// @param gpio GPIO provider for hardware access
    /// @param display Display provider for UI operations
    virtual void Update(std::function<void()> callbackFunction, IGpioProvider *gpio, IDisplayProvider *display) = 0;

    /// @brief Show the panel in the display
    virtual void Show() {
        if (screen_) {
            display_->LoadScreen(screen_);
        }
    }
    
    /// @brief Get the input service interface if this panel supports input
    /// @return Pointer to IInputService or nullptr if not supported
    virtual IInputService* GetInputService() { return nullptr; }

protected:
    lv_obj_t *screen_ = nullptr;
    IDisplayProvider *display_ = nullptr;

protected:
    // Protected Data Members
    std::function<void()> callbackFunction_;
};