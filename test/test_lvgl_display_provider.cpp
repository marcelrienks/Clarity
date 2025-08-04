#include <unity.h>
#include "providers/lvgl_display_provider.h"
#include "lvgl.h"
#include <vector>

LvglDisplayProvider* displayProvider = nullptr;
lv_obj_t* testMainScreen = nullptr;

void setUp(void) {
    // Create a test main screen object
    testMainScreen = lv_obj_create(nullptr);
    displayProvider = new LvglDisplayProvider(testMainScreen);
}

void tearDown(void) {
    delete displayProvider;
    displayProvider = nullptr;
    if (testMainScreen) {
        lv_obj_del(testMainScreen);
        testMainScreen = nullptr;
    }
}

void test_lvgl_display_provider_construction() {
    // Test that provider can be created and destroyed
    TEST_ASSERT_NOT_NULL(displayProvider);
    TEST_ASSERT_FALSE(displayProvider->isInitialized());
}

void test_lvgl_display_provider_initialization() {
    // Test initialization
    TEST_ASSERT_FALSE(displayProvider->isInitialized());
    displayProvider->initialize();
    TEST_ASSERT_TRUE(displayProvider->isInitialized());
    
    // Test multiple initialization calls are safe
    displayProvider->initialize();
    TEST_ASSERT_TRUE(displayProvider->isInitialized());
}

void test_lvgl_display_provider_get_main_screen() {
    // Test getting main screen
    lv_obj_t* mainScreen = displayProvider->getMainScreen();
    TEST_ASSERT_NOT_NULL(mainScreen);
    TEST_ASSERT_EQUAL_PTR(testMainScreen, mainScreen);
}

void test_lvgl_display_provider_create_screen() {
    // Test screen creation
    lv_obj_t* screen = displayProvider->createScreen();
    TEST_ASSERT_NOT_NULL(screen);
    
    // Clean up
    displayProvider->deleteObject(screen);
}

void test_lvgl_display_provider_load_screen() {
    // Test screen loading
    lv_obj_t* screen = displayProvider->createScreen();
    TEST_ASSERT_NOT_NULL(screen);
    
    // Loading should not crash
    displayProvider->loadScreen(screen);
    
    // Clean up
    displayProvider->deleteObject(screen);
}

void test_lvgl_display_provider_create_label() {
    // Test label creation
    lv_obj_t* parent = displayProvider->createScreen();
    TEST_ASSERT_NOT_NULL(parent);
    
    lv_obj_t* label = displayProvider->createLabel(parent);
    TEST_ASSERT_NOT_NULL(label);
    
    // Clean up
    displayProvider->deleteObject(parent); // Should delete children too
}

void test_lvgl_display_provider_create_object() {
    // Test generic object creation
    lv_obj_t* parent = displayProvider->createScreen();
    TEST_ASSERT_NOT_NULL(parent);
    
    lv_obj_t* obj = displayProvider->createObject(parent);
    TEST_ASSERT_NOT_NULL(obj);
    
    // Clean up
    displayProvider->deleteObject(parent);
}

void test_lvgl_display_provider_create_arc() {
    // Test arc creation
    lv_obj_t* parent = displayProvider->createScreen();
    TEST_ASSERT_NOT_NULL(parent);
    
    lv_obj_t* arc = displayProvider->createArc(parent);
    TEST_ASSERT_NOT_NULL(arc);
    
    // Clean up
    displayProvider->deleteObject(parent);
}

void test_lvgl_display_provider_create_scale() {
    // Test scale creation
    lv_obj_t* parent = displayProvider->createScreen();
    TEST_ASSERT_NOT_NULL(parent);
    
    lv_obj_t* scale = displayProvider->createScale(parent);
    TEST_ASSERT_NOT_NULL(scale);
    
    // Clean up
    displayProvider->deleteObject(parent);
}

void test_lvgl_display_provider_create_image() {
    // Test image creation
    lv_obj_t* parent = displayProvider->createScreen();
    TEST_ASSERT_NOT_NULL(parent);
    
    lv_obj_t* image = displayProvider->createImage(parent);
    TEST_ASSERT_NOT_NULL(image);
    
    // Clean up
    displayProvider->deleteObject(parent);
}

void test_lvgl_display_provider_create_line() {
    // Test line creation
    lv_obj_t* parent = displayProvider->createScreen();
    TEST_ASSERT_NOT_NULL(parent);
    
    lv_obj_t* line = displayProvider->createLine(parent);
    TEST_ASSERT_NOT_NULL(line);
    
    // Clean up
    displayProvider->deleteObject(parent);
}

