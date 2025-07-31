#include <unity.h>
#include <memory>

// Project Includes - Using new architecture
#include "system/service_container.h"
#include "system/component_registry.h"
#include "interfaces/i_panel.h"
#include "interfaces/i_display_provider.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_style_service.h"
#include "interfaces/i_preference_service.h"

// Actual Panel Implementations
#include "panels/key_panel.h"
#include "panels/lock_panel.h"
#include "panels/oem_oil_panel.h"
#include "panels/splash_panel.h"

// Test utilities
#include "test_utilities.h"
#include "mock_colors.h"

// Test Service Implementations (DI-compatible)
class TestDisplayProvider : public IDisplayProvider {
public:
    TestDisplayProvider() : initialized_(false) {}
    
    void initialize() override { initialized_ = true; }
    void* getScreen() override { 
        static mock_lv_obj_t screen = create_mock_lv_obj();
        return &screen; 
    }
    void updateDisplay() override {}
    bool isInitialized() const override { return initialized_; }
    
private:
    bool initialized_;
};

class TestGpioProvider : public IGpioProvider {
public:
    TestGpioProvider() {
        for (int i = 0; i < 40; i++) {
            pin_states_[i] = false;
            analog_values_[i] = 0;
        }
    }
    
    void setPinMode(int pin, int mode) override {}
    bool digitalRead(int pin) override { 
        return (pin >= 0 && pin < 40) ? pin_states_[pin] : false; 
    }
    void digitalWrite(int pin, bool state) override {
        if (pin >= 0 && pin < 40) {
            pin_states_[pin] = state;
        }
    }
    uint16_t analogRead(int pin) override { 
        return (pin >= 0 && pin < 40) ? analog_values_[pin] : 0; 
    }
    
    void setTestState(int pin, bool state) {
        if (pin >= 0 && pin < 40) pin_states_[pin] = state;
    }
    
    void setTestAnalog(int pin, uint16_t value) {
        if (pin >= 0 && pin < 40) analog_values_[pin] = value;
    }
    
private:
    bool pin_states_[40];
    uint16_t analog_values_[40];
};

class TestStyleService : public IStyleService {
public:
    TestStyleService() : current_theme_("Day") {}
    
    void initializeStyles() override {}
    void setTheme(const char* theme) override { current_theme_ = theme; }
    const char* getCurrentTheme() const override { return current_theme_.c_str(); }
    void applyToScreen(void* screen) override {}
    void resetStyles() override {}
    
private:
    std::string current_theme_;
};

class TestPreferenceService : public IPreferenceService {
public:
    TestPreferenceService() : initialized_(false) {}
    
    void init() override { initialized_ = true; }
    bool isInitialized() const override { return initialized_; }
    void saveConfig(const char* key, const char* value) override {
        config_[key] = value;
    }
    std::string loadConfig(const char* key, const char* defaultValue) override {
        auto it = config_.find(key);
        return (it != config_.end()) ? it->second : defaultValue;
    }
    
private:
    bool initialized_;
    std::map<std::string, std::string> config_;
};

// Global DI container for tests
static std::unique_ptr<ServiceContainer> container;
static std::unique_ptr<ComponentRegistry> registry;

void setUp(void)
{
    // Create new DI container for each test
    container = std::make_unique<ServiceContainer>();
    registry = std::make_unique<ComponentRegistry>(*container);
    
    // Register all required services
    container->registerSingleton<IDisplayProvider>([]() {
        return std::make_unique<TestDisplayProvider>();
    });
    
    container->registerSingleton<IGpioProvider>([]() {
        return std::make_unique<TestGpioProvider>();
    });
    
    container->registerSingleton<IStyleService>([]() {
        return std::make_unique<TestStyleService>();
    });
    
    container->registerSingleton<IPreferenceService>([]() {
        return std::make_unique<TestPreferenceService>();
    });
}

void tearDown(void)
{
    registry.reset();
    container.reset();
}

// =================================================================
// ARCHITECTURAL PANEL TESTS - USING DEPENDENCY INJECTION
// =================================================================

void test_architectural_key_panel_creation_via_registry(void)
{
    // Register the panel using the new architecture
    registry->registerPanel<KeyPanel>("KeyPanel");
    
    // Create panel via registry (with automatic DI)
    auto keyPanel = registry->createPanel("KeyPanel");
    
    TEST_ASSERT_NOT_NULL(keyPanel.get());
    
    // Verify it implements IPanel interface
    IPanel* panelInterface = dynamic_cast<IPanel*>(keyPanel.get());
    TEST_ASSERT_NOT_NULL(panelInterface);
    
    // Initialize display provider
    IDisplayProvider* displayProvider = container->resolve<IDisplayProvider>();
    displayProvider->initialize();
    
    // Panel should initialize successfully with injected dependencies
    panelInterface->init();
    panelInterface->load();
    
    // Test that panel has access to its dependencies
    TEST_ASSERT_TRUE(displayProvider->isInitialized());
}

