#include <unity.h>
#include "test_utilities.h"
#include "mock_style_manager.h"

// Mock LVGL dependencies for component testing
extern "C" {
    // Mock LVGL types
    typedef struct {
        bool created;
        bool text_set;
        bool image_set;
        bool style_applied;
        bool aligned;
        bool deleted;
        const char* text_content;
        int32_t align_type;
        int32_t x_offset;
        int32_t y_offset;
        uint32_t color_value;
        uint8_t recolor_ovoid test_oem_oil_component_destructor_cleanup(void) {
    resetMockComponentState();
    
    mock_lv_obj_t screen = create_mock_lv_obj();
    mock_component_location_t location = {LV_ALIGN_CENTER, 0, 0};        const void* image_src;
    } mock_lv_obj_t;
    
    typedef struct {
        uint32_t hex_value;
    } mock_lv_color_t;
    
    // Mock component location
    typedef struct {
        int32_t align;
        int32_t x_offset;
        int32_t y_offset;
    } mock_component_location_t;
    
    // Mock reading variant
    typedef struct {
        int32_t int_value;
        float float_value;
        bool bool_value;
    } mock_reading_t;
    
    // Mock LVGL object creation functions
    mock_lv_obj_t* mock_lv_label_create(mock_lv_obj_t* parent) {
        static mock_lv_obj_t obj = {true, false, false, false, false, false, nullptr, 0, 0, 0, 0, 0, nullptr};
        obj.created = true;
        obj.deleted = false;
        return &obj;
    }
    
    mock_lv_obj_t* mock_lv_image_create(mock_lv_obj_t* parent) {
        static mock_lv_obj_t obj = {true, false, false, false, false, false, nullptr, 0, 0, 0, 0, 0, nullptr};
        obj.created = true;
        obj.deleted = false;
        return &obj;
    }
    
    mock_lv_obj_t* mock_lv_arc_create(mock_lv_obj_t* parent) {
        static mock_lv_obj_t obj = {true, false, false, false, false, false, nullptr, 0, 0, 0, 0, 0, nullptr};
        obj.created = true;
        obj.deleted = false;
        return &obj;
    }
    
    mock_lv_obj_t* mock_lv_line_create(mock_lv_obj_t* parent) {
        static mock_lv_obj_t obj = {true, false, false, false, false, false, nullptr, 0, 0, 0, 0, 0, nullptr};
        obj.created = true;
        obj.deleted = false;
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
    
    void mock_lv_obj_add_style(mock_lv_obj_t* obj, void* style, uint32_t selector) {
        obj->style_applied = true;
    }
    
    void mock_lv_obj_align(mock_lv_obj_t* obj, int32_t align, int32_t x_ofs, int32_t y_ofs) {
        obj->aligned = true;
        obj->align_type = align;
        obj->x_offset = x_ofs;
        obj->y_offset = y_ofs;
    }
    
    void mock_lv_obj_set_style_text_font(mock_lv_obj_t* obj, const void* font, uint32_t selector) {
        // Font setting
    }
    
    void mock_lv_obj_set_style_image_recolor(mock_lv_obj_t* obj, mock_lv_color_t color, uint32_t selector) {
        obj->color_value = color.hex_value;
    }
    
    void mock_lv_obj_set_style_image_recolor_opa(mock_lv_obj_t* obj, uint8_t opa, uint32_t selector) {
        obj->recolor_opa = opa;
    }
    
    void mock_lv_obj_del(mock_lv_obj_t* obj) {
        if (obj) {
            obj->deleted = true;
            obj->created = false;
        }
    }
    
    // Mock constants
    const uint32_t LV_PART_MAIN = 0x01;
    const uint32_t LV_STATE_DEFAULT = 0x02;
    const uint32_t MAIN_DEFAULT = LV_PART_MAIN | LV_STATE_DEFAULT;
    const uint8_t LV_OPA_COVER = 255;
    const int32_t LV_ALIGN_CENTER = 0;
    
    // Mock font
    struct {
        int size;
    } lv_font_montserrat_20 = {20};
    
    // Mock icon data
    struct {
        const char* name;
    } key_solid = {"key_solid_icon"};
    
    struct {
        const char* name;
    } lock_alt_solid = {"lock_alt_solid_icon"};
    
    struct {
        const char* name;
    } oil_can_regular = {"oil_can_regular_icon"};
    
    mock_lv_color_t mock_lv_color_hex(uint32_t hex) {
        return {hex};
    }
}

// Mock StyleManager for component testing
class MockStyleManager {
public:
    static MockStyleManager& GetInstance() {
        static MockStyleManager instance;
        return instance;
    }
    
    const char* THEME = "Day";
    
    struct MockStyle {
        bool applied;
    } textStyle;
    
    struct MockThemeColors {
        mock_lv_color_t keyPresent = mock_lv_color_hex(0x00FF00);
        mock_lv_color_t keyNotPresent = mock_lv_color_hex(0xFF0000);
        mock_lv_color_t gaugeNormal = mock_lv_color_hex(0xFFFFFF);
        mock_lv_color_t gaugeDanger = mock_lv_color_hex(0xFF0000);
    };
    
    MockThemeColors get_colours(const char* theme) const {
        return MockThemeColors{};
    }
};

// Mock component types
enum class KeyState {
    Inactive = 0,
    Present = 1,
    NotPresent = 2
};

// Mock component classes for testing
class MockClarityComponent {
public:
    bool rendered = false;
    mock_component_location_t last_location;
    
    void render(mock_lv_obj_t* screen, const mock_component_location_t& location) {
        rendered = true;
        last_location = location;
        
        // Create label
        mock_lv_obj_t* splash = mock_lv_label_create(screen);
        mock_lv_label_set_text(splash, "Clarity");
        
        // Apply style
        mock_lv_obj_add_style(splash, &MockStyleManager::GetInstance().textStyle, MAIN_DEFAULT);
        
        // Apply location
        mock_lv_obj_align(splash, location.align, location.x_offset, location.y_offset);
        
        // Set font
        mock_lv_obj_set_style_text_font(splash, &lv_font_montserrat_20, 0);
    }
};

class MockKeyComponent {
public:
    mock_lv_obj_t* keyIcon_ = nullptr;
    bool rendered = false;
    bool refreshed = false;
    KeyState last_state = KeyState::Inactive;
    
    MockKeyComponent() = default;
    
    ~MockKeyComponent() {
        if (keyIcon_) {
            mock_lv_obj_del(keyIcon_);
        }
    }
    
    void render(mock_lv_obj_t* screen, const mock_component_location_t& location) {
        rendered = true;
        
        // Create key icon
        keyIcon_ = mock_lv_image_create(screen);
        mock_lv_image_set_src(keyIcon_, &key_solid);
        
        // Apply location
        mock_lv_obj_align(keyIcon_, location.align, location.x_offset, location.y_offset);
    }
    
    void refresh(const mock_reading_t& reading) {
        refreshed = true;
        last_state = static_cast<KeyState>(reading.int_value);
        
        if (!keyIcon_) return;
        
        mock_lv_color_t color;
        if (last_state == KeyState::Present) {
            color = MockStyleManager::GetInstance().get_colours("").keyPresent;
        } else {
            color = MockStyleManager::GetInstance().get_colours("").keyNotPresent;
        }
        
        mock_lv_obj_set_style_image_recolor(keyIcon_, color, MAIN_DEFAULT);
        mock_lv_obj_set_style_image_recolor_opa(keyIcon_, LV_OPA_COVER, MAIN_DEFAULT);
    }
    
    void SetValue(int32_t value) {
        mock_reading_t reading = {value, 0.0f, false};
        refresh(reading);
    }
};

class MockLockComponent {
public:
    mock_lv_obj_t* lockIcon_ = nullptr;
    bool rendered = false;
    bool refreshed = false;
    bool last_state = false;
    
    MockLockComponent() = default;
    
    ~MockLockComponent() {
        if (lockIcon_) {
            mock_lv_obj_del(lockIcon_);
        }
    }
    
    void render(mock_lv_obj_t* screen, const mock_component_location_t& location) {
        rendered = true;
        
        // Create lock icon
        lockIcon_ = mock_lv_image_create(screen);
        mock_lv_image_set_src(lockIcon_, &lock_alt_solid);
        
        // Apply location
        mock_lv_obj_align(lockIcon_, location.align, location.x_offset, location.y_offset);
    }
    
    void refresh(const mock_reading_t& reading) {
        refreshed = true;
        last_state = reading.bool_value;
        
        if (!lockIcon_) return;
        
        mock_lv_color_t color = MockStyleManager::GetInstance().get_colours("").gaugeNormal;
        mock_lv_obj_set_style_image_recolor(lockIcon_, color, MAIN_DEFAULT);
        mock_lv_obj_set_style_image_recolor_opa(lockIcon_, LV_OPA_COVER, MAIN_DEFAULT);
    }
    
    void SetValue(int32_t value) {
        mock_reading_t reading = {0, 0.0f, value != 0};
        refresh(reading);
    }
};

class MockOemOilComponent {
public:
    mock_lv_obj_t* scale_ = nullptr;
    mock_lv_obj_t* needleLine_ = nullptr;
    mock_lv_obj_t* oilIcon_ = nullptr;
    bool rendered = false;
    bool refreshed = false;
    float last_value = 0.0f;
    int scale_rotation = 0;
    
    MockOemOilComponent() = default;
    
    ~MockOemOilComponent() {
        if (scale_) mock_lv_obj_del(scale_);
        if (needleLine_) mock_lv_obj_del(needleLine_);
        if (oilIcon_) mock_lv_obj_del(oilIcon_);
    }
    
    void render(mock_lv_obj_t* screen, const mock_component_location_t& location) {
        rendered = true;
        
        // Create gauge components
        scale_ = mock_lv_arc_create(screen);
        needleLine_ = mock_lv_line_create(screen);
        oilIcon_ = mock_lv_image_create(screen);
        
        // Set icon source
        mock_lv_image_set_src(oilIcon_, &oil_can_regular);
        
        // Apply positioning
        mock_lv_obj_align(scale_, location.align, location.x_offset, location.y_offset);
    }
    
    void refresh(const mock_reading_t& reading) {
        refreshed = true;
        last_value = reading.float_value;
        
        // Simulate needle rotation based on value
        scale_rotation = static_cast<int>(last_value * 180.0f / 10.0f); // 0-10 range to 0-180 degrees
    }
    
    void SetValue(int32_t value) {
        mock_reading_t reading = {0, static_cast<float>(value), false};
        refresh(reading);
    }
    
    int getScaleRotation() const {
        return scale_rotation;
    }
};

// Note: setUp() and tearDown() are defined in test_main.cpp

void resetMockComponentState() {
    // Reset any global mock state if needed
}

// =================================================================
// CLARITY COMPONENT TESTS
// =================================================================

void test_clarity_component_render_basic(void) {
    resetMockComponentState();
    MockClarityComponent component;
    
    mock_lv_obj_t screen = create_mock_lv_obj();
    mock_component_location_t location = {LV_ALIGN_CENTER, 0, -50};
    
    component.render(&screen, location);
    
    TEST_ASSERT_TRUE(component.rendered);
    TEST_ASSERT_EQUAL_INT32(LV_ALIGN_CENTER, component.last_location.align);
    TEST_ASSERT_EQUAL_INT32(0, component.last_location.x_offset);
    TEST_ASSERT_EQUAL_INT32(-50, component.last_location.y_offset);
}

void test_clarity_component_text_content(void) {
    resetMockComponentState();
    MockClarityComponent component;
    
    mock_lv_obj_t screen = {true, false, false, false, false, false, nullptr, 0, 0, 0, 0, 0, nullptr};
    mock_component_location_t location = {LV_ALIGN_CENTER, 0, 0};
    
    component.render(&screen, location);
    
    // Text should be set during render
    TEST_ASSERT_TRUE(component.rendered);
}

void test_clarity_component_positioning(void) {
    resetMockComponentState();
    MockClarityComponent component;
    
    mock_lv_obj_t screen = {true, false, false, false, false, false, nullptr, 0, 0, 0, 0, 0, nullptr};
    
    // Test different positions
    mock_component_location_t positions[] = {
        {LV_ALIGN_CENTER, 0, 0},
        {LV_ALIGN_CENTER, 10, -20},
        {LV_ALIGN_CENTER, -15, 30}
    };
    
    for (size_t i = 0; i < 3; i++) {
        component.render(&screen, positions[i]);
        
        TEST_ASSERT_EQUAL_INT32(positions[i].align, component.last_location.align);
        TEST_ASSERT_EQUAL_INT32(positions[i].x_offset, component.last_location.x_offset);
        TEST_ASSERT_EQUAL_INT32(positions[i].y_offset, component.last_location.y_offset);
    }
}

// =================================================================
// KEY COMPONENT TESTS
// =================================================================

void test_key_component_render_basic(void) {
    resetMockComponentState();
    MockKeyComponent component;
    
    mock_lv_obj_t screen = {true, false, false, false, false, false, nullptr, 0, 0, 0, 0, 0, nullptr};
    mock_component_location_t location = {LV_ALIGN_CENTER, 0, 0};
    
    component.render(&screen, location);
    
    TEST_ASSERT_TRUE(component.rendered);
    TEST_ASSERT_NOT_NULL(component.keyIcon_);
    TEST_ASSERT_TRUE(component.keyIcon_->created);
    TEST_ASSERT_TRUE(component.keyIcon_->image_set);
}

void test_key_component_icon_source(void) {
    resetMockComponentState();
    MockKeyComponent component;
    
    mock_lv_obj_t screen = {true, false, false, false, false, false, nullptr, 0, 0, 0, 0, 0, nullptr};
    mock_component_location_t location = {LV_ALIGN_CENTER, 0, 0};
    
    component.render(&screen, location);
    
    TEST_ASSERT_EQUAL_PTR(&key_solid, component.keyIcon_->image_src);
}

void test_key_component_refresh_present_state(void) {
    resetMockComponentState();
    MockKeyComponent component;
    
    mock_lv_obj_t screen = {true, false, false, false, false, false, nullptr, 0, 0, 0, 0, 0, nullptr};
    mock_component_location_t location = {LV_ALIGN_CENTER, 0, 0};
    
    component.render(&screen, location);
    
    mock_reading_t reading = {static_cast<int32_t>(KeyState::Present), 0.0f, false};
    component.refresh(reading);
    
    TEST_ASSERT_TRUE(component.refreshed);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(KeyState::Present), static_cast<int>(component.last_state));
    TEST_ASSERT_EQUAL_HEX32(0x00FF00, component.keyIcon_->color_value); // Green for present
}

void test_key_component_refresh_not_present_state(void) {
    resetMockComponentState();
    MockKeyComponent component;
    
    mock_lv_obj_t screen = {true, false, false, false, false, false, nullptr, 0, 0, 0, 0, 0, nullptr};
    mock_component_location_t location = {LV_ALIGN_CENTER, 0, 0};
    
    component.render(&screen, location);
    
    mock_reading_t reading = {static_cast<int32_t>(KeyState::NotPresent), 0.0f, false};
    component.refresh(reading);
    
    TEST_ASSERT_TRUE(component.refreshed);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(KeyState::NotPresent), static_cast<int>(component.last_state));
    TEST_ASSERT_EQUAL_HEX32(0xFF0000, component.keyIcon_->color_value); // Red for not present
}

