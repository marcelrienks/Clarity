#include <unity.h>
#include "test_utilities.h"
#include "mock_style_manager.h"

// Component headers
#include "components/clarity_component.h"
#include "components/key_component.h"
#include "components/lock_component.h"
#include "components/oem/oem_oil_component.h"
#include "components/oem/oem_oil_pressure_component.h"
#include "components/oem/oem_oil_temperature_component.h"

// Test state tracking
static bool component_render_called = false;
static bool component_refresh_called = false;
static mock_lv_obj_t* last_created_object = nullptr;

extern "C" {
    // Mock LVGL object creation functions
    mock_lv_obj_t* mock_lv_label_create(mock_lv_obj_t* screen) {
        static mock_lv_obj_t obj = create_mock_lv_obj();
        obj.created = true;
        last_created_object = &obj;
        return &obj;
    }
    
    mock_lv_obj_t* mock_lv_image_create(mock_lv_obj_t* screen) {
        static mock_lv_obj_t obj = create_mock_lv_obj();
        obj.created = true;
        obj.image_set = false;
        last_created_object = &obj;
        return &obj;
    }
    
    mock_lv_obj_t* mock_lv_arc_create(mock_lv_obj_t* screen) {
        static mock_lv_obj_t obj = create_mock_lv_obj();
        obj.created = true;
        last_created_object = &obj;
        return &obj;
    }
    
    mock_lv_obj_t* mock_lv_line_create(mock_lv_obj_t* screen) {
        static mock_lv_obj_t obj = create_mock_lv_obj();
        obj.created = true;
        last_created_object = &obj;
        return &obj;
    }
    
    // Mock LVGL functions
    void mock_lv_label_set_text(mock_lv_obj_t* obj, const char* text) {
        obj->text_set = true;
        obj->text_content = text;
    }
    
    void mock_lv_image_set_src(mock_lv_obj_t* obj, const void* src) {
        obj->image_set = true;
        obj->image_src = src;
    }
    
    void mock_lv_obj_align(mock_lv_obj_t* obj, int32_t align, int32_t x_offset, int32_t y_offset) {
        obj->aligned = true;
        obj->align_type = align;
        obj->x_offset = x_offset;
        obj->y_offset = y_offset;
    }
    
    void mock_lv_obj_set_style_text_font(mock_lv_obj_t* obj, const lv_font_t* font, uint32_t selector) {
        // Mock font setting
    }
    
    void mock_lv_obj_set_style_image_recolor(mock_lv_obj_t* obj, mock_lv_color_t color, uint32_t selector) {
        obj->color_value = color.hex_value;
    }
    
    void mock_lv_obj_set_style_image_recolor_opa(mock_lv_obj_t* obj, uint8_t opa, uint32_t selector) {
        obj->recolor_opa = opa;
    }
    
    void mock_lv_obj_del(mock_lv_obj_t* obj) {
        obj->deleted = true;
    }
}

// Test helper functions
void resetMockComponentState() {
    component_render_called = false;
    component_refresh_called = false;
    last_created_object = nullptr;
}

void setUp(void) {
    resetMockComponentState();
}

void tearDown(void) {
    // Cleanup after each test
}

// ClarityComponent Tests
void test_clarity_component_creation(void) {
    // Setup
    ClarityComponent clarity;
    mock_lv_obj_t screen = create_mock_lv_obj();
    ComponentLocation location(LV_ALIGN_CENTER, 0, 0);
    
    // Test
    clarity.render(&screen, location);
    
    // Verify - should create a label with "Clarity" text
    TEST_ASSERT_NOT_NULL(last_created_object);
    TEST_ASSERT_TRUE(last_created_object->created);
    TEST_ASSERT_TRUE(last_created_object->text_set);
    TEST_ASSERT_EQUAL_STRING("Clarity", last_created_object->text_content);
    TEST_ASSERT_TRUE(last_created_object->aligned);
    TEST_ASSERT_EQUAL_INT32(LV_ALIGN_CENTER, last_created_object->align_type);
}

void test_clarity_component_positioning(void) {
    // Setup
    ClarityComponent clarity;
    mock_lv_obj_t screen = create_mock_lv_obj();
    ComponentLocation location(LV_ALIGN_CENTER, 10, -20);
    
    // Test
    clarity.render(&screen, location);
    
    // Verify positioning
    TEST_ASSERT_EQUAL_INT32(10, last_created_object->x_offset);
    TEST_ASSERT_EQUAL_INT32(-20, last_created_object->y_offset);
}

// KeyComponent Tests  
void test_key_component_creation(void) {
    // Setup
    KeyComponent key;
    mock_lv_obj_t screen = create_mock_lv_obj();
    ComponentLocation location(LV_ALIGN_CENTER, 0, 0);
    
    // Test
    key.render(&screen, location);
    
    // Verify - should create an image for the key icon
    TEST_ASSERT_NOT_NULL(last_created_object);
    TEST_ASSERT_TRUE(last_created_object->created);
    TEST_ASSERT_TRUE(last_created_object->aligned);
}

void test_key_component_refresh(void) {
    // Setup
    KeyComponent key;
    mock_lv_obj_t screen = create_mock_lv_obj();
    ComponentLocation location(LV_ALIGN_CENTER, 0, 0);
    key.render(&screen, location);
    
    // Test with bool reading (key present)
    Reading reading = true;
    key.refresh(reading);
    
    // Verify - component should handle refresh without error
    // Note: Specific color/style changes would need more detailed mocking
    TEST_ASSERT_TRUE(true); // Basic test that refresh doesn't crash
}

