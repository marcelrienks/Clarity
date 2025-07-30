#pragma once

// System/Library Includes
#include <string>

// Project Includes
#include "interfaces/i_component.h"
#include "interfaces/i_display_provider.h"
#include "utilities/types.h"
#include "mock_types.h"

/**
 * @class MockComponent
 * @brief Mock implementation of IComponent for testing
 */
class MockComponent : public IComponent
{
public:
    MockComponent(const std::string& name) 
        : name_(name)
        , renderCalled_(false)
        , refreshCalled_(false)
        , setValueCalled_(false)
        , lastValue_(0)
    {}

    virtual ~MockComponent() = default;

    // IComponent Methods
    void render(lv_obj_t* screen, const ComponentLocation& location, IDisplayProvider* display) override 
    {
        renderCalled_ = true;
        lastScreen_ = screen;
        lastLocation_ = location;
        lastDisplay_ = display;
    }

    void refresh(const Reading& reading) override 
    {
        refreshCalled_ = true;
        lastReading_ = reading;
    }

    void SetValue(int32_t value) override 
    {
        setValueCalled_ = true;
        lastValue_ = value;
    }

    // Test Helper Methods
    const std::string& getName() const { return name_; }
    bool wasRenderCalled() const { return renderCalled_; }
    bool wasRefreshCalled() const { return refreshCalled_; }
    bool wasSetValueCalled() const { return setValueCalled_; }
    int32_t getLastValue() const { return lastValue_; }
    const Reading& getLastReading() const { return lastReading_; }
    lv_obj_t* getLastScreen() const { return lastScreen_; }
    const ComponentLocation& getLastLocation() const { return lastLocation_; }
    IDisplayProvider* getLastDisplay() const { return lastDisplay_; }

    void reset() 
    {
        renderCalled_ = false;
        refreshCalled_ = false;
        setValueCalled_ = false;
        lastValue_ = 0;
        lastReading_ = std::monostate{};
        lastScreen_ = nullptr;
        lastDisplay_ = nullptr;
    }

private:
    std::string name_;
    bool renderCalled_;
    bool refreshCalled_;
    bool setValueCalled_;
    int32_t lastValue_;
    Reading lastReading_;
    lv_obj_t* lastScreen_;
    ComponentLocation lastLocation_;
    IDisplayProvider* lastDisplay_;
};