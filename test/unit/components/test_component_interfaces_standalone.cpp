#include <unity.h>
#include <memory>
#include <stdexcept>
#include "interfaces/i_component.h"
#include "interfaces/i_style_service.h"
#include "mock_services.h"
#include "mock_gpio_provider.h"
#include "utilities/types.h"

// Simple standalone mock component implementation for interface testing
class StandaloneTestComponent : public IComponent {
public:
    StandaloneTestComponent(IStyleService* style) : style_service_(style) {
        render_called = false;
        refresh_called = false;
        set_value_called = false;
        last_value = 0;
        last_reading = Reading{};
    }

    void render(lv_obj_t *screen, const ComponentLocation& location, IDisplayProvider* display) override {
        render_called = true;
        last_screen = screen;
        last_location = location;
        last_display = display;
        
        // Simulate creating LVGL objects
        if (screen) {
            component_obj = lv_obj_create(screen);
            lv_obj_set_pos(component_obj, location.x, location.y);
            lv_obj_align(component_obj, location.align, location.x_offset, location.y_offset);
        }
    }

    void refresh(const Reading& reading) override {
        refresh_called = true;
        last_reading = reading;
    }

    void setValue(int32_t value) override {
        set_value_called = true;
        last_value = value;
    }

    // Test state tracking
    bool render_called;
    bool refresh_called;
    bool set_value_called;
    int32_t last_value;
    Reading last_reading;
    lv_obj_t* last_screen;
    ComponentLocation last_location;
    IDisplayProvider* last_display;
    lv_obj_t* component_obj;
    IStyleService* style_service_;
};

// Standalone mock services
static MockDisplayProvider* standalone_display = nullptr;
static MockStyleService* standalone_style = nullptr;

void test_standalone_component_construction() {
    standalone_display = new MockDisplayProvider();
    standalone_style = new MockStyleService();
    standalone_display->initialize();
    standalone_style->initializeStyles();
    
    auto component = std::make_unique<StandaloneTestComponent>(standalone_style);
    
    TEST_ASSERT_NOT_NULL(component.get());
    TEST_ASSERT_EQUAL_PTR(standalone_style, component->style_service_);
    TEST_ASSERT_FALSE(component->render_called);
    TEST_ASSERT_FALSE(component->refresh_called);
    TEST_ASSERT_FALSE(component->set_value_called);
    
    delete standalone_display;
    delete standalone_style;
}

void test_standalone_component_render() {
    standalone_display = new MockDisplayProvider();
    standalone_style = new MockStyleService();
    standalone_display->initialize();
    standalone_style->initializeStyles();
    
    auto component = std::make_unique<StandaloneTestComponent>(standalone_style);
    
    lv_obj_t* screen = standalone_display->getMainScreen();
    ComponentLocation location(static_cast<lv_coord_t>(10), static_cast<lv_coord_t>(20));
    
    component->render(screen, location, standalone_display);
    
    TEST_ASSERT_TRUE(component->render_called);
    TEST_ASSERT_EQUAL_PTR(screen, component->last_screen);
    TEST_ASSERT_EQUAL_PTR(standalone_display, component->last_display);
    TEST_ASSERT_EQUAL(10, component->last_location.x);
    TEST_ASSERT_EQUAL(20, component->last_location.y);
    TEST_ASSERT_NOT_NULL(component->component_obj);
    
    delete standalone_display;
    delete standalone_style;
}

void test_standalone_component_refresh() {
    standalone_display = new MockDisplayProvider();
    standalone_style = new MockStyleService();
    standalone_display->initialize();
    standalone_style->initializeStyles();
    
    auto component = std::make_unique<StandaloneTestComponent>(standalone_style);
    
    Reading testReading = 75.5;
    component->refresh(testReading);
    
    TEST_ASSERT_TRUE(component->refresh_called);
    
    delete standalone_display;
    delete standalone_style;
}

void test_standalone_component_set_value() {
    standalone_display = new MockDisplayProvider();
    standalone_style = new MockStyleService();
    standalone_display->initialize();
    standalone_style->initializeStyles();
    
    auto component = std::make_unique<StandaloneTestComponent>(standalone_style);
    
    component->setValue(42);
    
    TEST_ASSERT_TRUE(component->set_value_called);
    TEST_ASSERT_EQUAL(42, component->last_value);
    
    delete standalone_display;
    delete standalone_style;
}

void test_standalone_component_lifecycle() {
    standalone_display = new MockDisplayProvider();
    standalone_style = new MockStyleService();
    standalone_display->initialize();
    standalone_style->initializeStyles();
    
    auto component = std::make_unique<StandaloneTestComponent>(standalone_style);
    
    // 1. Render component
    lv_obj_t* screen = standalone_display->getMainScreen();
    ComponentLocation location(static_cast<lv_coord_t>(5), static_cast<lv_coord_t>(10));
    component->render(screen, location, standalone_display);
    
    TEST_ASSERT_TRUE(component->render_called);
    TEST_ASSERT_NOT_NULL(component->component_obj);
    
    // 2. Refresh with sensor data
    Reading reading = 88.3;
    component->refresh(reading);
    
    TEST_ASSERT_TRUE(component->refresh_called);
    
    // 3. Direct value update
    component->setValue(99);
    
    TEST_ASSERT_TRUE(component->set_value_called);
    TEST_ASSERT_EQUAL(99, component->last_value);
    
    delete standalone_display;
    delete standalone_style;
}

void runStandaloneComponentTests() {
    RUN_TEST(test_standalone_component_construction);
    RUN_TEST(test_standalone_component_render);
    RUN_TEST(test_standalone_component_refresh);
    RUN_TEST(test_standalone_component_set_value);
    RUN_TEST(test_standalone_component_lifecycle);
}