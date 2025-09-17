#pragma once

#include <lvgl.h>

/**
 * @interface IDisplayProvider
 * @brief Interface for display hardware abstraction layer
 *
 * @details This interface provides hardware abstraction for display operations,
 * enabling dependency injection and testability for LVGL-based UI components.
 * It abstracts display initialization, screen management, and drawing operations.
 *
 * @display_abstraction Enables testing with mock display implementations
 * @dependency_injection Injectable into panels and UI components
 * @lvgl_integration Provides LVGL-specific display functionality
 *
 * @core_capabilities:
 * - Display initialization and hardware setup
 * - Screen and object management
 * - Display properties (width, height, capabilities)
 * - Hardware-specific optimizations
 *
 * @implementation_notes:
 * - Real hardware: LvglDisplayProvider with DeviceProvider integration
 * - Testing: MockDisplayProvider with simulated display
 * - LVGL threading: All operations must be LVGL thread-safe
 *
 * @memory_management Interface does not own LVGL objects
 * @thread_safety Must be called from LVGL thread context
 * @performance Hardware-specific optimizations in implementations
 *
 * @context This interface allows UI components to work with displays
 * without direct hardware dependencies, enabling unit testing and
 * hardware abstraction for the LVGL-based UI system.
 */
class IDisplayProvider
{
  public:
    virtual ~IDisplayProvider() = default;

    virtual lv_obj_t *CreateScreen() = 0;
    virtual lv_obj_t *CreateLabel(lv_obj_t *parent) = 0;
    virtual lv_obj_t *CreateObject(lv_obj_t *parent) = 0;
    virtual lv_obj_t *CreateArc(lv_obj_t *parent) = 0;
    virtual lv_obj_t *CreateScale(lv_obj_t *parent) = 0;
    virtual lv_obj_t *CreateImage(lv_obj_t *parent) = 0;
};