#include <unity.h>
#include <memory>
#include <stdexcept>
#include "mocks/mock_services.h"
#include "mocks/mock_gpio_provider.h"

// Mock component interfaces - testing factory patterns only
class SimplifiedMockComponent {
public:
    SimplifiedMockComponent(void* style_service) : style_service_(style_service) {}
    void* style_service_;
    virtual ~SimplifiedMockComponent() = default;
};

class SimplifiedMockPanel {
public:
    SimplifiedMockPanel(void* gpio, void* display, void* style) 
        : gpio_(gpio), display_(display), style_(style) {}
    void* gpio_;
    void* display_;
    void* style_;
    virtual ~SimplifiedMockPanel() = default;
};

// Simplified UIFactory for testing - tests factory patterns without implementations
class SimplifiedUIFactory {
public:
    // Component factory methods - return mock components for testing
    static std::unique_ptr<SimplifiedMockComponent> createKeyComponent(void* style_service) {
        if (!style_service) throw std::invalid_argument("IStyleService cannot be null");
        return std::make_unique<SimplifiedMockComponent>(style_service);
    }
    
    static std::unique_ptr<SimplifiedMockComponent> createLockComponent(void* style_service) {
        if (!style_service) throw std::invalid_argument("IStyleService cannot be null");
        return std::make_unique<SimplifiedMockComponent>(style_service);
    }
    
    static std::unique_ptr<SimplifiedMockComponent> createClarityComponent(void* style_service) {
        if (!style_service) throw std::invalid_argument("IStyleService cannot be null");
        return std::make_unique<SimplifiedMockComponent>(style_service);
    }
    
    static std::unique_ptr<SimplifiedMockComponent> createOemOilPressureComponent(void* style_service) {
        if (!style_service) throw std::invalid_argument("IStyleService cannot be null");
        return std::make_unique<SimplifiedMockComponent>(style_service);
    }
    
    static std::unique_ptr<SimplifiedMockComponent> createOemOilTemperatureComponent(void* style_service) {
        if (!style_service) throw std::invalid_argument("IStyleService cannot be null");
        return std::make_unique<SimplifiedMockComponent>(style_service);
    }
    
    // Panel factory methods - return mock panels for testing
    static std::unique_ptr<SimplifiedMockPanel> createKeyPanel(void* gpio, void* display, void* style) {
        if (!gpio) throw std::invalid_argument("IGpioProvider cannot be null");
        if (!display) throw std::invalid_argument("IDisplayProvider cannot be null");
        if (!style) throw std::invalid_argument("IStyleService cannot be null");
        return std::make_unique<SimplifiedMockPanel>(gpio, display, style);
    }
    
    static std::unique_ptr<SimplifiedMockPanel> createLockPanel(void* gpio, void* display, void* style) {
        if (!gpio) throw std::invalid_argument("IGpioProvider cannot be null");
        if (!display) throw std::invalid_argument("IDisplayProvider cannot be null");
        if (!style) throw std::invalid_argument("IStyleService cannot be null");
        return std::make_unique<SimplifiedMockPanel>(gpio, display, style);
    }
    
    static std::unique_ptr<SimplifiedMockPanel> createSplashPanel(void* gpio, void* display, void* style) {
        if (!gpio) throw std::invalid_argument("IGpioProvider cannot be null");
        if (!display) throw std::invalid_argument("IDisplayProvider cannot be null");
        if (!style) throw std::invalid_argument("IStyleService cannot be null");
        return std::make_unique<SimplifiedMockPanel>(gpio, display, style);
    }
    
    static std::unique_ptr<SimplifiedMockPanel> createOemOilPanel(void* gpio, void* display, void* style) {
        if (!gpio) throw std::invalid_argument("IGpioProvider cannot be null");
        if (!display) throw std::invalid_argument("IDisplayProvider cannot be null");
        if (!style) throw std::invalid_argument("IStyleService cannot be null");
        return std::make_unique<SimplifiedMockPanel>(gpio, display, style);
    }
};

// Mock services for testing - use different names to avoid conflicts
static MockDisplayProvider* mockDisplaySimplified = nullptr;
static MockGpioProvider* mockGpioSimplified = nullptr;
static MockStyleService* mockStyleSimplified = nullptr;

