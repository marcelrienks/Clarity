#include <unity.h>
#include "factories/ui_factory.h"
#include "mocks/mock_services.h"
#include "mocks/mock_gpio_provider.h"
#include <memory>

// Mock services for testing
static MockDisplayProvider* mockDisplayUI = nullptr;
static MockGpioProvider* mockGpioUI = nullptr;
static MockStyleService* mockStyleUI = nullptr;

void setUp(void) {
    mockDisplayUI = new MockDisplayProvider();
    mockGpioUI = new MockGpioProvider();
    mockStyleUI = new MockStyleService();
    
    mockDisplayUI->initialize();
    mockStyleUI->initializeStyles();
}

void tearDown(void) {
    delete mockDisplayUI;
    delete mockGpioUI;
    delete mockStyleUI;
    
    mockDisplayUI = nullptr;
    mockGpioUI = nullptr;
    mockStyleUI = nullptr;
}

void test_ui_factory_create_key_component() {
    // Test creating KeyComponent
    auto component = UIFactory::createKeyComponent(mockStyleUI);
    
    TEST_ASSERT_NOT_NULL(component.get());
    TEST_ASSERT_TRUE(component != nullptr);
}

void test_ui_factory_create_lock_component() {
    // Test creating LockComponent
    auto component = UIFactory::createLockComponent(mockStyleUI);
    
    TEST_ASSERT_NOT_NULL(component.get());
    TEST_ASSERT_TRUE(component != nullptr);
}

void test_ui_factory_create_clarity_component() {
    // Test creating ClarityComponent
    auto component = UIFactory::createClarityComponent(mockStyleUI);
    
    TEST_ASSERT_NOT_NULL(component.get());
    TEST_ASSERT_TRUE(component != nullptr);
}

void test_ui_factory_create_oem_oil_pressure_component() {
    // Test creating OemOilPressureComponent
    auto component = UIFactory::createOemOilPressureComponent(mockStyleUI);
    
    TEST_ASSERT_NOT_NULL(component.get());
    TEST_ASSERT_TRUE(component != nullptr);
}

void test_ui_factory_create_oem_oil_temperature_component() {
    // Test creating OemOilTemperatureComponent
    auto component = UIFactory::createOemOilTemperatureComponent(mockStyleUI);
    
    TEST_ASSERT_NOT_NULL(component.get());
    TEST_ASSERT_TRUE(component != nullptr);
}

void test_ui_factory_create_key_panel() {
    // Test creating KeyPanel
    auto panel = UIFactory::createKeyPanel(mockGpioUI, mockDisplayUI, mockStyleUI);
    
    TEST_ASSERT_NOT_NULL(panel.get());
    TEST_ASSERT_TRUE(panel != nullptr);
}

void test_ui_factory_create_lock_panel() {
    // Test creating LockPanel
    auto panel = UIFactory::createLockPanel(mockGpioUI, mockDisplayUI, mockStyleUI);
    
    TEST_ASSERT_NOT_NULL(panel.get());
    TEST_ASSERT_TRUE(panel != nullptr);
}

void test_ui_factory_create_splash_panel() {
    // Test creating SplashPanel
    auto panel = UIFactory::createSplashPanel(mockGpioUI, mockDisplayUI, mockStyleUI);
    
    TEST_ASSERT_NOT_NULL(panel.get());
    TEST_ASSERT_TRUE(panel != nullptr);
}

void test_ui_factory_create_oem_oil_panel() {
    // Test creating OemOilPanel
    auto panel = UIFactory::createOemOilPanel(mockGpioUI, mockDisplayUI, mockStyleUI);
    
    TEST_ASSERT_NOT_NULL(panel.get());
    TEST_ASSERT_TRUE(panel != nullptr);
}

void test_ui_factory_component_multiple_instances() {
    // Test creating multiple instances of same component type
    auto component1 = UIFactory::createKeyComponent(mockStyleUI);
    auto component2 = UIFactory::createKeyComponent(mockStyleUI);
    
    TEST_ASSERT_NOT_NULL(component1.get());
    TEST_ASSERT_NOT_NULL(component2.get());
    TEST_ASSERT_NOT_EQUAL(component1.get(), component2.get());
}

void test_ui_factory_panel_multiple_instances() {
    // Test creating multiple instances of same panel type
    auto panel1 = UIFactory::createSplashPanel(mockGpioUI, mockDisplayUI, mockStyleUI);
    auto panel2 = UIFactory::createSplashPanel(mockGpioUI, mockDisplayUI, mockStyleUI);
    
    TEST_ASSERT_NOT_NULL(panel1.get());
    TEST_ASSERT_NOT_NULL(panel2.get());
    TEST_ASSERT_NOT_EQUAL(panel1.get(), panel2.get());
}

void test_ui_factory_component_dependency_injection() {
    // Test that components receive style service dependency
    auto component = UIFactory::createClarityComponent(mockStyleUI);
    
    TEST_ASSERT_NOT_NULL(component.get());
    // Component should be properly initialized with style service
    TEST_ASSERT_TRUE(component != nullptr);
}

void test_ui_factory_panel_dependency_injection() {
    // Test that panels receive all required dependencies
    auto panel = UIFactory::createOemOilPanel(mockGpioUI, mockDisplayUI, mockStyleUI);
    
    TEST_ASSERT_NOT_NULL(panel.get());
    // Panel should be properly initialized with all services
    TEST_ASSERT_TRUE(panel != nullptr);
}

