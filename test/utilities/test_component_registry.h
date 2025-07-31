#pragma once

#include "system/component_registry.h"
#include "interfaces/i_panel.h"
#include "interfaces/i_component.h"
#include "utilities/types.h"
#include <functional>
#include <lvgl.h>

namespace TestUtilities {

// Test panel that can be registered for testing
class TestKeyPanel : public IPanel {
public:
    TestKeyPanel() : initialized_(false) {}
    
    void init(IGpioProvider* gpio, IDisplayProvider* display) override { 
        gpio_ = gpio;
        display_ = display;
        initialized_ = true; 
    }
    void load(std::function<void()> callbackFunction, IGpioProvider* gpio, IDisplayProvider* display) override {}
    void update(std::function<void()> callbackFunction, IGpioProvider* gpio, IDisplayProvider* display) override {}
    const char* getPanelName() const { return "test_key"; }
    bool isInitialized() const { return initialized_; }

private:
    IGpioProvider* gpio_ = nullptr;
    IDisplayProvider* display_ = nullptr;
    bool initialized_;
};

// Test component that can be registered for testing
class TestKeyComponent : public IComponent {
public:
    TestKeyComponent() : display_(nullptr), loaded_(false), updated_(false) {}
    
    void render(lv_obj_t *screen, const ComponentLocation& location, IDisplayProvider* display) override { 
        display_ = display;
        loaded_ = true;
    }
    void refresh(const Reading& reading) override { updated_ = true; }
    bool isLoaded() const { return loaded_; }
    bool isUpdated() const { return updated_; }

private:
    IDisplayProvider* display_;
    bool loaded_;
    bool updated_;
};

// Helper function to register test components
void registerTestComponents() {
    auto& registry = ComponentRegistry::GetInstance();
    
    // Clear existing registrations
    registry.clear();
    
    // Register test panels
    registry.registerPanel("key", [](IGpioProvider* gpio, IDisplayProvider* display) {
        return std::make_unique<TestKeyPanel>();
    });
    
    // Register test components
    registry.registerComponent("key", [](IDisplayProvider* display, IStyleService* style) {
        return std::make_unique<TestKeyComponent>();
    });
}

} // namespace TestUtilities