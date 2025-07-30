#pragma once

#include "interfaces/i_display_provider.h"
#include <lvgl.h>

/// @brief LVGL hardware implementation of display provider
/// @details Provides concrete implementation for LVGL display operations
class LvglDisplayProvider : public IDisplayProvider
{
private:
    lv_obj_t* mainScreen_;

public:
    /// @brief Constructor
    explicit LvglDisplayProvider(lv_obj_t* mainScreen);

    /// @brief Create a new screen object
    /// @return Pointer to the created screen object
    lv_obj_t* createScreen() override;

    /// @brief Load a screen and make it active
    /// @param screen Screen object to load
    void loadScreen(lv_obj_t* screen) override;

    /// @brief Create a label object
    /// @param parent Parent object (screen or container)
    /// @return Pointer to the created label object
    lv_obj_t* createLabel(lv_obj_t* parent) override;

    /// @brief Create a generic object/container
    /// @param parent Parent object (screen or container)
    /// @return Pointer to the created object
    lv_obj_t* createObject(lv_obj_t* parent) override;

    /// @brief Create an arc (gauge) object
    /// @param parent Parent object (screen or container)
    /// @return Pointer to the created arc object
    lv_obj_t* createArc(lv_obj_t* parent) override;

    /// @brief Delete an object and its children
    /// @param obj Object to delete
    void deleteObject(lv_obj_t* obj) override;

    /// @brief Add event callback to an object
    /// @param obj Target object
    /// @param callback Callback function
    /// @param event_code Event code to listen for
    /// @param user_data User data to pass to callback
    void addEventCallback(lv_obj_t* obj, lv_event_cb_t callback, lv_event_code_t event_code, void* user_data) override;

    /// @brief Get the main screen object
    /// @return Pointer to the main screen
    lv_obj_t* getMainScreen() override;
};