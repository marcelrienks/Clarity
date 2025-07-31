#include <unity.h>
#include "system/component_registry.h"
#include "test_device.h"
#include "providers/mock_gpio_provider.h"
#include "providers/mock_display_provider.h"
#include "factories/manager_factory.h"
#include "utilities/test_component_registry.h"

void test_component_registry_panel_creation() {
    // Setup test environment
    TestUtilities::registerTestComponents();
    
    auto mockGpio = std::make_unique<MockGpioProvider>();
    auto mockDisplay = std::make_unique<MockDisplayProvider>();
    
    // Get raw pointers before moving
    MockGpioProvider* gpioPtr = mockGpio.get();
    MockDisplayProvider* displayPtr = mockDisplay.get();
    
    auto testDevice = std::make_unique<TestDevice>(std::move(mockGpio), std::move(mockDisplay));
    
    // Test panel creation via registry
    auto& registry = ComponentRegistry::GetInstance();
    auto panel = registry.createPanel("key", gpioPtr, displayPtr);
    
    TEST_ASSERT_NOT_NULL(panel.get());
    // Cast to test panel to access test-specific method
    auto testPanel = dynamic_cast<TestUtilities::TestKeyPanel*>(panel.get());
    TEST_ASSERT_NOT_NULL(testPanel);
    TEST_ASSERT_EQUAL_STRING("test_key", testPanel->getPanelName());
    
    // Test panel initialization  
    panel->init(gpioPtr, displayPtr);
    TEST_ASSERT_TRUE(testPanel->isInitialized());
}

void test_component_registry_component_creation() {
    // Setup test environment
    TestUtilities::registerTestComponents();
    
    auto mockDisplay = std::make_unique<MockDisplayProvider>();
    MockDisplayProvider* displayPtr = mockDisplay.get();
    
    // Test component creation via registry
    auto& registry = ComponentRegistry::GetInstance();
    auto component = registry.createComponent("key");
    
    TEST_ASSERT_NOT_NULL(component.get());
    
    // Test component with reading
    Reading testReading = 1.0f;  // Use variant assignment
    // In a real test, we'd test component methods here
    
    auto testComponent = dynamic_cast<TestUtilities::TestKeyComponent*>(component.get());
    TEST_ASSERT_NOT_NULL(testComponent);
    TEST_ASSERT_TRUE(testComponent->isLoaded());
    TEST_ASSERT_TRUE(testComponent->isUpdated());
}

void test_full_system_with_registry() {
    // Setup complete test system
    TestUtilities::registerTestComponents();
    
    auto mockGpio = std::make_unique<MockGpioProvider>();
    auto mockDisplay = std::make_unique<MockDisplayProvider>();
    
    // Configure test scenario
    mockGpio->setDigitalPin(25, true); // KEY_PRESENT
    
    // Get raw pointers before moving
    MockGpioProvider* gpioPtr = mockGpio.get();
    MockDisplayProvider* displayPtr = mockDisplay.get();
    
    auto testDevice = std::make_unique<TestDevice>(std::move(mockGpio), std::move(mockDisplay));
    
    // Create real system managers with test device
    auto panelManager = ManagerFactory::createPanelManager(displayPtr, gpioPtr);
    
    // Test that we can use registry to create components in manager context
    auto& registry = ComponentRegistry::GetInstance();
    TEST_ASSERT_TRUE(registry.hasPanelRegistration("key"));
    TEST_ASSERT_TRUE(registry.hasComponentRegistration("key"));
    
    // Verify registry can create both panels and components
    auto panel = registry.createPanel("key", gpioPtr, displayPtr);
    auto component = registry.createComponent("key");
    
    TEST_ASSERT_NOT_NULL(panel.get());
    TEST_ASSERT_NOT_NULL(component.get());
    
    // Test that unregistered components return nullptr
    auto unknownPanel = registry.createPanel("unknown", gpioPtr, displayPtr);
    auto unknownComponent = registry.createComponent("unknown");
    
    TEST_ASSERT_NULL(unknownPanel.get());
    TEST_ASSERT_NULL(unknownComponent.get());
}

void setUp(void) {
    // Clear registry before each test
    ComponentRegistry::GetInstance().clear();
}

void tearDown(void) {
    // Clean up after each test
    ComponentRegistry::GetInstance().clear();
}

void runComponentRegistryIntegrationTests() {
    UNITY_BEGIN();
    RUN_TEST(test_component_registry_panel_creation);
    RUN_TEST(test_component_registry_component_creation);
    RUN_TEST(test_full_system_with_registry);
    UNITY_END();
}