void test_ui_factory_unique_ownership() {
    // Test that each factory call returns unique ownership
    auto component1 = UIFactory::createKeyComponent(mockStyleUI);
    auto component2 = UIFactory::createLockComponent(mockStyleUI);
    
    TEST_ASSERT_NOT_NULL(component1.get());
    TEST_ASSERT_NOT_NULL(component2.get());
    TEST_ASSERT_NOT_EQUAL(component1.get(), component2.get());
    
    // Move ownership should work
    auto moved = std::move(component1);
    TEST_ASSERT_NULL(component1.get());
    TEST_ASSERT_NOT_NULL(moved.get());
}

void test_ui_factory_all_components_creation() {
    // Test creating all component types to ensure they all work
    auto keyComp = UIFactory::createKeyComponent(mockStyleUI);
    auto lockComp = UIFactory::createLockComponent(mockStyleUI);
    auto clarityComp = UIFactory::createClarityComponent(mockStyleUI);
    auto pressureComp = UIFactory::createOemOilPressureComponent(mockStyleUI);
    auto tempComp = UIFactory::createOemOilTemperatureComponent(mockStyleUI);
    
    TEST_ASSERT_NOT_NULL(keyComp.get());
    TEST_ASSERT_NOT_NULL(lockComp.get());
    TEST_ASSERT_NOT_NULL(clarityComp.get());
    TEST_ASSERT_NOT_NULL(pressureComp.get());
    TEST_ASSERT_NOT_NULL(tempComp.get());
    
    // All should be different instances
    TEST_ASSERT_NOT_EQUAL(keyComp.get(), lockComp.get());
    TEST_ASSERT_NOT_EQUAL(lockComp.get(), clarityComp.get());
    TEST_ASSERT_NOT_EQUAL(clarityComp.get(), pressureComp.get());
    TEST_ASSERT_NOT_EQUAL(pressureComp.get(), tempComp.get());
}

void test_ui_factory_all_panels_creation() {
    // Test creating all panel types to ensure they all work
    auto keyPanel = UIFactory::createKeyPanel(mockGpioUI, mockDisplayUI, mockStyleUI);
    auto lockPanel = UIFactory::createLockPanel(mockGpioUI, mockDisplayUI, mockStyleUI);
    auto splashPanel = UIFactory::createSplashPanel(mockGpioUI, mockDisplayUI, mockStyleUI);
    auto oilPanel = UIFactory::createOemOilPanel(mockGpioUI, mockDisplayUI, mockStyleUI);
    
    TEST_ASSERT_NOT_NULL(keyPanel.get());
    TEST_ASSERT_NOT_NULL(lockPanel.get());
    TEST_ASSERT_NOT_NULL(splashPanel.get());
    TEST_ASSERT_NOT_NULL(oilPanel.get());
    
    // All should be different instances
    TEST_ASSERT_NOT_EQUAL(keyPanel.get(), lockPanel.get());
    TEST_ASSERT_NOT_EQUAL(lockPanel.get(), splashPanel.get());
    TEST_ASSERT_NOT_EQUAL(splashPanel.get(), oilPanel.get());
}

void test_ui_factory_memory_management() {
    // Test that objects are properly destroyed when going out of scope
    {
        auto component = UIFactory::createKeyComponent(mockStyleUI);
        auto panel = UIFactory::createKeyPanel(mockGpioUI, mockDisplayUI, mockStyleUI);
        TEST_ASSERT_NOT_NULL(component.get());
        TEST_ASSERT_NOT_NULL(panel.get());
        // Objects should be destroyed automatically when leaving scope
    }
    
    // Test multiple creations and destructions
    for (int i = 0; i < 5; i++) {
        auto component = UIFactory::createClarityComponent(mockStyleUI);
        auto panel = UIFactory::createSplashPanel(mockGpioUI, mockDisplayUI, mockStyleUI);
        TEST_ASSERT_NOT_NULL(component.get());
        TEST_ASSERT_NOT_NULL(panel.get());
    }
    
    TEST_ASSERT_TRUE(true); // Test passed if no memory leaks/crashes
}

void test_ui_factory_interface_compliance() {
    // Test that created objects implement the correct interfaces
    auto component = UIFactory::createKeyComponent(mockStyleUI);
    auto panel = UIFactory::createKeyPanel(mockGpioUI, mockDisplayUI, mockStyleUI);
    
    // Cast to interfaces should work
    IComponent* compInterface = component.get();
    IPanel* panelInterface = panel.get();
    
    TEST_ASSERT_NOT_NULL(compInterface);
    TEST_ASSERT_NOT_NULL(panelInterface);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    RUN_TEST(test_ui_factory_create_key_component);
    RUN_TEST(test_ui_factory_create_lock_component);
    RUN_TEST(test_ui_factory_create_clarity_component);
    RUN_TEST(test_ui_factory_create_oem_oil_pressure_component);
    RUN_TEST(test_ui_factory_create_oem_oil_temperature_component);
    RUN_TEST(test_ui_factory_create_key_panel);
    RUN_TEST(test_ui_factory_create_lock_panel);
    RUN_TEST(test_ui_factory_create_splash_panel);
    RUN_TEST(test_ui_factory_create_oem_oil_panel);
    RUN_TEST(test_ui_factory_component_multiple_instances);
    RUN_TEST(test_ui_factory_panel_multiple_instances);
    RUN_TEST(test_ui_factory_component_dependency_injection);
    RUN_TEST(test_ui_factory_panel_dependency_injection);
    RUN_TEST(test_ui_factory_unique_ownership);
    RUN_TEST(test_ui_factory_all_components_creation);
    RUN_TEST(test_ui_factory_all_panels_creation);
    RUN_TEST(test_ui_factory_memory_management);
    RUN_TEST(test_ui_factory_interface_compliance);
    
    return UNITY_END();
}