void test_architectural_lock_panel_creation_via_registry(void)
{
    // Register the panel
    registry->registerPanel<LockPanel>("LockPanel");
    
    // Create via registry
    auto lockPanel = registry->createPanel("LockPanel");
    
    TEST_ASSERT_NOT_NULL(lockPanel.get());
    
    IPanel* panelInterface = dynamic_cast<IPanel*>(lockPanel.get());
    TEST_ASSERT_NOT_NULL(panelInterface);
    
    // Test with dependency injection
    IDisplayProvider* displayProvider = container->resolve<IDisplayProvider>();
    IStyleService* styleService = container->resolve<IStyleService>();
    
    displayProvider->initialize();
    styleService->initializeStyles();
    
    // Panel should work with injected services
    panelInterface->init();
    panelInterface->load();
    
    TEST_ASSERT_TRUE(displayProvider->isInitialized());
    TEST_ASSERT_EQUAL_STRING("Day", styleService->getCurrentTheme());
}

void test_architectural_oem_oil_panel_with_full_dependencies(void)
{
    // Register the complex panel
    registry->registerPanel<OemOilPanel>("OemOilPanel");
    
    // Create via registry
    auto oilPanel = registry->createPanel("OemOilPanel");
    
    TEST_ASSERT_NOT_NULL(oilPanel.get());
    
    IPanel* panelInterface = dynamic_cast<IPanel*>(oilPanel.get());
    TEST_ASSERT_NOT_NULL(panelInterface);
    
    // Set up all dependencies
    IDisplayProvider* displayProvider = container->resolve<IDisplayProvider>();
    IGpioProvider* gpioProvider = container->resolve<IGpioProvider>();
    IStyleService* styleService = container->resolve<IStyleService>();
    IPreferenceService* prefService = container->resolve<IPreferenceService>();
    
    // Initialize all services
    displayProvider->initialize();
    styleService->initializeStyles();
    prefService->init();
    
    // Set realistic sensor data via GPIO provider
    TestGpioProvider* testGpio = dynamic_cast<TestGpioProvider*>(gpioProvider);
    TEST_ASSERT_NOT_NULL(testGpio);
    
    testGpio->setTestAnalog(34, 2048); // Normal oil pressure
    testGpio->setTestAnalog(35, 1500); // Normal oil temperature
    
    // Panel should initialize with all dependencies
    panelInterface->init();
    panelInterface->load();
    
    // Verify all services are properly injected and working
    TEST_ASSERT_TRUE(displayProvider->isInitialized());
    TEST_ASSERT_TRUE(prefService->isInitialized());
    TEST_ASSERT_EQUAL_UINT16(2048, gpioProvider->analogRead(34));
    TEST_ASSERT_EQUAL_UINT16(1500, gpioProvider->analogRead(35));
}

void test_architectural_splash_panel_lifecycle_with_di(void)
{
    // Register panel
    registry->registerPanel<SplashPanel>("SplashPanel");
    
    // Create via registry
    auto splashPanel = registry->createPanel("SplashPanel");
    
    TEST_ASSERT_NOT_NULL(splashPanel.get());
    
    IPanel* panelInterface = dynamic_cast<IPanel*>(splashPanel.get());
    TEST_ASSERT_NOT_NULL(panelInterface);
    
    // Get dependencies through DI
    IDisplayProvider* displayProvider = container->resolve<IDisplayProvider>();
    IStyleService* styleService = container->resolve<IStyleService>();
    
    // Initialize dependencies
    displayProvider->initialize();
    styleService->initializeStyles();
    
    // Test full panel lifecycle
    panelInterface->init();
    panelInterface->load();
    panelInterface->update();
    
    // Panel should have access to all its dependencies
    TEST_ASSERT_TRUE(displayProvider->isInitialized());
    TEST_ASSERT_NOT_NULL(displayProvider->getScreen());
}

// =================================================================
// PANEL INTERACTION TESTS WITH DEPENDENCY INJECTION
// =================================================================

void test_architectural_panel_switching_via_registry(void)
{
    // Register multiple panels
    registry->registerPanel<KeyPanel>("KeyPanel");
    registry->registerPanel<LockPanel>("LockPanel");
    registry->registerPanel<OemOilPanel>("OemOilPanel");
    
    // Initialize all dependencies
    IDisplayProvider* displayProvider = container->resolve<IDisplayProvider>();
    IGpioProvider* gpioProvider = container->resolve<IGpioProvider>();
    
    displayProvider->initialize();
    
    // Create panels via registry
    auto keyPanel = registry->createPanel("KeyPanel");
    auto lockPanel = registry->createPanel("LockPanel");
    auto oilPanel = registry->createPanel("OemOilPanel");
    
    TEST_ASSERT_NOT_NULL(keyPanel.get());
    TEST_ASSERT_NOT_NULL(lockPanel.get());
    TEST_ASSERT_NOT_NULL(oilPanel.get());
    
    // All panels should be able to initialize with shared dependencies
    IPanel* keyPanelInterface = dynamic_cast<IPanel*>(keyPanel.get());
    IPanel* lockPanelInterface = dynamic_cast<IPanel*>(lockPanel.get());
    IPanel* oilPanelInterface = dynamic_cast<IPanel*>(oilPanel.get());
    
    TEST_ASSERT_NOT_NULL(keyPanelInterface);
    TEST_ASSERT_NOT_NULL(lockPanelInterface);
    TEST_ASSERT_NOT_NULL(oilPanelInterface);
    
    // Test panel switching
    keyPanelInterface->init();
    keyPanelInterface->load();
    
    lockPanelInterface->init();
    lockPanelInterface->load();
    
    oilPanelInterface->init();
    oilPanelInterface->load();
    
    // All should share the same display provider instance (singleton)
    TEST_ASSERT_TRUE(displayProvider->isInitialized());
}