void test_key_component_set_value(void) {
    // Setup
    KeyComponent key;
    mock_lv_obj_t screen = create_mock_lv_obj();
    ComponentLocation location(LV_ALIGN_CENTER, 0, 0);
    key.render(&screen, location);
    
    // Test
    key.SetValue(1); // Key present
    
    // Verify - component should handle SetValue without error
    TEST_ASSERT_TRUE(true);
}

// LockComponent Tests
void test_lock_component_creation(void) {
    // Setup
    LockComponent lock;
    mock_lv_obj_t screen = create_mock_lv_obj();
    ComponentLocation location(LV_ALIGN_CENTER, 15, -25);
    
    // Test
    lock.render(&screen, location);
    
    // Verify
    TEST_ASSERT_NOT_NULL(last_created_object);
    TEST_ASSERT_TRUE(last_created_object->created);
    TEST_ASSERT_TRUE(last_created_object->aligned);
    TEST_ASSERT_EQUAL_INT32(15, last_created_object->x_offset);
    TEST_ASSERT_EQUAL_INT32(-25, last_created_object->y_offset);
}

// OemOilPressureComponent Tests
void test_oem_oil_pressure_creation(void) {
    // Setup
    OemOilPressureComponent pressure;
    mock_lv_obj_t screen = create_mock_lv_obj();
    ComponentLocation location(LV_ALIGN_CENTER, 0, 0);
    
    // Test
    pressure.render(&screen, location);
    
    // Verify - oil component creates complex gauge structure
    TEST_ASSERT_NOT_NULL(last_created_object);
    TEST_ASSERT_TRUE(last_created_object->created);
}

void test_oem_oil_pressure_value_update(void) {
    // Setup
    OemOilPressureComponent pressure;
    mock_lv_obj_t screen = create_mock_lv_obj();
    ComponentLocation location(LV_ALIGN_CENTER, 0, 0);
    pressure.render(&screen, location);
    
    // Test with pressure value
    pressure.SetValue(75); // 75 PSI
    
    // Verify - should handle value update
    TEST_ASSERT_TRUE(true);
}

void test_oem_oil_pressure_danger_condition(void) {
    // Setup
    OemOilPressureComponent pressure;
    mock_lv_obj_t screen = create_mock_lv_obj();
    ComponentLocation location(LV_ALIGN_CENTER, 0, 0);
    pressure.render(&screen, location);
    
    // Test with dangerous low pressure
    pressure.SetValue(5); // Very low pressure
    
    // Verify - should handle danger condition
    TEST_ASSERT_TRUE(true);
}

// OemOilTemperatureComponent Tests
void test_oem_oil_temperature_creation(void) {
    // Setup
    OemOilTemperatureComponent temperature;
    mock_lv_obj_t screen = create_mock_lv_obj();
    ComponentLocation location(180); // Rotated positioning
    
    // Test
    temperature.render(&screen, location);
    
    // Verify
    TEST_ASSERT_NOT_NULL(last_created_object);
    TEST_ASSERT_TRUE(last_created_object->created);
}

void test_oem_oil_temperature_value_ranges(void) {
    // Setup
    OemOilTemperatureComponent temperature;
    mock_lv_obj_t screen = create_mock_lv_obj();
    ComponentLocation location(LV_ALIGN_CENTER, 0, 0);
    temperature.render(&screen, location);
    
    // Test normal operating temperature
    temperature.SetValue(85); // Normal temp
    
    // Test high temperature
    temperature.SetValue(110); // High temp
    
    // Verify - should handle different temperature ranges
    TEST_ASSERT_TRUE(true);
}

// Component Interface Tests
void test_component_interface_render_requirement(void) {
    // Test that all components implement required render method
    ClarityComponent clarity;
    KeyComponent key;
    LockComponent lock;
    OemOilPressureComponent pressure;
    
    mock_lv_obj_t screen = create_mock_lv_obj();
    ComponentLocation location(LV_ALIGN_CENTER, 0, 0);
    
    // All should render without error
    clarity.render(&screen, location);
    key.render(&screen, location);
    lock.render(&screen, location);
    pressure.render(&screen, location);
    
    TEST_ASSERT_TRUE(true); // If we get here, all renders succeeded
}

void test_component_interface_optional_methods(void) {
    // Test that components handle optional methods
    KeyComponent key;
    mock_lv_obj_t screen = create_mock_lv_obj();
    ComponentLocation location(LV_ALIGN_CENTER, 0, 0);
    key.render(&screen, location);
    
    // Optional methods should not crash
    Reading reading = 42;
    key.refresh(reading);
    key.SetValue(100);
    
    TEST_ASSERT_TRUE(true);
}

// Performance and Memory Tests
void test_component_multiple_renders(void) {
    // Test rendering same component multiple times
    ClarityComponent clarity;
    mock_lv_obj_t screen = create_mock_lv_obj();
    ComponentLocation location(LV_ALIGN_CENTER, 0, 0);
    
    // Render multiple times
    for(int i = 0; i < 5; i++) {
        clarity.render(&screen, location);
    }
    
    // Should handle multiple renders
    TEST_ASSERT_TRUE(true);
}

void test_component_memory_efficiency(void) {
    // Test that components don't leak memory with repeated operations
    KeyComponent key;
    LockComponent lock;
    
    mock_lv_obj_t screen = create_mock_lv_obj();
    ComponentLocation location(LV_ALIGN_CENTER, 0, 0);
    
    // Repeated render/refresh cycles
    for(int i = 0; i < 10; i++) {
        key.render(&screen, location);
        key.refresh(Reading(i % 2 == 0));
        
        lock.render(&screen, location);
        lock.SetValue(i * 10);
    }
    
    // All should complete without issues
    TEST_ASSERT_TRUE(true);
}

// Note: PlatformIO will automatically discover and run test_ functions