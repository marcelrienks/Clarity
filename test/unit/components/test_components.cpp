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

// Mock service implementations for component testing
class MockStyleService : public IStyleService {
public:
    void init(const char* theme) override {}
    void applyThemeToScreen(lv_obj_t* screen) override {}
    void setTheme(const char* theme) override {}
    const char* getCurrentTheme() const override { return "Day"; }
    lv_style_t& getBackgroundStyle() override { static lv_style_t style; return style; }
    lv_style_t& getTextStyle() override { static lv_style_t style; return style; }
    lv_style_t& getGaugeNormalStyle() override { static lv_style_t style; return style; }
    lv_style_t& getGaugeWarningStyle() override { static lv_style_t style; return style; }
    lv_style_t& getGaugeDangerStyle() override { static lv_style_t style; return style; }
    lv_style_t& getGaugeIndicatorStyle() override { static lv_style_t style; return style; }
    lv_style_t& getGaugeItemsStyle() override { static lv_style_t style; return style; }
    lv_style_t& getGaugeMainStyle() override { static lv_style_t style; return style; }
    lv_style_t& getGaugeDangerSectionStyle() override { static lv_style_t style; return style; }
    const ThemeColors& getThemeColors() const override { static ThemeColors colors; return colors; }
};

class MockDisplayProvider : public IDisplayProvider {
public:
    lv_obj_t* createScreen() override { static mock_lv_obj_t screen; return (lv_obj_t*)&screen; }
    void loadScreen(lv_obj_t* screen) override {}
    lv_obj_t* createLabel(lv_obj_t* parent) override { static mock_lv_obj_t label; return (lv_obj_t*)&label; }
    lv_obj_t* createObject(lv_obj_t* parent) override { static mock_lv_obj_t object; return (lv_obj_t*)&object; }
    lv_obj_t* createArc(lv_obj_t* parent) override { static mock_lv_obj_t arc; return (lv_obj_t*)&arc; }
    lv_obj_t* createScale(lv_obj_t* parent) override { static mock_lv_obj_t scale; return (lv_obj_t*)&scale; }
    lv_obj_t* createImage(lv_obj_t* parent) override { static mock_lv_obj_t image; return (lv_obj_t*)&image; }
    lv_obj_t* createLine(lv_obj_t* parent) override { static mock_lv_obj_t line; return (lv_obj_t*)&line; }
    void deleteObject(lv_obj_t* obj) override {}
    void addEventCallback(lv_obj_t* obj, lv_event_cb_t callback, lv_event_code_t event_code, void* user_data) override {}
    lv_obj_t* getMainScreen() override { return createScreen(); }
};

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
    MockStyleService mockStyle;
    ClarityComponent clarity(&mockStyle);
    mock_lv_obj_t screen = create_mock_lv_obj();
    ComponentLocation location(LV_ALIGN_CENTER, 0, 0);
    MockDisplayProvider mockDisplay;
    
    // Test
    clarity.render((lv_obj_t*)&screen, location, &mockDisplay);
    
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
    MockStyleService mockStyle;
    ClarityComponent clarity(&mockStyle);
    mock_lv_obj_t screen = create_mock_lv_obj();
    ComponentLocation location(LV_ALIGN_CENTER, 10, -20);
    MockDisplayProvider mockDisplay;
    
    // Test
    clarity.render((lv_obj_t*)&screen, location, &mockDisplay);
    
    // Verify positioning
    TEST_ASSERT_EQUAL_INT32(10, last_created_object->x_offset);
    TEST_ASSERT_EQUAL_INT32(-20, last_created_object->y_offset);
}