void setUp(void) {
    mockDisplaySimplified = new MockDisplayProvider();
    mockGpioSimplified = new MockGpioProvider();
    mockStyleSimplified = new MockStyleService();
    
    mockDisplaySimplified->initialize();
    mockStyleSimplified->initializeStyles();
}

void tearDown(void) {
    delete mockDisplaySimplified;
    delete mockGpioSimplified;
    delete mockStyleSimplified;
    
    mockDisplaySimplified = nullptr;
    mockGpioSimplified = nullptr;
    mockStyleSimplified = nullptr;
}

void test_simplified_ui_factory_create_key_component() {
    // Test creating KeyComponent with factory pattern
    auto component = SimplifiedUIFactory::createKeyComponent(mockStyleSimplified);
    
    TEST_ASSERT_NOT_NULL(component.get());
    TEST_ASSERT_EQUAL_PTR(mockStyleSimplified, component->style_service_);
}

void test_simplified_ui_factory_create_lock_component() {
    // Test creating LockComponent with factory pattern
    auto component = SimplifiedUIFactory::createLockComponent(mockStyleSimplified);
    
    TEST_ASSERT_NOT_NULL(component.get());
    TEST_ASSERT_EQUAL_PTR(mockStyleSimplified, component->style_service_);
}

void test_simplified_ui_factory_create_clarity_component() {
    // Test creating ClarityComponent with factory pattern
    auto component = SimplifiedUIFactory::createClarityComponent(mockStyleSimplified);
    
    TEST_ASSERT_NOT_NULL(component.get());
    TEST_ASSERT_EQUAL_PTR(mockStyleSimplified, component->style_service_);
}

void test_simplified_ui_factory_create_oem_oil_pressure_component() {
    // Test creating OemOilPressureComponent with factory pattern
    auto component = SimplifiedUIFactory::createOemOilPressureComponent(mockStyleSimplified);
    
    TEST_ASSERT_NOT_NULL(component.get());
    TEST_ASSERT_EQUAL_PTR(mockStyleSimplified, component->style_service_);
}

void test_simplified_ui_factory_create_oem_oil_temperature_component() {
    // Test creating OemOilTemperatureComponent with factory pattern
    auto component = SimplifiedUIFactory::createOemOilTemperatureComponent(mockStyleSimplified);
    
    TEST_ASSERT_NOT_NULL(component.get());
    TEST_ASSERT_EQUAL_PTR(mockStyleSimplified, component->style_service_);
}

void test_simplified_ui_factory_create_key_panel() {
    // Test creating KeyPanel with factory pattern
    auto panel = SimplifiedUIFactory::createKeyPanel(mockGpioSimplified, mockDisplaySimplified, mockStyleSimplified);
    
    TEST_ASSERT_NOT_NULL(panel.get());
    TEST_ASSERT_EQUAL_PTR(mockGpioSimplified, panel->gpio_);
    TEST_ASSERT_EQUAL_PTR(mockDisplaySimplified, panel->display_);
    TEST_ASSERT_EQUAL_PTR(mockStyleSimplified, panel->style_);
}

void test_simplified_ui_factory_create_lock_panel() {
    // Test creating LockPanel with factory pattern
    auto panel = SimplifiedUIFactory::createLockPanel(mockGpioSimplified, mockDisplaySimplified, mockStyleSimplified);
    
    TEST_ASSERT_NOT_NULL(panel.get());
    TEST_ASSERT_EQUAL_PTR(mockGpioSimplified, panel->gpio_);
    TEST_ASSERT_EQUAL_PTR(mockDisplaySimplified, panel->display_);
    TEST_ASSERT_EQUAL_PTR(mockStyleSimplified, panel->style_);
}

void test_simplified_ui_factory_create_splash_panel() {
    // Test creating SplashPanel with factory pattern
    auto panel = SimplifiedUIFactory::createSplashPanel(mockGpioSimplified, mockDisplaySimplified, mockStyleSimplified);
    
    TEST_ASSERT_NOT_NULL(panel.get());
    TEST_ASSERT_EQUAL_PTR(mockGpioSimplified, panel->gpio_);
    TEST_ASSERT_EQUAL_PTR(mockDisplaySimplified, panel->display_);
    TEST_ASSERT_EQUAL_PTR(mockStyleSimplified, panel->style_);
}

