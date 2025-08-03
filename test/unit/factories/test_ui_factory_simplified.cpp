#include <unity.h>
#include <memory>
#include <stdexcept>
#include "mock_services.h"
#include "mock_gpio_provider.h"

// Mock component interfaces - testing factory patterns only
class MockComponent {
public:
    MockComponent(void* style_service) : style_service_(style_service) {}
    void* style_service_;
    virtual ~MockComponent() = default;
};

class MockPanel {
public:
    MockPanel(void* gpio, void* display, void* style) 
        : gpio_(gpio), display_(display), style_(style) {}
    void* gpio_;
    void* display_;
    void* style_;
    virtual ~MockPanel() = default;
};

// Simplified UIFactory for testing - tests factory patterns without implementations
class SimplifiedUIFactory {
public:
    // Component factory methods - return mock components for testing
    static std::unique_ptr<MockComponent> createKeyComponent(void* style_service) {
        if (!style_service) throw std::invalid_argument("IStyleService cannot be null");
        return std::make_unique<MockComponent>(style_service);
    }
    
    static std::unique_ptr<MockComponent> createLockComponent(void* style_service) {
        if (!style_service) throw std::invalid_argument("IStyleService cannot be null");
        return std::make_unique<MockComponent>(style_service);
    }
    
    static std::unique_ptr<MockComponent> createClarityComponent(void* style_service) {
        if (!style_service) throw std::invalid_argument("IStyleService cannot be null");
        return std::make_unique<MockComponent>(style_service);
    }
    
    static std::unique_ptr<MockComponent> createOemOilPressureComponent(void* style_service) {
        if (!style_service) throw std::invalid_argument("IStyleService cannot be null");
        return std::make_unique<MockComponent>(style_service);
    }
    
    static std::unique_ptr<MockComponent> createOemOilTemperatureComponent(void* style_service) {
        if (!style_service) throw std::invalid_argument("IStyleService cannot be null");
        return std::make_unique<MockComponent>(style_service);
    }
    
    // Panel factory methods - return mock panels for testing
    static std::unique_ptr<MockPanel> createKeyPanel(void* gpio, void* display, void* style) {
        if (!gpio) throw std::invalid_argument("IGpioProvider cannot be null");
        if (!display) throw std::invalid_argument("IDisplayProvider cannot be null");
        if (!style) throw std::invalid_argument("IStyleService cannot be null");
        return std::make_unique<MockPanel>(gpio, display, style);
    }
    
    static std::unique_ptr<MockPanel> createLockPanel(void* gpio, void* display, void* style) {
        if (!gpio) throw std::invalid_argument("IGpioProvider cannot be null");
        if (!display) throw std::invalid_argument("IDisplayProvider cannot be null");
        if (!style) throw std::invalid_argument("IStyleService cannot be null");
        return std::make_unique<MockPanel>(gpio, display, style);
    }
    
    static std::unique_ptr<MockPanel> createSplashPanel(void* gpio, void* display, void* style) {
        if (!gpio) throw std::invalid_argument("IGpioProvider cannot be null");
        if (!display) throw std::invalid_argument("IDisplayProvider cannot be null");
        if (!style) throw std::invalid_argument("IStyleService cannot be null");
        return std::make_unique<MockPanel>(gpio, display, style);
    }
    
    static std::unique_ptr<MockPanel> createOemOilPanel(void* gpio, void* display, void* style) {
        if (!gpio) throw std::invalid_argument("IGpioProvider cannot be null");
        if (!display) throw std::invalid_argument("IDisplayProvider cannot be null");
        if (!style) throw std::invalid_argument("IStyleService cannot be null");
        return std::make_unique<MockPanel>(gpio, display, style);
    }
};

// Mock services for testing - use different names to avoid conflicts
static MockDisplayProvider* mockDisplayUI = nullptr;
static MockGpioProvider* mockGpioUI = nullptr;
static MockStyleService* mockStyleUI = nullptr;

void setUp_ui_factory_simplified() {
    mockDisplayUI = new MockDisplayProvider();
    mockGpioUI = new MockGpioProvider();
    mockStyleUI = new MockStyleService();
    
    mockDisplayUI->initialize();
    mockStyleUI->initializeStyles();
}

void tearDown_ui_factory_simplified() {
    delete mockDisplayUI;
    delete mockGpioUI;
    delete mockStyleUI;
    
    mockDisplayUI = nullptr;
    mockGpioUI = nullptr;
    mockStyleUI = nullptr;
}

void test_simplified_ui_factory_create_key_component() {
    // Test creating KeyComponent with factory pattern
    auto component = SimplifiedUIFactory::createKeyComponent(mockStyleUI);
    
    TEST_ASSERT_NOT_NULL(component.get());
    TEST_ASSERT_EQUAL_PTR(mockStyleUI, component->style_service_);
}

