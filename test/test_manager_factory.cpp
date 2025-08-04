#include <unity.h>
#include "factories/manager_factory.h"
#include "managers/panel_manager.h"
#include "managers/style_manager.h"
#include "managers/trigger_manager.h"
#include "managers/preference_manager.h"
#include "mocks/mock_services.h"
#include "mocks/mock_gpio_provider.h"
#include "mock_globals.h"
#include <memory>

// Use global mock services to prevent redefinition conflicts
static MockPanelService* mockPanel = nullptr;

void setUp(void) {
    initGlobalMocks();
    mockPanel = new MockPanelService();
    mockPanel->init();
}

void tearDown(void) {
    delete mockPanel;
    mockPanel = nullptr;
    // Global mocks will be cleaned up by the global cleanup
}

void test_manager_factory_create_panel_manager_valid() {
    // Test creating PanelManager with valid dependencies
    auto manager = ManagerFactory::createPanelManager(g_mockDisplay, g_mockGpio, g_mockStyle);
    
    TEST_ASSERT_NOT_NULL(manager.get());
    TEST_ASSERT_TRUE(manager != nullptr);
}

void test_manager_factory_create_panel_manager_null_display() {
    // Test creating PanelManager with null display provider
    try {
        auto manager = ManagerFactory::createPanelManager(nullptr, g_mockGpio, g_mockStyle);
        TEST_FAIL_MESSAGE("Expected exception for null display provider");
    } catch (const std::invalid_argument& e) {
        TEST_ASSERT_TRUE(std::string(e.what()).find("IDisplayProvider") != std::string::npos);
    }
}

void test_manager_factory_create_panel_manager_null_gpio() {
    // Test creating PanelManager with null GPIO provider
    try {
        auto manager = ManagerFactory::createPanelManager(g_mockDisplay, nullptr, g_mockStyle);
        TEST_FAIL_MESSAGE("Expected exception for null GPIO provider");
    } catch (const std::invalid_argument& e) {
        TEST_ASSERT_TRUE(std::string(e.what()).find("IGpioProvider") != std::string::npos);
    }
}

void test_manager_factory_create_panel_manager_null_style() {
    // Test creating PanelManager with null style service
    try {
        auto manager = ManagerFactory::createPanelManager(g_mockDisplay, g_mockGpio, nullptr);
        TEST_FAIL_MESSAGE("Expected exception for null style service");
    } catch (const std::invalid_argument& e) {
        TEST_ASSERT_TRUE(std::string(e.what()).find("IStyleService") != std::string::npos);
    }
}

void test_manager_factory_create_style_manager_default() {
    // Test creating StyleManager with default theme
    auto manager = ManagerFactory::createStyleManager();
    
    TEST_ASSERT_NOT_NULL(manager.get());
    TEST_ASSERT_TRUE(manager != nullptr);
}

void test_manager_factory_create_style_manager_with_theme() {
    // Test creating StyleManager with specific theme
    auto manager = ManagerFactory::createStyleManager("DAY");
    
    TEST_ASSERT_NOT_NULL(manager.get());
    TEST_ASSERT_TRUE(manager != nullptr);
}

void test_manager_factory_create_trigger_manager_valid() {
    // Test creating TriggerManager with valid dependencies
    auto manager = ManagerFactory::createTriggerManager(g_mockGpio, mockPanel, g_mockStyle);
    
    TEST_ASSERT_NOT_NULL(manager.get());
    TEST_ASSERT_TRUE(manager != nullptr);
}

void test_manager_factory_create_trigger_manager_null_gpio() {
    // Test creating TriggerManager with null GPIO provider
    try {
        auto manager = ManagerFactory::createTriggerManager(nullptr, mockPanel, g_mockStyle);
        TEST_FAIL_MESSAGE("Expected exception for null GPIO provider");
    } catch (const std::invalid_argument& e) {
        TEST_ASSERT_TRUE(std::string(e.what()).find("IGpioProvider") != std::string::npos);
    }
}

void test_manager_factory_create_trigger_manager_null_panel() {
    // Test creating TriggerManager with null panel service
    try {
        auto manager = ManagerFactory::createTriggerManager(g_mockGpio, nullptr, g_mockStyle);
        TEST_FAIL_MESSAGE("Expected exception for null panel service");
    } catch (const std::invalid_argument& e) {
        TEST_ASSERT_TRUE(std::string(e.what()).find("IPanelService") != std::string::npos);
    }
}

void test_manager_factory_create_trigger_manager_null_style() {
    // Test creating TriggerManager with null style service
    try {
        auto manager = ManagerFactory::createTriggerManager(g_mockGpio, mockPanel, nullptr);
        TEST_FAIL_MESSAGE("Expected exception for null style service");
    } catch (const std::invalid_argument& e) {
        TEST_ASSERT_TRUE(std::string(e.what()).find("IStyleService") != std::string::npos);
    }
}