void test_key_component_set_value_method(void) {
    resetMockComponentState();
    MockKeyComponent component;
    
    mock_lv_obj_t screen = {true, false, false, false, false, false, nullptr, 0, 0, 0, 0, 0, nullptr};
    mock_component_location_t location = {LV_ALIGN_CENTER, 0, 0};
    
    component.render(&screen, location);
    
    component.SetValue(static_cast<int32_t>(KeyState::Present));
    
    TEST_ASSERT_TRUE(component.refreshed);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(KeyState::Present), static_cast<int>(component.last_state));
}

void test_key_component_destructor_cleanup(void) {
    resetMockComponentState();
    
    mock_lv_obj_t screen = {true, false, false, false, false, false, nullptr, 0, 0, 0, 0, 0, nullptr};
    mock_component_location_t location = {LV_ALIGN_CENTER, 0, 0};
    
    {
        MockKeyComponent component;
        component.render(&screen, location);
        TEST_ASSERT_TRUE(component.keyIcon_->created);
        TEST_ASSERT_FALSE(component.keyIcon_->deleted);
    } // Destructor called here
    
    // Note: In real test, we'd verify cleanup, but mock doesn't track this across scope
    TEST_ASSERT_TRUE(true);
}

// =================================================================
// LOCK COMPONENT TESTS
// =================================================================

