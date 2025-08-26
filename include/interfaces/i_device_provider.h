#pragma once

#include <lvgl.h>

/**
 * @interface IDeviceProvider  
 * @brief Interface for device hardware abstraction layer
 *
 * @details This interface provides hardware abstraction for device operations,
 * enabling dependency injection and testability for hardware-specific components.
 * It abstracts device initialization, screen management, and hardware setup.
 *
 * @hardware_abstraction Enables testing with mock device implementations
 * @dependency_injection Injectable into managers and providers
 * @lvgl_integration Provides LVGL-specific device functionality
 *
 * @core_capabilities:
 * - Device initialization and hardware setup
 * - Screen object management and access
 * - Hardware driver configuration
 * - Display controller communication
 *
 * @implementation_notes:
 * - Real hardware: DeviceProvider with LovyanGFX integration
 * - Testing: MockDeviceProvider with simulated hardware
 * - LVGL threading: All operations must be LVGL thread-safe
 *
 * @memory_management Interface does not own LVGL objects
 * @thread_safety Must be called from appropriate thread context
 * @performance Hardware-specific optimizations in implementations
 *
 * @context This interface allows hardware-dependent components to work with
 * device drivers without direct hardware dependencies, enabling unit testing
 * and hardware abstraction for the display and GPIO system.
 */
class IDeviceProvider
{
public:
    virtual ~IDeviceProvider() = default;

    /// @brief Prepare and initialize the device hardware
    virtual void prepare() = 0;

    /// @brief Get the main screen object
    /// @return Pointer to the screen object, nullptr if not initialized
    virtual lv_obj_t* GetScreen() const = 0;

    /// @brief Check if the device has been properly initialized
    /// @return True if device is ready, false otherwise
    virtual bool IsReady() const = 0;
};