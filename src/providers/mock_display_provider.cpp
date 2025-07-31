#include "providers/mock_display_provider.h"

MockDisplayProvider::MockDisplayProvider()
    : mainScreen_(nullptr), currentScreen_(nullptr)
{
    static bool lvglInitialized = false;
    if (!lvglInitialized) {
        // Initialize LVGL only once
        lv_init();
        // Create a display buffer
        static lv_color_t buf1[SCREEN_WIDTH * 10];
        static lv_color_t buf2[SCREEN_WIDTH * 10];
        lv_display_t* display = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
        lv_display_set_buffers(display, buf1, buf2, sizeof(buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);
        lvglInitialized = true;
    }

    // Create main screen
    mainScreen_ = createMockObject();
}

MockDisplayProvider::~MockDisplayProvider()
{
    reset();
}

lv_obj_t* MockDisplayProvider::createScreen()
{
    return createMockObject();
}

void MockDisplayProvider::loadScreen(lv_obj_t* screen)
{
    currentScreen_ = screen;
}

lv_obj_t* MockDisplayProvider::createLabel(lv_obj_t* parent)
{
    return createMockObject();
}

lv_obj_t* MockDisplayProvider::createObject(lv_obj_t* parent)
{
    return createMockObject();
}

lv_obj_t* MockDisplayProvider::createArc(lv_obj_t* parent)
{
    return createMockObject();
}

lv_obj_t* MockDisplayProvider::createScale(lv_obj_t* parent)
{
    return createMockObject();
}

lv_obj_t* MockDisplayProvider::createImage(lv_obj_t* parent)
{
    return createMockObject();
}

lv_obj_t* MockDisplayProvider::createLine(lv_obj_t* parent)
{
    return createMockObject();
}

void MockDisplayProvider::deleteObject(lv_obj_t* obj)
{
    if (obj == nullptr) return;

#ifdef UNIT_TESTING
    // In testing, manage object deletion ourselves
    auto it = std::find(createdObjects_.begin(), createdObjects_.end(), obj);
    if (it != createdObjects_.end()) {
        createdObjects_.erase(it);
        delete obj;
    }
#else
    // In real environment, use LVGL's deletion
    lv_obj_del(obj);
#endif

    if (currentScreen_ == obj) {
        currentScreen_ = nullptr;
    }
}

void MockDisplayProvider::addEventCallback(lv_obj_t* obj, lv_event_cb_t callback, lv_event_code_t event_code, void* user_data)
{
    // Mock implementation - store callback info if needed for testing
}

lv_obj_t* MockDisplayProvider::getMainScreen()
{
    return mainScreen_;
}

lv_obj_t* MockDisplayProvider::getCurrentScreen() const
{
    return currentScreen_;
}

size_t MockDisplayProvider::getObjectCount() const
{
#ifdef UNIT_TESTING
    return createdObjects_.size();
#else
    return 0;
#endif
}

void MockDisplayProvider::reset()
{
#ifdef UNIT_TESTING
    // In testing, manage our own memory
    for (auto* obj : createdObjects_) {
        delete obj;
    }
    createdObjects_.clear();
#else
    // In real environment, let LVGL handle cleanup
    if (mainScreen_) {
        lv_obj_del(mainScreen_);
    }
    if (currentScreen_ && currentScreen_ != mainScreen_) {
        lv_obj_del(currentScreen_);
    }
#endif
    currentScreen_ = nullptr;
    mainScreen_ = createMockObject();
}

lv_obj_t* MockDisplayProvider::createMockObject()
{
    // In UNIT_TESTING we manage our own objects
#ifdef UNIT_TESTING
    auto obj = new lv_obj_t();
    createdObjects_.push_back(obj);
    return obj;
#else
    // For ESP32 build, create a real LVGL object since we have LVGL available
    // This makes the mock usable in both test and real environments
    lv_obj_t* obj = lv_obj_create(NULL);
    return obj;
#endif
}