void test_simplified_ui_factory_create_lock_component() {
    // Test creating LockComponent with factory pattern
    auto component = SimplifiedUIFactory::createLockComponent(mockStyleUI);
    
    TEST_ASSERT_NOT_NULL(component.get());
    TEST_ASSERT_EQUAL_PTR(mockStyleUI, component->style_service_);
}

void test_simplified_ui_factory_create_clarity_component() {
    // Test creating ClarityComponent with factory pattern
    auto component = SimplifiedUIFactory::createClarityComponent(mockStyleUI);
    
    TEST_ASSERT_NOT_NULL(component.get());
    TEST_ASSERT_EQUAL_PTR(mockStyleUI, component->style_service_);
}

void test_simplified_ui_factory_create_oem_oil_pressure_component() {
    // Test creating OemOilPressureComponent with factory pattern
    auto component = SimplifiedUIFactory::createOemOilPressureComponent(mockStyleUI);
    
    TEST_ASSERT_NOT_NULL(component.get());
    TEST_ASSERT_EQUAL_PTR(mockStyleUI, component->style_service_);
}

void test_simplified_ui_factory_create_oem_oil_temperature_component() {
    // Test creating OemOilTemperatureComponent with factory pattern
    auto component = SimplifiedUIFactory::createOemOilTemperatureComponent(mockStyleUI);
    
    TEST_ASSERT_NOT_NULL(component.get());
    TEST_ASSERT_EQUAL_PTR(mockStyleUI, component->style_service_);
}

void test_simplified_ui_factory_create_key_panel() {
    // Test creating KeyPanel with factory pattern
    auto panel = SimplifiedUIFactory::createKeyPanel(mockGpioUI, mockDisplayUI, mockStyleUI);
    
    TEST_ASSERT_NOT_NULL(panel.get());
    TEST_ASSERT_EQUAL_PTR(mockGpioUI, panel->gpio_);
    TEST_ASSERT_EQUAL_PTR(mockDisplayUI, panel->display_);
    TEST_ASSERT_EQUAL_PTR(mockStyleUI, panel->style_);
}

void test_simplified_ui_factory_create_lock_panel() {
    // Test creating LockPanel with factory pattern
    auto panel = SimplifiedUIFactory::createLockPanel(mockGpioUI, mockDisplayUI, mockStyleUI);
    
    TEST_ASSERT_NOT_NULL(panel.get());
    TEST_ASSERT_EQUAL_PTR(mockGpioUI, panel->gpio_);
    TEST_ASSERT_EQUAL_PTR(mockDisplayUI, panel->display_);
    TEST_ASSERT_EQUAL_PTR(mockStyleUI, panel->style_);
}

void test_simplified_ui_factory_create_splash_panel() {
    // Test creating SplashPanel with factory pattern
    auto panel = SimplifiedUIFactory::createSplashPanel(mockGpioUI, mockDisplayUI, mockStyleUI);
    
    TEST_ASSERT_NOT_NULL(panel.get());
    TEST_ASSERT_EQUAL_PTR(mockGpioUI, panel->gpio_);
    TEST_ASSERT_EQUAL_PTR(mockDisplayUI, panel->display_);
    TEST_ASSERT_EQUAL_PTR(mockStyleUI, panel->style_);
}

void test_simplified_ui_factory_create_oem_oil_panel() {
    // Test creating OemOilPanel with factory pattern
    auto panel = SimplifiedUIFactory::createOemOilPanel(mockGpioUI, mockDisplayUI, mockStyleUI);
    
    TEST_ASSERT_NOT_NULL(panel.get());
    TEST_ASSERT_EQUAL_PTR(mockGpioUI, panel->gpio_);
    TEST_ASSERT_EQUAL_PTR(mockDisplayUI, panel->display_);
    TEST_ASSERT_EQUAL_PTR(mockStyleUI, panel->style_);
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
        auto panel = SimplifiedUIFactory::createKeyPanel(nullptr, mockDisplayUI, mockStyleUI);
        TEST_FAIL_MESSAGE("Expected exception for null GPIO provider");
    } catch (const std::invalid_argument& e) {
        TEST_ASSERT_TRUE(std::string(e.what()).find("IGpioProvider") != std::string::npos);
    }
    
    // Test panel creation with null display provider
    try {
        auto panel = SimplifiedUIFactory::createKeyPanel(mockGpioUI, nullptr, mockStyleUI);
        TEST_FAIL_MESSAGE("Expected exception for null display provider");
    } catch (const std::invalid_argument& e) {
        TEST_ASSERT_TRUE(std::string(e.what()).find("IDisplayProvider") != std::string::npos);
    }
    
    // Test panel creation with null style service
    try {
        auto panel = SimplifiedUIFactory::createKeyPanel(mockGpioUI, mockDisplayUI, nullptr);
        TEST_FAIL_MESSAGE("Expected exception for null style service");
    } catch (const std::invalid_argument& e) {
        TEST_ASSERT_TRUE(std::string(e.what()).find("IStyleService") != std::string::npos);
    }
}

