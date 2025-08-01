#pragma once

#include <lvgl.h>

/// @brief Interface for display hardware abstraction
/// @details Provides abstraction for LVGL display operations
/// to enable testing and display independence
class IDisplayProvider
{
public:
    virtual ~IDisplayProvider() = default;

    /// @brief Initialize the display hardware and LVGL
    virtual void initialize() = 0;

    /// @brief Check if the display has been initialized
    /// @return True if initialized, false otherwise
    virtual bool isInitialized() const = 0;

    /// @brief Create a new screen object
    /// @return Pointer to the created screen object
    virtual lv_obj_t *createScreen() = 0;

    /// @brief Load a screen and make it active
    /// @param screen Screen object to load
    virtual void loadScreen(lv_obj_t *screen) = 0;

    /// @brief Create a label object
    /// @param parent Parent object (screen or container)
    /// @return Pointer to the created label object
    virtual lv_obj_t *createLabel(lv_obj_t *parent) = 0;

    /// @brief Create a generic object/container
    /// @param parent Parent object (screen or container)
    /// @return Pointer to the created object
    virtual lv_obj_t *createObject(lv_obj_t *parent) = 0;

    /// @brief Create an arc (gauge) object
    /// @param parent Parent object (screen or container)
    /// @return Pointer to the created arc object
    virtual lv_obj_t *createArc(lv_obj_t *parent) = 0;

    /// @brief Create a scale object
    /// @param parent Parent object (screen or container)
    /// @return Pointer to the created scale object
    virtual lv_obj_t *createScale(lv_obj_t *parent) = 0;

    /// @brief Create an image object
    /// @param parent Parent object (screen or container)
    /// @return Pointer to the created image object
    virtual lv_obj_t *createImage(lv_obj_t *parent) = 0;

    /// @brief Create a line object
    /// @param parent Parent object (screen or container)
    /// @return Pointer to the created line object
    virtual lv_obj_t *createLine(lv_obj_t *parent) = 0;

    /// @brief Delete an object and its children
    /// @param obj Object to delete
    virtual void deleteObject(lv_obj_t *obj) = 0;

    /// @brief Add event callback to an object
    /// @param obj Target object
    /// @param callback Callback function
    /// @param event_code Event code to listen for
    /// @param user_data User data to pass to callback
    virtual void addEventCallback(lv_obj_t *obj, lv_event_cb_t callback, lv_event_code_t event_code, void *user_data) = 0;

    /// @brief Get the main screen object
    /// @return Pointer to the main screen
    virtual lv_obj_t *getMainScreen() = 0;
};