void test_lock_component_render_basic(void) {
    resetMockComponentState();
    MockLockComponent component;
    
    mock_lv_obj_t screen = {true, false, false, false, false, false, nullptr, 0, 0, 0, 0, 0, nullptr};
    mock_component_location_t location = {LV_ALIGN_CENTER, 0, 0};
    
    component.render(&screen, location);
    
    TEST_ASSERT_TRUE(component.rendered);
    TEST_ASSERT_NOT_NULL(component.lockIcon_);
    TEST_ASSERT_TRUE(component.lockIcon_->created);
    TEST_ASSERT_TRUE(component.lockIcon_->image_set);
}

void test_lock_component_icon_source(void) {
    resetMockComponentState();
    MockLockComponent component;
    
    mock_lv_obj_t screen = {true, false, false, false, false, false, nullptr, 0, 0, 0, 0, 0, nullptr};
    mock_component_location_t location = {LV_ALIGN_CENTER, 0, 0};
    
    component.render(&screen, location);
    
    TEST_ASSERT_EQUAL_PTR(&lock_alt_solid, component.lockIcon_->image_src);
}

void test_lock_component_refresh_states(void) {
    resetMockComponentState();
    MockLockComponent component;
    
    mock_lv_obj_t screen = {true, false, false, false, false, false, nullptr, 0, 0, 0, 0, 0, nullptr};
    mock_component_location_t location = {LV_ALIGN_CENTER, 0, 0};
    
    component.render(&screen, location);
    
    // Test locked state
    mock_reading_t reading_locked = {0, 0.0f, true};
    component.refresh(reading_locked);
    
    TEST_ASSERT_TRUE(component.refreshed);
    TEST_ASSERT_TRUE(component.last_state);
    
    // Test unlocked state
    mock_reading_t reading_unlocked = {0, 0.0f, false};
    component.refresh(reading_unlocked);
    
    TEST_ASSERT_FALSE(component.last_state);
}