void test_simplified_ui_factory_create_oem_oil_panel() {
    // Test creating OemOilPanel with factory pattern
    auto panel = SimplifiedUIFactory::createOemOilPanel(mockGpioSimplified, mockDisplaySimplified, mockStyleSimplified);
    
    TEST_ASSERT_NOT_NULL(panel.get());
    TEST_ASSERT_EQUAL_PTR(mockGpioSimplified, panel->gpio_);
    TEST_ASSERT_EQUAL_PTR(mockDisplaySimplified, panel->display_);
    TEST_ASSERT_EQUAL_PTR(mockStyleSimplified, panel->style_);
}

void test_simplified_ui_factory_component_null_style() {
    // Test component creation with null style service
    try {
        auto component = SimplifiedUIFactory::createKeyComponent(nullptr);
        TEST_FAIL_MESSAGE("Expected exception for null style service");
    } catch (const std::invalid_argument& e) {
        TEST_ASSERT_TRUE(std::string(e.what()).find("IStyleService") != std::string::npos);
    }
}

void test_simplified_ui_factory_panel_null_dependencies() {
    // Test panel creation with null GPIO provider
    try {
        auto panel = SimplifiedUIFactory::createKeyPanel(nullptr, mockDisplaySimplified, mockStyleSimplified);
        TEST_FAIL_MESSAGE("Expected exception for null GPIO provider");
    } catch (const std::invalid_argument& e) {
        TEST_ASSERT_TRUE(std::string(e.what()).find("IGpioProvider") != std::string::npos);
    }
    
    // Test panel creation with null display provider
    try {
        auto panel = SimplifiedUIFactory::createKeyPanel(mockGpioSimplified, nullptr, mockStyleSimplified);
        TEST_FAIL_MESSAGE("Expected exception for null display provider");
    } catch (const std::invalid_argument& e) {
        TEST_ASSERT_TRUE(std::string(e.what()).find("IDisplayProvider") != std::string::npos);
    }
    
    // Test panel creation with null style service
    try {
        auto panel = SimplifiedUIFactory::createKeyPanel(mockGpioSimplified, mockDisplaySimplified, nullptr);
        TEST_FAIL_MESSAGE("Expected exception for null style service");
    } catch (const std::invalid_argument& e) {
        TEST_ASSERT_TRUE(std::string(e.what()).find("IStyleService") != std::string::npos);
    }
}

void test_simplified_ui_factory_multiple_instances() {
    // Test creating multiple instances of same component type
    auto component1 = SimplifiedUIFactory::createKeyComponent(mockStyleSimplified);
    auto component2 = SimplifiedUIFactory::createKeyComponent(mockStyleSimplified);
    
    TEST_ASSERT_NOT_NULL(component1.get());
    TEST_ASSERT_NOT_NULL(component2.get());
    TEST_ASSERT_NOT_EQUAL(component1.get(), component2.get());
    
    // Test creating multiple instances of same panel type
    auto panel1 = SimplifiedUIFactory::createSplashPanel(mockGpioSimplified, mockDisplaySimplified, mockStyleSimplified);
    auto panel2 = SimplifiedUIFactory::createSplashPanel(mockGpioSimplified, mockDisplaySimplified, mockStyleSimplified);
    
    TEST_ASSERT_NOT_NULL(panel1.get());
    TEST_ASSERT_NOT_NULL(panel2.get());
    TEST_ASSERT_NOT_EQUAL(panel1.get(), panel2.get());
}