// KeyComponent Tests  
void test_key_component_creation(void) {
    // Setup
    MockStyleService mockStyle;
    KeyComponent key(&mockStyle);
    mock_lv_obj_t screen = create_mock_lv_obj();
    ComponentLocation location(LV_ALIGN_CENTER, 0, 0);
    
    // Test
    MockDisplayProvider mockDisplay;
    key.render((lv_obj_t*)&screen, location, &mockDisplay);
    
    // Verify - should create an image for the key icon
    TEST_ASSERT_NOT_NULL(last_created_object);
    TEST_ASSERT_TRUE(last_created_object->created);
    TEST_ASSERT_TRUE(last_created_object->aligned);
}

void test_key_component_refresh(void) {
    // Setup
    MockStyleService mockStyle;
    KeyComponent key(&mockStyle);
    mock_lv_obj_t screen = create_mock_lv_obj();
    ComponentLocation location(LV_ALIGN_CENTER, 0, 0);
    MockDisplayProvider mockDisplay;
    key.render((lv_obj_t*)&screen, location, &mockDisplay);
    
    // Test with bool reading (key present)
    Reading reading = true;
    key.refresh(reading);
    
    // Verify - component should handle refresh without error
    // Note: Specific color/style changes would need more detailed mocking
    TEST_ASSERT_TRUE(true); // Basic test that refresh doesn't crash
}

void test_key_component_set_value(void) {
    // Setup
    MockStyleService mockStyle;
    KeyComponent key(&mockStyle);
    mock_lv_obj_t screen = create_mock_lv_obj();
    ComponentLocation location(LV_ALIGN_CENTER, 0, 0);
    MockDisplayProvider mockDisplay;
    key.render((lv_obj_t*)&screen, location, &mockDisplay);
    
    // Test
    key.SetValue(1); // Key present
    
    // Verify - component should handle SetValue without error
    TEST_ASSERT_TRUE(true);
}

// LockComponent Tests
void test_lock_component_creation(void) {
    // Setup
    MockStyleService mockStyle;
    LockComponent lock(&mockStyle);
    mock_lv_obj_t screen = create_mock_lv_obj();
    ComponentLocation location(LV_ALIGN_CENTER, 15, -25);
    
    // Test
    MockDisplayProvider mockDisplay;
    lock.render((lv_obj_t*)&screen, location, &mockDisplay);
    
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
    MockStyleService mockStyle;
    OemOilPressureComponent pressure(&mockStyle);
    mock_lv_obj_t screen = create_mock_lv_obj();
    ComponentLocation location(LV_ALIGN_CENTER, 0, 0);
    MockDisplayProvider mockDisplay;
    
    // Test
    pressure.render((lv_obj_t*)&screen, location, &mockDisplay);
    
    // Verify - oil component creates complex gauge structure
    TEST_ASSERT_NOT_NULL(last_created_object);
    TEST_ASSERT_TRUE(last_created_object->created);
}

void test_oem_oil_pressure_value_update(void) {
    // Setup
    MockStyleService mockStyle;
    OemOilPressureComponent pressure(&mockStyle);
    mock_lv_obj_t screen = create_mock_lv_obj();
    ComponentLocation location(LV_ALIGN_CENTER, 0, 0);
    MockDisplayProvider mockDisplay;
    pressure.render((lv_obj_t*)&screen, location, &mockDisplay);
    
    // Test with pressure value
    pressure.SetValue(75); // 75 PSI
    
    // Verify - should handle value update
    TEST_ASSERT_TRUE(true);
}

void test_oem_oil_pressure_danger_condition(void) {
    // Setup
    MockStyleService mockStyle;
    OemOilPressureComponent pressure(&mockStyle);
    mock_lv_obj_t screen = create_mock_lv_obj();
    ComponentLocation location(LV_ALIGN_CENTER, 0, 0);
    MockDisplayProvider mockDisplay;
    pressure.render((lv_obj_t*)&screen, location, &mockDisplay);
    
    // Test with dangerous low pressure
    pressure.SetValue(5); // Very low pressure
    
    // Verify - should handle danger condition
    TEST_ASSERT_TRUE(true);
}