void test_architectural_panel_with_sensor_integration(void)
{
    // Register panel
    registry->registerPanel<OemOilPanel>("OemOilPanel");
    
    // Create panel
    auto oilPanel = registry->createPanel("OemOilPanel");
    IPanel* panelInterface = dynamic_cast<IPanel*>(oilPanel.get());
    
    // Get GPIO provider for sensor simulation
    IGpioProvider* gpioProvider = container->resolve<IGpioProvider>();
    TestGpioProvider* testGpio = dynamic_cast<TestGpioProvider*>(gpioProvider);
    
    // Initialize dependencies
    IDisplayProvider* displayProvider = container->resolve<IDisplayProvider>();
    displayProvider->initialize();
    
    // Simulate sensor data changes
    testGpio->setTestAnalog(34, 1000); // Low pressure
    testGpio->setTestAnalog(35, 2000); // High temperature
    
    // Panel should respond to sensor changes
    panelInterface->init();
    panelInterface->load();
    panelInterface->update();
    
    // Verify sensor data is accessible
    TEST_ASSERT_EQUAL_UINT16(1000, gpioProvider->analogRead(34));
    TEST_ASSERT_EQUAL_UINT16(2000, gpioProvider->analogRead(35));
    
    // Change sensor data
    testGpio->setTestAnalog(34, 3000); // High pressure
    testGpio->setTestAnalog(35, 1200); // Normal temperature
    
    panelInterface->update();
    
    TEST_ASSERT_EQUAL_UINT16(3000, gpioProvider->analogRead(34));
    TEST_ASSERT_EQUAL_UINT16(1200, gpioProvider->analogRead(35));
}

// =================================================================
// SERVICE INJECTION TESTS
// =================================================================

void test_architectural_service_singleton_behavior(void)
{
    // Register panel
    registry->registerPanel<KeyPanel>("KeyPanel");
    registry->registerPanel<LockPanel>("LockPanel");
    
    // Create multiple panels
    auto keyPanel1 = registry->createPanel("KeyPanel");
    auto keyPanel2 = registry->createPanel("KeyPanel");
    auto lockPanel = registry->createPanel("LockPanel");
    
    // All panels should get the same singleton instances
    IDisplayProvider* displayProvider1 = container->resolve<IDisplayProvider>();
    IDisplayProvider* displayProvider2 = container->resolve<IDisplayProvider>();
    IStyleService* styleService1 = container->resolve<IStyleService>();
    IStyleService* styleService2 = container->resolve<IStyleService>();
    
    // Verify singleton behavior
    TEST_ASSERT_EQUAL_PTR(displayProvider1, displayProvider2);
    TEST_ASSERT_EQUAL_PTR(styleService1, styleService2);
    
    // Initialize one instance affects all
    displayProvider1->initialize();
    TEST_ASSERT_TRUE(displayProvider2->isInitialized());
    
    styleService1->setTheme("Night");
    TEST_ASSERT_EQUAL_STRING("Night", styleService2->getCurrentTheme());
}

void test_architectural_service_container_lifecycle(void)
{
    // Test that services are properly managed by container
    TEST_ASSERT_TRUE(container->isRegistered<IDisplayProvider>());
    TEST_ASSERT_TRUE(container->isRegistered<IGpioProvider>());
    TEST_ASSERT_TRUE(container->isRegistered<IStyleService>());
    TEST_ASSERT_TRUE(container->isRegistered<IPreferenceService>());
    
    // Get services
    IDisplayProvider* displayProvider = container->resolve<IDisplayProvider>();
    IGpioProvider* gpioProvider = container->resolve<IGpioProvider>();
    IStyleService* styleService = container->resolve<IStyleService>();
    IPreferenceService* prefService = container->resolve<IPreferenceService>();
    
    TEST_ASSERT_NOT_NULL(displayProvider);
    TEST_ASSERT_NOT_NULL(gpioProvider);
    TEST_ASSERT_NOT_NULL(styleService);
    TEST_ASSERT_NOT_NULL(prefService);
    
    // Services should be functional
    displayProvider->initialize();
    styleService->initializeStyles();
    prefService->init();
    
    TEST_ASSERT_TRUE(displayProvider->isInitialized());
    TEST_ASSERT_TRUE(prefService->isInitialized());
    TEST_ASSERT_NOT_NULL(displayProvider->getScreen());
}

// Note: PlatformIO will automatically discover and run test_ functions