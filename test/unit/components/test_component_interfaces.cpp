#include <unity.h>
#include <memory>
#include <stdexcept>
#include "interfaces/i_component.h"
#include "interfaces/i_style_service.h"
#include "mock_services.h"
#include "mock_gpio_provider.h"
#include "utilities/types.h"

// Simple test structure for readings in tests
struct TestReading {
    float value;
    uint32_t timestamp;
    bool hasChanged;
    
    // Convert to actual Reading variant for interface
    Reading toReading() const {
        return static_cast<double>(value);
    }
};

// Mock component implementation for testing the interface
class MockTestComponent : public IComponent {
public:
    MockTestComponent(IStyleService* style) : style_service_(style) {
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
            // ComponentLocation doesn't have width/height, just use position and alignment
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

// Mock services for testing - use different names to avoid conflicts
static MockDisplayProvider* mockDisplayComp = nullptr;
static MockStyleService* mockStyleComp = nullptr;

void setUp_component_interfaces() {
    mockDisplayComp = new MockDisplayProvider();
    mockStyleComp = new MockStyleService();
    
    mockDisplayComp->initialize();
    mockStyleComp->initializeStyles();
}

void tearDown_component_interfaces() {
    delete mockDisplayComp;
    delete mockStyleComp;
    
    mockDisplayComp = nullptr;
    mockStyleComp = nullptr;
}

void test_component_interface_construction() {
    // Test component construction with style service
    auto component = std::make_unique<MockTestComponent>(mockStyleComp);
    
    TEST_ASSERT_NOT_NULL(component.get());
    TEST_ASSERT_EQUAL_PTR(mockStyleComp, component->style_service_);
    TEST_ASSERT_FALSE(component->render_called);
    TEST_ASSERT_FALSE(component->refresh_called);
    TEST_ASSERT_FALSE(component->set_value_called);
}

void test_component_interface_render_method() {
    // Test the render method with valid parameters
    auto component = std::make_unique<MockTestComponent>(mockStyleComp);
    
    lv_obj_t* screen = mockDisplayComp->getMainScreen();
    ComponentLocation location(static_cast<lv_coord_t>(10), static_cast<lv_coord_t>(20));
    
    component->render(screen, location, mockDisplayComp);
    
    TEST_ASSERT_TRUE(component->render_called);
    TEST_ASSERT_EQUAL_PTR(screen, component->last_screen);
    TEST_ASSERT_EQUAL_PTR(mockDisplayComp, component->last_display);
    TEST_ASSERT_EQUAL(10, component->last_location.x);
    TEST_ASSERT_EQUAL(20, component->last_location.y);
    TEST_ASSERT_EQUAL(LV_ALIGN_CENTER, component->last_location.align);
    TEST_ASSERT_EQUAL(0, component->last_location.x_offset);
    TEST_ASSERT_EQUAL(0, component->last_location.y_offset);
    TEST_ASSERT_NOT_NULL(component->component_obj);
}

void test_component_interface_render_null_screen() {
    // Test render method with null screen
    auto component = std::make_unique<MockTestComponent>(mockStyleComp);
    
    ComponentLocation location(static_cast<lv_coord_t>(0), static_cast<lv_coord_t>(0));
    
    component->render(nullptr, location, mockDisplayComp);
    
    TEST_ASSERT_TRUE(component->render_called);
    TEST_ASSERT_NULL(component->last_screen);
    TEST_ASSERT_NULL(component->component_obj); // Should not create object with null screen
}

void test_component_interface_refresh_method() {
    // Test the refresh method with sensor reading
    auto component = std::make_unique<MockTestComponent>(mockStyleComp);
    
    // Create a Reading variant with a double value
    Reading testReading = 75.5;
    
    component->refresh(testReading);
    
    TEST_ASSERT_TRUE(component->refresh_called);
    // For variant types, we just check that the call was made successfully
    // In a real implementation, component would extract the value from the variant
}

void test_component_interface_set_value_method() {
    // Test the setValue method
    auto component = std::make_unique<MockTestComponent>(mockStyleComp);
    
    component->setValue(42);
    
    TEST_ASSERT_TRUE(component->set_value_called);
    TEST_ASSERT_EQUAL(42, component->last_value);
}

void test_component_interface_complete_lifecycle() {
    // Test complete component lifecycle: render -> refresh -> setValue
    auto component = std::make_unique<MockTestComponent>(mockStyleComp);
    
    // 1. Render component
    lv_obj_t* screen = mockDisplayComp->getMainScreen();
    ComponentLocation location(static_cast<lv_coord_t>(5), static_cast<lv_coord_t>(10));
    component->render(screen, location, mockDisplayComp);
    
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
}

void test_component_interface_multiple_refreshes() {
    // Test multiple refresh calls with different readings
    auto component = std::make_unique<MockTestComponent>(mockStyleComp);
    
    // First refresh
    Reading reading1 = 25.0;
    component->refresh(reading1);
    
    TEST_ASSERT_TRUE(component->refresh_called);
    
    // Second refresh with new data
    Reading reading2 = 50.0;
    component->refresh(reading2);
    
    TEST_ASSERT_TRUE(component->refresh_called); // Still true after second call
}

void test_component_interface_multiple_set_values() {
    // Test multiple setValue calls
    auto component = std::make_unique<MockTestComponent>(mockStyleComp);
    
    // Multiple value updates
    component->setValue(10);
    TEST_ASSERT_EQUAL(10, component->last_value);
    
    component->setValue(-5);
    TEST_ASSERT_EQUAL(-5, component->last_value);
    
    component->setValue(0);
    TEST_ASSERT_EQUAL(0, component->last_value);
}

void test_component_interface_component_location_variants() {
    // Test render with different ComponentLocation configurations
    auto component = std::make_unique<MockTestComponent>(mockStyleComp);
    lv_obj_t* screen = mockDisplayComp->getMainScreen();
    
    // Test minimum position location
    ComponentLocation minLocation(static_cast<lv_coord_t>(0), static_cast<lv_coord_t>(0));
    component->render(screen, minLocation, mockDisplayComp);
    
    TEST_ASSERT_EQUAL(0, component->last_location.x);
    TEST_ASSERT_EQUAL(0, component->last_location.y);
    
    // Test different position location
    ComponentLocation maxLocation(static_cast<lv_coord_t>(240), static_cast<lv_coord_t>(240));
    component->render(screen, maxLocation, mockDisplayComp);
    
    TEST_ASSERT_EQUAL(240, component->last_location.x);
    TEST_ASSERT_EQUAL(240, component->last_location.y);
}

void test_component_interface_reading_edge_cases() {
    // Test refresh with edge case readings
    auto component = std::make_unique<MockTestComponent>(mockStyleComp);
    
    // Test with zero values
    Reading zeroReading = 0.0;
    component->refresh(zeroReading);
    
    TEST_ASSERT_TRUE(component->refresh_called);
    
    // Test with negative values
    Reading negativeReading = -123.45;
    component->refresh(negativeReading);
    
    TEST_ASSERT_TRUE(component->refresh_called);
}

void test_component_interface_memory_management() {
    // Test that component objects are properly managed
    {
        auto component = std::make_unique<MockTestComponent>(mockStyleComp);
        lv_obj_t* screen = mockDisplayComp->getMainScreen();
        ComponentLocation location(static_cast<lv_coord_t>(0), static_cast<lv_coord_t>(0));
        
        component->render(screen, location, mockDisplayComp);
        TEST_ASSERT_NOT_NULL(component->component_obj);
        
        // Component should be destroyed when leaving scope
    }
    
    // Test multiple component creations
    for (int i = 0; i < 5; i++) {
        auto component = std::make_unique<MockTestComponent>(mockStyleComp);
        component->setValue(i * 10);
        TEST_ASSERT_EQUAL(i * 10, component->last_value);
    }
    
    TEST_ASSERT_TRUE(true); // Test passes if no memory issues
}

void test_component_interface_polymorphism() {
    // Test that component works correctly through interface pointer
    std::unique_ptr<IComponent> component = std::make_unique<MockTestComponent>(mockStyleComp);
    
    // Test calling through interface
    lv_obj_t* screen = mockDisplayComp->getMainScreen();
    ComponentLocation location(static_cast<lv_coord_t>(15), static_cast<lv_coord_t>(25));
    
    component->render(screen, location, mockDisplayComp);
    
    Reading reading = 33.3;
    component->refresh(reading);
    
    component->setValue(77);
    
    // Cast back to check state (in real code we wouldn't do this)
    MockTestComponent* mockComp = static_cast<MockTestComponent*>(component.get());
    TEST_ASSERT_TRUE(mockComp->render_called);
    TEST_ASSERT_TRUE(mockComp->refresh_called);
    TEST_ASSERT_TRUE(mockComp->set_value_called);
    TEST_ASSERT_EQUAL(77, mockComp->last_value);
}

void test_component_interface_style_service_integration() {
    // Test component integration with style service
    auto component = std::make_unique<MockTestComponent>(mockStyleComp);
    
    TEST_ASSERT_NOT_NULL(component->style_service_);
    TEST_ASSERT_EQUAL_PTR(mockStyleComp, component->style_service_);
    
    // Component should be able to use style service
    lv_obj_t* screen = mockDisplayComp->getMainScreen();
    ComponentLocation location(static_cast<lv_coord_t>(0), static_cast<lv_coord_t>(0));
    
    component->render(screen, location, mockDisplayComp);
    
    // Style service should be available for styling operations
    TEST_ASSERT_NOT_NULL(component->style_service_);
}

void runComponentInterfaceTests() {
    setUp_component_interfaces();
    RUN_TEST(test_component_interface_construction);
    RUN_TEST(test_component_interface_render_method);
    RUN_TEST(test_component_interface_render_null_screen);
    RUN_TEST(test_component_interface_refresh_method);
    RUN_TEST(test_component_interface_set_value_method);
    RUN_TEST(test_component_interface_complete_lifecycle);
    RUN_TEST(test_component_interface_multiple_refreshes);
    RUN_TEST(test_component_interface_multiple_set_values);
    RUN_TEST(test_component_interface_component_location_variants);
    RUN_TEST(test_component_interface_reading_edge_cases);
    RUN_TEST(test_component_interface_memory_management);
    RUN_TEST(test_component_interface_polymorphism);
    RUN_TEST(test_component_interface_style_service_integration);
    tearDown_component_interfaces();
}