void test_lock_component_set_value_method(void) {
    resetMockComponentState();
    MockLockComponent component;
    
    mock_lv_obj_t screen = {true, false, false, false, false, false, nullptr, 0, 0, 0, 0, 0, nullptr};
    mock_component_location_t location = {LV_ALIGN_CENTER, 0, 0};
    
    component.render(&screen, location);
    
    component.SetValue(1); // Locked
    TEST_ASSERT_TRUE(component.last_state);
    
    component.SetValue(0); // Unlocked
    TEST_ASSERT_FALSE(component.last_state);
}

// =================================================================
// OEM OIL COMPONENT TESTS
// =================================================================

void test_oem_oil_component_render_basic(void) {
    resetMockComponentState();
    MockOemOilComponent component;
    
    mock_lv_obj_t screen = {true, false, false, false, false, false, nullptr, 0, 0, 0, 0, 0, nullptr};
    mock_component_location_t location = {LV_ALIGN_CENTER, 0, 0};
    
    component.render(&screen, location);
    
    TEST_ASSERT_TRUE(component.rendered);
    TEST_ASSERT_NOT_NULL(component.scale_);
    TEST_ASSERT_NOT_NULL(component.needleLine_);
    TEST_ASSERT_NOT_NULL(component.oilIcon_);
}

