#include "providers/lvgl_display_provider.h"
#include "managers/style_manager.h"
#include <esp32-hal-log.h>

LvglDisplayProvider::LvglDisplayProvider(lv_obj_t *mainScreen) : mainScreen_(mainScreen), initialized_(false)
{
    log_v("LvglDisplayProvider() constructor called");
}

void LvglDisplayProvider::Initialize()
{
    log_v("Initialize() called");
    if (!initialized_)
    {
        initialized_ = true;
    }
}

bool LvglDisplayProvider::IsInitialized() const
{
    log_v("IsInitialized() called");
    return initialized_;
}

lv_obj_t *LvglDisplayProvider::CreateScreen()
{
    log_v("CreateScreen() called");
    return lv_obj_create(nullptr);
}

void LvglDisplayProvider::LoadScreen(lv_obj_t *screen)
{
    log_v("LoadScreen() called");
#ifdef UNIT_TESTING
    lv_scr_load(screen);
#else
    lv_screen_load(screen);
#endif
}

lv_obj_t *LvglDisplayProvider::CreateLabel(lv_obj_t *parent)
{
    log_v("CreateLabel() called");
    return lv_label_create(parent);
}

lv_obj_t *LvglDisplayProvider::CreateObject(lv_obj_t *parent)
{
    log_v("CreateObject() called");
    return lv_obj_create(parent);
}

lv_obj_t *LvglDisplayProvider::CreateArc(lv_obj_t *parent)
{
    log_v("CreateArc() called");
    return lv_arc_create(parent);
}

lv_obj_t *LvglDisplayProvider::CreateScale(lv_obj_t *parent)
{
    log_v("CreateScale() called");
#ifdef UNIT_TESTING
    return lv_arc_create(parent); // Use arc as substitute in tests
#else
    return lv_scale_create(parent);
#endif
}

lv_obj_t *LvglDisplayProvider::CreateImage(lv_obj_t *parent)
{
    log_v("CreateImage() called");
#ifdef UNIT_TESTING
    return lv_img_create(parent); // Use available img_create function
#else
    return lv_image_create(parent);
#endif
}

lv_obj_t *LvglDisplayProvider::CreateLine(lv_obj_t *parent)
{
    log_v("CreateLine() called");
#ifdef UNIT_TESTING
    return lv_obj_create(parent); // Use generic object for line in tests
#else
    return lv_line_create(parent);
#endif
}

void LvglDisplayProvider::DeleteObject(lv_obj_t *obj)
{
    log_v("DeleteObject() called");
    if (obj != nullptr)
    {
#ifdef UNIT_TESTING
        lv_obj_del(obj);
#else
        lv_obj_delete(obj);
#endif
    }
}

void LvglDisplayProvider::AddEventCallback(lv_obj_t *obj, lv_event_cb_t callback, lv_event_code_t event_code,
                                           void *user_data)
{
    log_v("AddEventCallback() called");
    lv_obj_add_event_cb(obj, callback, event_code, user_data);
}

lv_obj_t *LvglDisplayProvider::GetMainScreen()
{
    log_v("GetMainScreen() called");
    return mainScreen_;
}