void test_lvgl_display_provider_delete_object_null_safety() {
    // Test that deleting null object is safe
    displayProvider->deleteObject(nullptr);
    // Should not crash
    TEST_ASSERT_TRUE(true);
}

void test_lvgl_display_provider_delete_object_valid() {
    // Test deleting valid object
    lv_obj_t* obj = displayProvider->createObject(nullptr);
    TEST_ASSERT_NOT_NULL(obj);
    
    displayProvider->deleteObject(obj);
    // Should not crash
    TEST_ASSERT_TRUE(true);
}

void test_lvgl_display_provider_add_event_callback() {
    // Test adding event callback
    lv_obj_t* obj = displayProvider->createObject(nullptr);
    TEST_ASSERT_NOT_NULL(obj);
    
    // Mock event callback
    auto callback = [](lv_event_t* e) {
        // Mock callback implementation
    };
    
    // Should not crash
    displayProvider->addEventCallback(obj, callback, LV_EVENT_CLICKED, nullptr);
    
    // Clean up
    displayProvider->deleteObject(obj);
}

void test_lvgl_display_provider_interface_compliance() {
    // Test that LvglDisplayProvider implements IDisplayProvider interface correctly
    IDisplayProvider* provider = displayProvider;
    TEST_ASSERT_NOT_NULL(provider);
    
    // Test interface methods work
    provider->initialize();
    TEST_ASSERT_TRUE(provider->isInitialized());
    
    lv_obj_t* screen = provider->createScreen();
    TEST_ASSERT_NOT_NULL(screen);
    
    lv_obj_t* label = provider->createLabel(screen);
    TEST_ASSERT_NOT_NULL(label);
    
    lv_obj_t* mainScreen = provider->getMainScreen();
    TEST_ASSERT_NOT_NULL(mainScreen);
    
    // Clean up
    provider->deleteObject(screen);
}

void test_lvgl_display_provider_multiple_objects() {
    // Test creating multiple objects
    lv_obj_t* screen = displayProvider->createScreen();
    TEST_ASSERT_NOT_NULL(screen);
    
    // Create multiple child objects
    std::vector<lv_obj_t*> objects;
    for (int i = 0; i < 5; i++) {
        lv_obj_t* obj = displayProvider->createObject(screen);
        TEST_ASSERT_NOT_NULL(obj);
        objects.push_back(obj);
    }
    
    // Create different types of objects
    lv_obj_t* label = displayProvider->createLabel(screen);
    lv_obj_t* arc = displayProvider->createArc(screen);
    lv_obj_t* image = displayProvider->createImage(screen);
    
    TEST_ASSERT_NOT_NULL(label);
    TEST_ASSERT_NOT_NULL(arc);
    TEST_ASSERT_NOT_NULL(image);
    
    // Clean up - deleting parent should delete all children
    displayProvider->deleteObject(screen);
}

void test_lvgl_display_provider_constructor_with_null() {
    // Test constructor with null main screen
    LvglDisplayProvider* nullProvider = new LvglDisplayProvider(nullptr);
    TEST_ASSERT_NOT_NULL(nullProvider);
    
    // Should return null for getMainScreen
    lv_obj_t* mainScreen = nullProvider->getMainScreen();
    TEST_ASSERT_NULL(mainScreen);
    
    delete nullProvider;
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    RUN_TEST(test_lvgl_display_provider_construction);
    RUN_TEST(test_lvgl_display_provider_initialization);
    RUN_TEST(test_lvgl_display_provider_get_main_screen);
    RUN_TEST(test_lvgl_display_provider_create_screen);
    RUN_TEST(test_lvgl_display_provider_load_screen);
    RUN_TEST(test_lvgl_display_provider_create_label);
    RUN_TEST(test_lvgl_display_provider_create_object);
    RUN_TEST(test_lvgl_display_provider_create_arc);
    RUN_TEST(test_lvgl_display_provider_create_scale);
    RUN_TEST(test_lvgl_display_provider_create_image);
    RUN_TEST(test_lvgl_display_provider_create_line);
    RUN_TEST(test_lvgl_display_provider_delete_object_null_safety);
    RUN_TEST(test_lvgl_display_provider_delete_object_valid);
    RUN_TEST(test_lvgl_display_provider_add_event_callback);
    RUN_TEST(test_lvgl_display_provider_interface_compliance);
    RUN_TEST(test_lvgl_display_provider_multiple_objects);
    
    // Test constructor with null separately
    RUN_TEST(test_lvgl_display_provider_constructor_with_null);
    
    return UNITY_END();
}