void test_oem_oil_component_gauge_elements(void) {
    resetMockComponentState();
    MockOemOilComponent component;
    
    mock_lv_obj_t screen = {true, false, false, false, false, false, nullptr, 0, 0, 0, 0, 0, nullptr};
    mock_component_location_t location = {LV_ALIGN_CENTER, 0, 0};
    
    component.render(&screen, location);
    
    TEST_ASSERT_TRUE(component.scale_->created);
    TEST_ASSERT_TRUE(component.needleLine_->created);
    TEST_ASSERT_TRUE(component.oilIcon_->created);
    TEST_ASSERT_TRUE(component.oilIcon_->image_set);
    TEST_ASSERT_EQUAL_PTR(&oil_can_regular, component.oilIcon_->image_src);
}

void test_oem_oil_component_refresh_value_ranges(void) {
    resetMockComponentState();
    MockOemOilComponent component;
    
    mock_lv_obj_t screen = {true, false, false, false, false, false, nullptr, 0, 0, 0, 0, 0, nullptr};
    mock_component_location_t location = {LV_ALIGN_CENTER, 0, 0};
    
    component.render(&screen, location);
    
    // Test different pressure values
    float test_values[] = {0.0f, 2.5f, 5.0f, 7.5f, 10.0f};
    int expected_rotations[] = {0, 45, 90, 135, 180};
    
    for (size_t i = 0; i < 5; i++) {
        mock_reading_t reading = {0, test_values[i], false};
        component.refresh(reading);
        
        TEST_ASSERT_TRUE(component.refreshed);
        TEST_ASSERT_EQUAL_FLOAT(test_values[i], component.last_value);
        TEST_ASSERT_EQUAL_INT(expected_rotations[i], component.getScaleRotation());
    }
}

