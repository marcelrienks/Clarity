#pragma once

#include "interfaces/i_display_provider.h"
#include <lvgl.h>

/**
 * @class LvglDisplayProvider
 * @brief LVGL hardware implementation of display provider interface
 * 
 * @details This class provides the concrete implementation of IDisplayProvider
 * interface, handling all LVGL display operations and object creation. It
 * manages LVGL objects, screens, and provides a clean abstraction layer for
 * UI components to interact with the LVGL graphics library.
 * 
 * @display_abstraction Bridge between IDisplayProvider interface and LVGL API
 * @dependency_injection Injectable into panels and components for testability
 * @lvgl_integration Direct LVGL API wrapper with safety checks
 * 
 * @object_management:
 * - Screen creation and management
 * - LVGL object creation (labels, arcs, scales, images, lines)
 * - Object deletion and cleanup
 * - Event callback registration
 * 
 * @screen_lifecycle:
 * - createScreen(): Create new LVGL screen objects
 * - loadScreen(): Make screen active and visible
 * - getMainScreen(): Access to primary display screen
 * 
 * @safety_features:
 * - Initialization state tracking
 * - Null pointer checks on object operations
 * - LVGL thread safety compliance
 * 
 * @performance_considerations:
 * - Direct LVGL API calls for minimal overhead
 * - Efficient object creation patterns
 * - Memory management delegated to LVGL
 * 
 * @context This is the primary display provider implementation used
 * throughout the system. It wraps LVGL operations in a clean interface
 * that can be dependency-injected into UI components for testing.
 */
class LvglDisplayProvider : public IDisplayProvider
{
private:
    lv_obj_t *mainScreen_;
    bool initialized_;

public:
    /// @brief Constructor
    explicit LvglDisplayProvider(lv_obj_t *mainScreen);

    /// @brief Initialize the display provider
    void initialize() override;

    /// @brief Check if the display provider is initialized
    /// @return True if initialized, false otherwise
    bool isInitialized() const override;

    /// @brief Create a new screen object
    /// @return Pointer to the created screen object
    lv_obj_t *createScreen() override;

    /// @brief Load a screen and make it active
    /// @param screen Screen object to load
    void loadScreen(lv_obj_t *screen) override;

    /// @brief Create a label object
    /// @param parent Parent object (screen or container)
    /// @return Pointer to the created label object
    lv_obj_t *createLabel(lv_obj_t *parent) override;

    /// @brief Create a generic object/container
    /// @param parent Parent object (screen or container)
    /// @return Pointer to the created object
    lv_obj_t *createObject(lv_obj_t *parent) override;

    /// @brief Create an arc (gauge) object
    /// @param parent Parent object (screen or container)
    /// @return Pointer to the created arc object
    lv_obj_t *createArc(lv_obj_t *parent) override;

    /// @brief Create a scale object
    /// @param parent Parent object (screen or container)
    /// @return Pointer to the created scale object
    lv_obj_t *createScale(lv_obj_t *parent) override;

    /// @brief Create an image object
    /// @param parent Parent object (screen or container)
    /// @return Pointer to the created image object
    lv_obj_t *createImage(lv_obj_t *parent) override;

    /// @brief Create a line object
    /// @param parent Parent object (screen or container)
    /// @return Pointer to the created line object
    lv_obj_t *createLine(lv_obj_t *parent) override;

    /// @brief Delete an object and its children
    /// @param obj Object to delete
    void deleteObject(lv_obj_t *obj) override;

    /// @brief Add event callback to an object
    /// @param obj Target object
    /// @param callback Callback function
    /// @param event_code Event code to listen for
    /// @param user_data User data to pass to callback
    void addEventCallback(lv_obj_t *obj, lv_event_cb_t callback, lv_event_code_t event_code, void *user_data) override;

    /// @brief Get the main screen object
    /// @return Pointer to the main screen
    lv_obj_t *getMainScreen() override;
};