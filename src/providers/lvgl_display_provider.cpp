#include "providers/lvgl_display_provider.h"
#include "managers/style_manager.h"

LvglDisplayProvider::LvglDisplayProvider(lv_obj_t *mainScreen)
    : mainScreen_(mainScreen), initialized_(false)
{
}

void LvglDisplayProvider::initialize() {
    if (!initialized_) {
        initialized_ = true;
    }
}

bool LvglDisplayProvider::isInitialized() const {
    return initialized_;
}

lv_obj_t *LvglDisplayProvider::createScreen()
{
    return lv_obj_create(nullptr);
}

void LvglDisplayProvider::loadScreen(lv_obj_t *screen)
{
#ifdef UNIT_TESTING
    lv_scr_load(screen);
#else
    lv_screen_load(screen);
#endif
}

lv_obj_t *LvglDisplayProvider::createLabel(lv_obj_t *parent)
{
    return lv_label_create(parent);
}

lv_obj_t *LvglDisplayProvider::createObject(lv_obj_t *parent)
{
    return lv_obj_create(parent);
}

lv_obj_t *LvglDisplayProvider::createArc(lv_obj_t *parent)
{
    return lv_arc_create(parent);
}

lv_obj_t *LvglDisplayProvider::createScale(lv_obj_t *parent)
{
#ifdef UNIT_TESTING
    return lv_arc_create(parent); // Use arc as substitute in tests
#else
    return lv_scale_create(parent);
#endif
}

lv_obj_t *LvglDisplayProvider::createImage(lv_obj_t *parent)
{
#ifdef UNIT_TESTING
    return lv_img_create(parent); // Use available img_create function
#else
    return lv_image_create(parent);
#endif
}

lv_obj_t *LvglDisplayProvider::createLine(lv_obj_t *parent)
{
#ifdef UNIT_TESTING
    return lv_obj_create(parent); // Use generic object for line in tests
#else
    return lv_line_create(parent);
#endif
}

void LvglDisplayProvider::deleteObject(lv_obj_t *obj)
{
    if (obj != nullptr) {
#ifdef UNIT_TESTING
        lv_obj_del(obj);
#else
        lv_obj_delete(obj);
#endif
    }
}

void LvglDisplayProvider::addEventCallback(lv_obj_t *obj, lv_event_cb_t callback, lv_event_code_t event_code, void *user_data)
{
    lv_obj_add_event_cb(obj, callback, event_code, user_data);
}

lv_obj_t *LvglDisplayProvider::getMainScreen()
{
    return mainScreen_;
}