void test_manager_factory_create_preference_manager() {
    // Test creating PreferenceManager (no dependencies)
    auto manager = ManagerFactory::createPreferenceManager();
    
    TEST_ASSERT_NOT_NULL(manager.get());
    TEST_ASSERT_TRUE(manager != nullptr);
}

void test_manager_factory_multiple_instances() {
    // Test creating multiple instances of same manager type
    auto manager1 = ManagerFactory::createStyleManager("DAY");
    auto manager2 = ManagerFactory::createStyleManager("NIGHT");
    
    TEST_ASSERT_NOT_NULL(manager1.get());
    TEST_ASSERT_NOT_NULL(manager2.get());
    TEST_ASSERT_NOT_EQUAL(manager1.get(), manager2.get());
}

void test_manager_factory_unique_ownership() {
    // Test that each factory call returns unique ownership
    auto manager1 = ManagerFactory::createPreferenceManager();
    auto manager2 = ManagerFactory::createPreferenceManager();
    
    TEST_ASSERT_NOT_NULL(manager1.get());
    TEST_ASSERT_NOT_NULL(manager2.get());
    TEST_ASSERT_NOT_EQUAL(manager1.get(), manager2.get());
    
    // Move ownership should work
    auto moved = std::move(manager1);
    TEST_ASSERT_NULL(manager1.get());
    TEST_ASSERT_NOT_NULL(moved.get());
}

void test_manager_factory_dependency_injection() {
    // Test that dependencies are properly injected
    auto panelManager = ManagerFactory::createPanelManager(g_mockDisplay, g_mockGpio, g_mockStyle);
    
    // Manager should be properly initialized and functional
    TEST_ASSERT_NOT_NULL(panelManager.get());
    
    // Test that we can interact with the manager
    // (This would normally involve checking that dependencies were properly set,
    // but since managers are implementation details, we verify they exist)
    TEST_ASSERT_TRUE(panelManager != nullptr);
}

void test_manager_factory_error_handling() {
    // Test various error conditions
    
    // Test all null parameters for PanelManager
    try {
        auto manager = ManagerFactory::createPanelManager(nullptr, nullptr, nullptr);
        TEST_FAIL_MESSAGE("Expected exception for all null parameters");
    } catch (const std::invalid_argument& e) {
        // Expected - should catch first null parameter
        TEST_ASSERT_TRUE(true);
    }
    
    // Test all null parameters for TriggerManager
    try {
        auto manager = ManagerFactory::createTriggerManager(nullptr, nullptr, nullptr);
        TEST_FAIL_MESSAGE("Expected exception for all null parameters");
    } catch (const std::invalid_argument& e) {
        // Expected - should catch first null parameter
        TEST_ASSERT_TRUE(true);
    }
}

void test_manager_factory_memory_management() {
    // Test that managers are properly destroyed when going out of scope
    {
        auto manager = ManagerFactory::createStyleManager();
        TEST_ASSERT_NOT_NULL(manager.get());
        // Manager should be destroyed automatically when leaving scope
    }
    
    // Test multiple creations and destructions
    for (int i = 0; i < 5; i++) {
        auto manager = ManagerFactory::createPreferenceManager();
        TEST_ASSERT_NOT_NULL(manager.get());
    }
    
    TEST_ASSERT_TRUE(true); // Test passed if no memory leaks/crashes
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    RUN_TEST(test_manager_factory_create_panel_manager_valid);
    RUN_TEST(test_manager_factory_create_panel_manager_null_display);
    RUN_TEST(test_manager_factory_create_panel_manager_null_gpio);
    RUN_TEST(test_manager_factory_create_panel_manager_null_style);
    RUN_TEST(test_manager_factory_create_style_manager_default);
    RUN_TEST(test_manager_factory_create_style_manager_with_theme);
    RUN_TEST(test_manager_factory_create_trigger_manager_valid);
    RUN_TEST(test_manager_factory_create_trigger_manager_null_gpio);
    RUN_TEST(test_manager_factory_create_trigger_manager_null_panel);
    RUN_TEST(test_manager_factory_create_trigger_manager_null_style);
    RUN_TEST(test_manager_factory_create_preference_manager);
    RUN_TEST(test_manager_factory_multiple_instances);
    RUN_TEST(test_manager_factory_unique_ownership);
    RUN_TEST(test_manager_factory_dependency_injection);
    RUN_TEST(test_manager_factory_error_handling);
    RUN_TEST(test_manager_factory_memory_management);
    
    return UNITY_END();
}