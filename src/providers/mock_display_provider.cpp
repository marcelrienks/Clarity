#include "providers/mock_display_provider.h"

MockDisplayProvider::MockDisplayProvider()
    : mainScreen_(createMockObject()), currentScreen_(nullptr)
{
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

void MockDisplayProvider::deleteObject(lv_obj_t* obj)
{
    if (obj == nullptr) return;

    auto it = std::find_if(createdObjects_.begin(), createdObjects_.end(),
        [obj](const std::unique_ptr<lv_obj_t>& ptr) {
            return ptr.get() == obj;
        });

    if (it != createdObjects_.end()) {
        createdObjects_.erase(it);
    }

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
    return createdObjects_.size();
}

void MockDisplayProvider::reset()
{
    createdObjects_.clear();
    currentScreen_ = nullptr;
    mainScreen_ = createMockObject();
}

lv_obj_t* MockDisplayProvider::createMockObject()
{
    auto obj = std::make_unique<lv_obj_t>();
    lv_obj_t* ptr = obj.get();
    createdObjects_.push_back(std::move(obj));
    return ptr;
}