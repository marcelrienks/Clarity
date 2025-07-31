#include "providers/lvgl_display_provider.h"
#include "managers/style_manager.h"

LvglDisplayProvider::LvglDisplayProvider(lv_obj_t* mainScreen)
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

lv_obj_t* LvglDisplayProvider::createScreen()
{
    return lv_obj_create(nullptr);
}

void LvglDisplayProvider::loadScreen(lv_obj_t* screen)
{
    lv_screen_load(screen);
}

lv_obj_t* LvglDisplayProvider::createLabel(lv_obj_t* parent)
{
    return lv_label_create(parent);
}

lv_obj_t* LvglDisplayProvider::createObject(lv_obj_t* parent)
{
    return lv_obj_create(parent);
}

lv_obj_t* LvglDisplayProvider::createArc(lv_obj_t* parent)
{
    return lv_arc_create(parent);
}

lv_obj_t* LvglDisplayProvider::createScale(lv_obj_t* parent)
{
    return lv_scale_create(parent);
}

lv_obj_t* LvglDisplayProvider::createImage(lv_obj_t* parent)
{
    return lv_image_create(parent);
}

lv_obj_t* LvglDisplayProvider::createLine(lv_obj_t* parent)
{
    return lv_line_create(parent);
}

void LvglDisplayProvider::deleteObject(lv_obj_t* obj)
{
    if (obj != nullptr) {
        lv_obj_delete(obj);
    }
}

void LvglDisplayProvider::addEventCallback(lv_obj_t* obj, lv_event_cb_t callback, lv_event_code_t event_code, void* user_data)
{
    lv_obj_add_event_cb(obj, callback, event_code, user_data);
}

lv_obj_t* LvglDisplayProvider::getMainScreen()
{
    return mainScreen_;
}