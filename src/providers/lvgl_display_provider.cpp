#include "providers/lvgl_display_provider.h"
#include "managers/style_manager.h"
#include <esp32-hal-log.h>

/**
 * @brief Constructs the LVGL display provider with the main screen reference
 * @param mainScreen Pointer to the main LVGL screen object
 *
 * Initializes the display provider with a reference to the main screen.
 * This provides abstraction for LVGL display operations and allows for
 * consistent display management across the application.
 */
LvglDisplayProvider::LvglDisplayProvider(lv_obj_t *mainScreen) : mainScreen_(mainScreen), initialized_(false)
{
    log_v("LvglDisplayProvider() constructor called");
}

/**
 * @brief Initializes the LVGL display provider
 *
 * Sets the initialization state of the display provider. This ensures
 * the provider is ready for display operations and prevents duplicate
 * initialization attempts.
 */
void LvglDisplayProvider::Initialize()
{
    log_v("Initialize() called");
    if (!initialized_)
    {
        initialized_ = true;
    }
}

/**
 * @brief Checks if the display provider is initialized
 * @return true if initialized, false otherwise
 *
 * Returns the initialization state of the display provider. Used by
 * other components to verify the display system is ready before
 * attempting display operations.
 */
bool LvglDisplayProvider::IsInitialized() const
{
    return initialized_;
}

/**
 * @brief Creates a new LVGL screen object
 * @return Pointer to the newly created screen object
 *
 * Creates a new top-level LVGL screen for panels to render their UI.
 * The screen is created without a parent, making it a root object
 * suitable for screen transitions.
 */
lv_obj_t *LvglDisplayProvider::CreateScreen()
{
    return lv_obj_create(nullptr);
}

/**
 * @brief Loads an LVGL screen as the active display
 * @param screen Pointer to the screen object to load
 *
 * Transitions to the specified screen as the active display. Handles
 * API differences between unit testing and production environments
 * to ensure compatibility across build configurations.
 */
void LvglDisplayProvider::LoadScreen(lv_obj_t *screen)
{
#ifdef UNIT_TESTING
    lv_scr_load(screen);
#else
    lv_screen_load(screen);
#endif
}

/**
 * @brief Creates an LVGL label widget
 * @param parent Parent object for the label
 * @return Pointer to the newly created label object
 *
 * Creates a text label widget for displaying strings on the UI.
 * The label is created as a child of the specified parent object.
 */
lv_obj_t *LvglDisplayProvider::CreateLabel(lv_obj_t *parent)
{
    return lv_label_create(parent);
}

/**
 * @brief Creates a generic LVGL object
 * @param parent Parent object for the new object
 * @return Pointer to the newly created object
 *
 * Creates a generic LVGL container object. Used as a base for
 * custom widgets and layout containers.
 */
lv_obj_t *LvglDisplayProvider::CreateObject(lv_obj_t *parent)
{
    return lv_obj_create(parent);
}

/**
 * @brief Creates an LVGL arc widget
 * @param parent Parent object for the arc
 * @return Pointer to the newly created arc object
 *
 * Creates an arc widget for displaying circular gauges and progress
 * indicators. Commonly used for sensor value visualization.
 */
lv_obj_t *LvglDisplayProvider::CreateArc(lv_obj_t *parent)
{
    return lv_arc_create(parent);
}

/**
 * @brief Creates an LVGL scale widget
 * @param parent Parent object for the scale
 * @return Pointer to the newly created scale object
 *
 * Creates a scale widget for displaying measurement scales with tick marks.
 * Falls back to arc widget in unit testing environment where scale widget
 * may not be available.
 */
lv_obj_t *LvglDisplayProvider::CreateScale(lv_obj_t *parent)
{
#ifdef UNIT_TESTING
    return lv_arc_create(parent); // Use arc as substitute in tests
#else
    return lv_scale_create(parent);
#endif
}

/**
 * @brief Creates an LVGL image widget
 * @param parent Parent object for the image
 * @return Pointer to the newly created image object
 *
 * Creates an image widget for displaying graphics and icons.
 * Handles API differences between LVGL versions for compatibility.
 */
lv_obj_t *LvglDisplayProvider::CreateImage(lv_obj_t *parent)
{
#ifdef UNIT_TESTING
    return lv_img_create(parent); // Use available img_create function
#else
    return lv_image_create(parent);
#endif
}

/**
 * @brief Creates an LVGL line widget
 * @param parent Parent object for the line
 * @return Pointer to the newly created line object
 *
 * Creates a line widget for drawing lines between points.
 * Falls back to generic object in unit testing environment where
 * line widget may not be available.
 */
lv_obj_t *LvglDisplayProvider::CreateLine(lv_obj_t *parent)
{
#ifdef UNIT_TESTING
    return lv_obj_create(parent); // Use generic object for line in tests
#else
    return lv_line_create(parent);
#endif
}

/**
 * @brief Deletes an LVGL object and its children
 * @param obj Pointer to the object to delete
 *
 * Safely deletes an LVGL object and all its child objects. Includes
 * null pointer check for safety. Handles API differences between
 * LVGL versions for compatibility.
 */
void LvglDisplayProvider::DeleteObject(lv_obj_t *obj)
{
    if (obj != nullptr)
    {
#ifdef UNIT_TESTING
        lv_obj_del(obj);
#else
        lv_obj_delete(obj);
#endif
    }
}

/**
 * @brief Adds an event callback to an LVGL object
 * @param obj Object to attach the callback to
 * @param callback Function pointer to the event handler
 * @param event_code Type of event to handle
 * @param user_data Optional user data passed to the callback
 *
 * Registers an event handler for specific LVGL events. Used for
 * handling user interactions and system events in the UI.
 */
void LvglDisplayProvider::AddEventCallback(lv_obj_t *obj, lv_event_cb_t callback, lv_event_code_t event_code,
                                           void *user_data)
{
    lv_obj_add_event_cb(obj, callback, event_code, user_data);
}

/**
 * @brief Gets the main LVGL screen reference
 * @return Pointer to the main screen object
 *
 * Returns the reference to the main LVGL screen used as the root
 * for all display operations. This provides access to the primary
 * display context.
 */
lv_obj_t *LvglDisplayProvider::GetMainScreen()
{
    return mainScreen_;
}