void test_simplified_ui_factory_all_components_creation() {
    // Test creating all component types to ensure factory methods work
    auto keyComp = SimplifiedUIFactory::createKeyComponent(mockStyleSimplified);
    auto lockComp = SimplifiedUIFactory::createLockComponent(mockStyleSimplified);
    auto clarityComp = SimplifiedUIFactory::createClarityComponent(mockStyleSimplified);
    auto pressureComp = SimplifiedUIFactory::createOemOilPressureComponent(mockStyleSimplified);
    auto tempComp = SimplifiedUIFactory::createOemOilTemperatureComponent(mockStyleSimplified);
    
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

void test_simplified_ui_factory_all_panels_creation() {
    // Test creating all panel types to ensure factory methods work
    auto keyPanel = SimplifiedUIFactory::createKeyPanel(mockGpioSimplified, mockDisplaySimplified, mockStyleSimplified);
    auto lockPanel = SimplifiedUIFactory::createLockPanel(mockGpioSimplified, mockDisplaySimplified, mockStyleSimplified);
    auto splashPanel = SimplifiedUIFactory::createSplashPanel(mockGpioSimplified, mockDisplaySimplified, mockStyleSimplified);
    auto oilPanel = SimplifiedUIFactory::createOemOilPanel(mockGpioSimplified, mockDisplaySimplified, mockStyleSimplified);
    
    TEST_ASSERT_NOT_NULL(keyPanel.get());
    TEST_ASSERT_NOT_NULL(lockPanel.get());
    TEST_ASSERT_NOT_NULL(splashPanel.get());
    TEST_ASSERT_NOT_NULL(oilPanel.get());
    
    // All should be different instances
    TEST_ASSERT_NOT_EQUAL(keyPanel.get(), lockPanel.get());
    TEST_ASSERT_NOT_EQUAL(lockPanel.get(), splashPanel.get());
    TEST_ASSERT_NOT_EQUAL(splashPanel.get(), oilPanel.get());
}

void test_simplified_ui_factory_memory_management() {
    // Test that objects are properly destroyed when going out of scope
    {
        auto component = SimplifiedUIFactory::createKeyComponent(mockStyleSimplified);
        auto panel = SimplifiedUIFactory::createKeyPanel(mockGpioSimplified, mockDisplaySimplified, mockStyleSimplified);
        TEST_ASSERT_NOT_NULL(component.get());
        TEST_ASSERT_NOT_NULL(panel.get());
        // Objects should be destroyed automatically when leaving scope
    }
    
    // Test multiple creations and destructions
    for (int i = 0; i < 5; i++) {
        auto component = SimplifiedUIFactory::createClarityComponent(mockStyleSimplified);
        auto panel = SimplifiedUIFactory::createSplashPanel(mockGpioSimplified, mockDisplaySimplified, mockStyleSimplified);
        TEST_ASSERT_NOT_NULL(component.get());
        TEST_ASSERT_NOT_NULL(panel.get());
    }
    
    TEST_ASSERT_TRUE(true); // Test passed if no memory leaks/crashes
}

void test_simplified_ui_factory_dependency_injection() {
    // Test that components receive style service dependency correctly
    auto component = SimplifiedUIFactory::createClarityComponent(mockStyleSimplified);
    TEST_ASSERT_NOT_NULL(component.get());
    TEST_ASSERT_EQUAL_PTR(mockStyleSimplified, component->style_service_);
    
    // Test that panels receive all required dependencies correctly
    auto panel = SimplifiedUIFactory::createOemOilPanel(mockGpioSimplified, mockDisplaySimplified, mockStyleSimplified);
    TEST_ASSERT_NOT_NULL(panel.get());
    TEST_ASSERT_EQUAL_PTR(mockGpioSimplified, panel->gpio_);
    TEST_ASSERT_EQUAL_PTR(mockDisplaySimplified, panel->display_);
    TEST_ASSERT_EQUAL_PTR(mockStyleSimplified, panel->style_);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    RUN_TEST(test_simplified_ui_factory_create_key_component);
    RUN_TEST(test_simplified_ui_factory_create_lock_component);
    RUN_TEST(test_simplified_ui_factory_create_clarity_component);
    RUN_TEST(test_simplified_ui_factory_create_oem_oil_pressure_component);
    RUN_TEST(test_simplified_ui_factory_create_oem_oil_temperature_component);
    RUN_TEST(test_simplified_ui_factory_create_key_panel);
    RUN_TEST(test_simplified_ui_factory_create_lock_panel);
    RUN_TEST(test_simplified_ui_factory_create_splash_panel);
    RUN_TEST(test_simplified_ui_factory_create_oem_oil_panel);
    RUN_TEST(test_simplified_ui_factory_component_null_style);
    RUN_TEST(test_simplified_ui_factory_panel_null_dependencies);
    RUN_TEST(test_simplified_ui_factory_multiple_instances);
    RUN_TEST(test_simplified_ui_factory_all_components_creation);
    RUN_TEST(test_simplified_ui_factory_all_panels_creation);
    RUN_TEST(test_simplified_ui_factory_memory_management);
    RUN_TEST(test_simplified_ui_factory_dependency_injection);
    
    return UNITY_END();
}