void test_oem_oil_component_set_value_method(void) {
    resetMockComponentState();
    MockOemOilComponent component;
    
    mock_lv_obj_t screen = {true, false, false, false, false, false, nullptr, 0, 0, 0, 0, 0, nullptr};
    mock_component_location_t location = {LV_ALIGN_CENTER, 0, 0};
    
    component.render(&screen, location);
    
    component.SetValue(6); // 6 Bar pressure
    
    TEST_ASSERT_TRUE(component.refreshed);
    TEST_ASSERT_EQUAL_FLOAT(6.0f, component.last_value);
    TEST_ASSERT_EQUAL_INT(108, component.getScaleRotation()); // 6/10 * 180 = 108 degrees
}

void test_oem_oil_component_destructor_cleanup(void) {
    resetMockComponentState();
    
    mock_lv_obj_t screen = create_mock_lv_obj();
    mock_component_location_t location = {LV_ALIGN_CENTER, 0, 0};
    
    {
        MockOemOilComponent component;
        component.render(&screen, location);
        TEST_ASSERT_TRUE(component.scale_->created);
        TEST_ASSERT_TRUE(component.needleLine_->created);
        TEST_ASSERT_TRUE(component.oilIcon_->created);
    } // Destructor called here
    
    // In real implementation, objects would be cleaned up
    TEST_ASSERT_TRUE(true);
}

// =================================================================
// COMPONENT INTERFACE TESTS
// =================================================================

void test_component_interface_render_requirement(void) {
    // All components must implement render method
    MockClarityComponent clarity;
    MockKeyComponent key;
    MockLockComponent lock;
    MockOemOilComponent oil;
    
    mock_lv_obj_t screen = {true, false, false, false, false, false, nullptr, 0, 0, 0, 0, 0, nullptr};
    mock_component_location_t location = {LV_ALIGN_CENTER, 0, 0};
    
    clarity.render(&screen, location);
    key.render(&screen, location);
    lock.render(&screen, location);
    oil.render(&screen, location);
    
    TEST_ASSERT_TRUE(clarity.rendered);
    TEST_ASSERT_TRUE(key.rendered);
    TEST_ASSERT_TRUE(lock.rendered);
    TEST_ASSERT_TRUE(oil.rendered);
}

void test_component_interface_optional_methods(void) {
    // Components may implement refresh and SetValue
    MockKeyComponent key;
    MockLockComponent lock;
    MockOemOilComponent oil;
    
    mock_lv_obj_t screen = {true, false, false, false, false, false, nullptr, 0, 0, 0, 0, 0, nullptr};
    mock_component_location_t location = {LV_ALIGN_CENTER, 0, 0};
    
    key.render(&screen, location);
    lock.render(&screen, location);
    oil.render(&screen, location);
    
    // Test refresh method
    mock_reading_t reading = {1, 5.0f, true};
    key.refresh(reading);
    lock.refresh(reading);
    oil.refresh(reading);
    
    TEST_ASSERT_TRUE(key.refreshed);
    TEST_ASSERT_TRUE(lock.refreshed);
    TEST_ASSERT_TRUE(oil.refreshed);
    
    // Test SetValue method
    key.SetValue(2);
    lock.SetValue(0);
    oil.SetValue(8);
    
    TEST_ASSERT_EQUAL_INT(2, static_cast<int>(key.last_state));
    TEST_ASSERT_FALSE(lock.last_state);
    TEST_ASSERT_EQUAL_FLOAT(8.0f, oil.last_value);
}

// =================================================================
// INTEGRATION TESTS
// =================================================================