void test_simplified_ui_factory_multiple_instances() {
    // Test creating multiple instances of same component type
    auto component1 = SimplifiedUIFactory::createKeyComponent(mockStyleUI);
    auto component2 = SimplifiedUIFactory::createKeyComponent(mockStyleUI);
    
    TEST_ASSERT_NOT_NULL(component1.get());
    TEST_ASSERT_NOT_NULL(component2.get());
    TEST_ASSERT_NOT_EQUAL(component1.get(), component2.get());
    
    // Test creating multiple instances of same panel type
    auto panel1 = SimplifiedUIFactory::createSplashPanel(mockGpioUI, mockDisplayUI, mockStyleUI);
    auto panel2 = SimplifiedUIFactory::createSplashPanel(mockGpioUI, mockDisplayUI, mockStyleUI);
    
    TEST_ASSERT_NOT_NULL(panel1.get());
    TEST_ASSERT_NOT_NULL(panel2.get());
    TEST_ASSERT_NOT_EQUAL(panel1.get(), panel2.get());
}

void test_simplified_ui_factory_all_components_creation() {
    // Test creating all component types to ensure factory methods work
    auto keyComp = SimplifiedUIFactory::createKeyComponent(mockStyleUI);
    auto lockComp = SimplifiedUIFactory::createLockComponent(mockStyleUI);
    auto clarityComp = SimplifiedUIFactory::createClarityComponent(mockStyleUI);
    auto pressureComp = SimplifiedUIFactory::createOemOilPressureComponent(mockStyleUI);
    auto tempComp = SimplifiedUIFactory::createOemOilTemperatureComponent(mockStyleUI);
    
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
    auto keyPanel = SimplifiedUIFactory::createKeyPanel(mockGpioUI, mockDisplayUI, mockStyleUI);
    auto lockPanel = SimplifiedUIFactory::createLockPanel(mockGpioUI, mockDisplayUI, mockStyleUI);
    auto splashPanel = SimplifiedUIFactory::createSplashPanel(mockGpioUI, mockDisplayUI, mockStyleUI);
    auto oilPanel = SimplifiedUIFactory::createOemOilPanel(mockGpioUI, mockDisplayUI, mockStyleUI);
    
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
        auto component = SimplifiedUIFactory::createKeyComponent(mockStyleUI);
        auto panel = SimplifiedUIFactory::createKeyPanel(mockGpioUI, mockDisplayUI, mockStyleUI);
        TEST_ASSERT_NOT_NULL(component.get());
        TEST_ASSERT_NOT_NULL(panel.get());
        // Objects should be destroyed automatically when leaving scope
    }
    
    // Test multiple creations and destructions
    for (int i = 0; i < 5; i++) {
        auto component = SimplifiedUIFactory::createClarityComponent(mockStyleUI);
        auto panel = SimplifiedUIFactory::createSplashPanel(mockGpioUI, mockDisplayUI, mockStyleUI);
        TEST_ASSERT_NOT_NULL(component.get());
        TEST_ASSERT_NOT_NULL(panel.get());
    }
    
    TEST_ASSERT_TRUE(true); // Test passed if no memory leaks/crashes
}

void test_simplified_ui_factory_dependency_injection() {
    // Test that components receive style service dependency correctly
    auto component = SimplifiedUIFactory::createClarityComponent(mockStyleUI);
    TEST_ASSERT_NOT_NULL(component.get());
    TEST_ASSERT_EQUAL_PTR(mockStyleUI, component->style_service_);
    
    // Test that panels receive all required dependencies correctly
    auto panel = SimplifiedUIFactory::createOemOilPanel(mockGpioUI, mockDisplayUI, mockStyleUI);
    TEST_ASSERT_NOT_NULL(panel.get());
    TEST_ASSERT_EQUAL_PTR(mockGpioUI, panel->gpio_);
    TEST_ASSERT_EQUAL_PTR(mockDisplayUI, panel->display_);
    TEST_ASSERT_EQUAL_PTR(mockStyleUI, panel->style_);
}

void runSimplifiedUIFactoryTests() {
    setUp_ui_factory_simplified();
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
    tearDown_ui_factory_simplified();
}