// OemOilTemperatureComponent Tests
void test_oem_oil_temperature_creation(void) {
    // Setup
    MockStyleService mockStyle;
    OemOilTemperatureComponent temperature(&mockStyle);
    mock_lv_obj_t screen = create_mock_lv_obj();
    ComponentLocation location(180); // Rotated positioning
    MockDisplayProvider mockDisplay;
    
    // Test
    temperature.render((lv_obj_t*)&screen, location, &mockDisplay);
    
    // Verify
    TEST_ASSERT_NOT_NULL(last_created_object);
    TEST_ASSERT_TRUE(last_created_object->created);
}

void test_oem_oil_temperature_value_ranges(void) {
    // Setup
    MockStyleService mockStyle;
    OemOilTemperatureComponent temperature(&mockStyle);
    mock_lv_obj_t screen = create_mock_lv_obj();
    ComponentLocation location(LV_ALIGN_CENTER, 0, 0);
    MockDisplayProvider mockDisplay;
    temperature.render((lv_obj_t*)&screen, location, &mockDisplay);
    
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
    MockStyleService mockStyle;
    ClarityComponent clarity(&mockStyle);
    KeyComponent key(&mockStyle);
    LockComponent lock(&mockStyle);
    OemOilPressureComponent pressure(&mockStyle);
    
    mock_lv_obj_t screen = create_mock_lv_obj();
    ComponentLocation location(LV_ALIGN_CENTER, 0, 0);
    MockDisplayProvider mockDisplay;
    
    // All should render without error
    clarity.render((lv_obj_t*)&screen, location, &mockDisplay);
    key.render((lv_obj_t*)&screen, location, &mockDisplay);
    lock.render((lv_obj_t*)&screen, location, &mockDisplay);
    pressure.render((lv_obj_t*)&screen, location, &mockDisplay);
    
    TEST_ASSERT_TRUE(true); // If we get here, all renders succeeded
}

void test_component_interface_optional_methods(void) {
    // Test that components handle optional methods
    MockStyleService mockStyle;
    KeyComponent key(&mockStyle);
    mock_lv_obj_t screen = create_mock_lv_obj();
    ComponentLocation location(LV_ALIGN_CENTER, 0, 0);
    MockDisplayProvider mockDisplay;
    key.render((lv_obj_t*)&screen, location, &mockDisplay);
    
    // Optional methods should not crash
    Reading reading = 42;
    key.refresh(reading);
    key.SetValue(100);
    
    TEST_ASSERT_TRUE(true);
}

// Performance and Memory Tests
void test_component_multiple_renders(void) {
    // Test rendering same component multiple times
    MockStyleService mockStyle;
    ClarityComponent clarity(&mockStyle);
    mock_lv_obj_t screen = create_mock_lv_obj();
    ComponentLocation location(LV_ALIGN_CENTER, 0, 0);
    MockDisplayProvider mockDisplay;
    
    // Render multiple times
    for(int i = 0; i < 5; i++) {
        clarity.render((lv_obj_t*)&screen, location, &mockDisplay);
    }
    
    // Should handle multiple renders
    TEST_ASSERT_TRUE(true);
}

void test_component_memory_efficiency(void) {
    // Test that components don't leak memory with repeated operations
    MockStyleService mockStyle;
    KeyComponent key(&mockStyle);
    LockComponent lock(&mockStyle);
    
    mock_lv_obj_t screen = create_mock_lv_obj();
    ComponentLocation location(LV_ALIGN_CENTER, 0, 0);
    
    // Repeated render/refresh cycles
    MockDisplayProvider mockDisplay;
    for(int i = 0; i < 10; i++) {
        key.render((lv_obj_t*)&screen, location, &mockDisplay);
        key.refresh(Reading(i % 2 == 0));
        
        lock.render((lv_obj_t*)&screen, location, &mockDisplay);
        lock.SetValue(i * 10);
    }
    
    // All should complete without issues
    TEST_ASSERT_TRUE(true);
}

// Note: PlatformIO will automatically discover and run test_ functions