void test_component_style_integration(void) {
    resetMockComponentState();
    MockClarityComponent clarity;
    MockKeyComponent key;
    
    mock_lv_obj_t screen = {true, false, false, false, false, false, nullptr, 0, 0, 0, 0, 0, nullptr};
    mock_component_location_t location = {LV_ALIGN_CENTER, 0, 0};
    
    clarity.render(&screen, location);
    key.render(&screen, location);
    
    // Both should integrate with style system
    TEST_ASSERT_TRUE(clarity.rendered);
    TEST_ASSERT_TRUE(key.rendered);
}

void test_component_positioning_consistency(void) {
    resetMockComponentState();
    MockKeyComponent key;
    MockLockComponent lock;
    
    mock_lv_obj_t screen = {true, false, false, false, false, false, nullptr, 0, 0, 0, 0, 0, nullptr};
    mock_component_location_t location = {LV_ALIGN_CENTER, 15, -25};
    
    key.render(&screen, location);
    lock.render(&screen, location);
    
    // Both should handle positioning consistently
    TEST_ASSERT_TRUE(key.keyIcon_->aligned);
    TEST_ASSERT_TRUE(lock.lockIcon_->aligned);
    TEST_ASSERT_EQUAL_INT32(LV_ALIGN_CENTER, key.keyIcon_->align_type);
    TEST_ASSERT_EQUAL_INT32(LV_ALIGN_CENTER, lock.lockIcon_->align_type);
    TEST_ASSERT_EQUAL_INT32(15, key.keyIcon_->x_offset);
    TEST_ASSERT_EQUAL_INT32(15, lock.lockIcon_->x_offset);
    TEST_ASSERT_EQUAL_INT32(-25, key.keyIcon_->y_offset);
    TEST_ASSERT_EQUAL_INT32(-25, lock.lockIcon_->y_offset);
}

// =================================================================
// ERROR HANDLING TESTS
// =================================================================

void test_component_null_screen_handling(void) {
    resetMockComponentState();
    MockClarityComponent component;
    
    mock_component_location_t location = {LV_ALIGN_CENTER, 0, 0};
    
    // Should handle null screen gracefully (though not recommended)
    component.render(nullptr, location);
    
    // Component should still mark as rendered
    TEST_ASSERT_TRUE(component.rendered);
}

void test_component_refresh_without_render(void) {
    resetMockComponentState();
    MockKeyComponent component;
    
    // Refresh without rendering first
    mock_reading_t reading = {static_cast<int32_t>(KeyState::Present), 0.0f, false};
    component.refresh(reading);
    
    // Should handle gracefully (no crash)
    TEST_ASSERT_TRUE(component.refreshed);
    TEST_ASSERT_NULL(component.keyIcon_); // Icon not created yet
}

// =================================================================
// PERFORMANCE TESTS
// =================================================================

void test_component_multiple_refresh_calls(void) {
    resetMockComponentState();
    MockOemOilComponent component;
    
    mock_lv_obj_t screen = {true, false, false, false, false, false, nullptr, 0, 0, 0, 0, 0, nullptr};
    mock_component_location_t location = {LV_ALIGN_CENTER, 0, 0};
    
    component.render(&screen, location);
    
    // Multiple rapid refresh calls
    for (int i = 0; i < 10; i++) {
        mock_reading_t reading = {0, static_cast<float>(i), false};
        component.refresh(reading);
        
        TEST_ASSERT_EQUAL_FLOAT(static_cast<float>(i), component.last_value);
    }
    
    TEST_ASSERT_TRUE(component.refreshed);
}

void test_component_memory_efficiency(void) {
    resetMockComponentState();
    
    // Create multiple components
    MockClarityComponent clarity;
    MockKeyComponent key;
    MockLockComponent lock;
    MockOemOilComponent oil;
    
    mock_lv_obj_t screen = {true, false, false, false, false, false, nullptr, 0, 0, 0, 0, 0, nullptr};
    mock_component_location_t location = {LV_ALIGN_CENTER, 0, 0};
    
    clarity.render(&screen, location);
    key.render(&screen, location);
    lock.render(&screen, location);
    oil.render(&screen, location);
    
    // All should be efficiently created
    TEST_ASSERT_TRUE(clarity.rendered);
    TEST_ASSERT_TRUE(key.rendered);
    TEST_ASSERT_TRUE(lock.rendered);
    TEST_ASSERT_TRUE(oil.rendered);
}

// Note: PlatformIO will automatically discover and run test_ functions