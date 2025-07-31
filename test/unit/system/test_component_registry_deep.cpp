#include <unity.h>
#include <memory>
#include <string>

// Project Includes
#include "system/service_container.h"
#include "system/component_registry.h"

// Actual Clarity component interfaces and implementations
#include "interfaces/i_component.h"
#include "interfaces/i_sensor.h"
#include "interfaces/i_panel.h"
#include "interfaces/i_display_provider.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_style_service.h"
#include "interfaces/i_preference_service.h"

#include "components/clarity_component.h"
#include "components/key_component.h"
#include "components/lock_component.h"
#include "components/oem/oem_oil_component.h"
#include "components/oem/oem_oil_pressure_component.h"
#include "components/oem/oem_oil_temperature_component.h"

#include "sensors/key_sensor.h"
#include "sensors/lock_sensor.h"
#include "sensors/oil_pressure_sensor.h"
#include "sensors/oil_temperature_sensor.h"

#include "panels/splash_panel.h"
#include "panels/key_panel.h"
#include "panels/lock_panel.h"
#include "panels/oem_oil_panel.h"

#include "managers/panel_manager.h"
#include "managers/preference_manager.h"
#include "managers/style_manager.h"
#include "managers/trigger_manager.h"

// Mock implementations for testing
#include "test_utilities.h"
#include "mock_colors.h"

// Test implementations of interfaces for dependency injection testing
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
    
    void setTestGpioState(int pin, bool state) {
        if (pin >= 0 && pin < 40) {
            pin_states_[pin] = state;
        }
    }
    
    void setTestAnalogValue(int pin, uint16_t value) {
        if (pin >= 0 && pin < 40) {
            analog_values_[pin] = value;
        }
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

// Global container and registry for tests
static std::unique_ptr<ServiceContainer> container;
static std::unique_ptr<ComponentRegistry> registry;

void setUp(void)
{
    container = std::make_unique<ServiceContainer>();
    registry = std::make_unique<ComponentRegistry>(*container);
}

void tearDown(void)
{
    registry.reset();
    container.reset();
}

// =================================================================
// COMPONENT REGISTRY WITH ACTUAL CLARITY COMPONENTS TESTS
// =================================================================

void test_component_registry_register_actual_clarity_components(void)
{
    // Register actual Clarity components
    registry->registerComponent<ClarityComponent>("ClarityComponent");
    registry->registerComponent<KeyComponent>("KeyComponent");
    registry->registerComponent<LockComponent>("LockComponent");
    registry->registerComponent<OemOilPressureComponent>("OemOilPressureComponent");
    registry->registerComponent<OemOilTemperatureComponent>("OemOilTemperatureComponent");
    
    // Verify all components are registered
    TEST_ASSERT_TRUE(registry->isComponentRegistered("ClarityComponent"));
    TEST_ASSERT_TRUE(registry->isComponentRegistered("KeyComponent"));
    TEST_ASSERT_TRUE(registry->isComponentRegistered("LockComponent"));
    TEST_ASSERT_TRUE(registry->isComponentRegistered("OemOilPressureComponent"));
    TEST_ASSERT_TRUE(registry->isComponentRegistered("OemOilTemperatureComponent"));
}

void test_component_registry_create_actual_clarity_components(void)
{
    // Register components
    registry->registerComponent<ClarityComponent>("ClarityComponent");
    registry->registerComponent<KeyComponent>("KeyComponent");
    registry->registerComponent<LockComponent>("LockComponent");
    
    // Create actual component instances
    auto clarity = registry->createComponent("ClarityComponent");
    auto key = registry->createComponent("KeyComponent");
    auto lock = registry->createComponent("LockComponent");
    
    TEST_ASSERT_NOT_NULL(clarity.get());
    TEST_ASSERT_NOT_NULL(key.get());
    TEST_ASSERT_NOT_NULL(lock.get());
    
    // Verify they implement IComponent interface
    IComponent* clarity_iface = dynamic_cast<IComponent*>(clarity.get());
    IComponent* key_iface = dynamic_cast<IComponent*>(key.get());
    IComponent* lock_iface = dynamic_cast<IComponent*>(lock.get());
    
    TEST_ASSERT_NOT_NULL(clarity_iface);
    TEST_ASSERT_NOT_NULL(key_iface);
    TEST_ASSERT_NOT_NULL(lock_iface);
}

void test_component_registry_register_actual_sensors(void)
{
    // Register actual sensor implementations
    registry->registerSensor<KeySensor>("KeySensor");
    registry->registerSensor<LockSensor>("LockSensor");
    registry->registerSensor<OilPressureSensor>("OilPressureSensor");
    registry->registerSensor<OilTemperatureSensor>("OilTemperatureSensor");
    
    // Verify sensors are registered
    TEST_ASSERT_TRUE(registry->isSensorRegistered("KeySensor"));
    TEST_ASSERT_TRUE(registry->isSensorRegistered("LockSensor"));
    TEST_ASSERT_TRUE(registry->isSensorRegistered("OilPressureSensor"));
    TEST_ASSERT_TRUE(registry->isSensorRegistered("OilTemperatureSensor"));
}

void test_component_registry_create_actual_sensors_with_dependencies(void)
{
    // Register GPIO provider dependency
    container->registerSingleton<IGpioProvider>([]() {
        return std::make_unique<TestGpioProvider>();
    });
    
    // Register sensors with dependency injection
    registry->registerSensor<KeySensor>("KeySensor");
    registry->registerSensor<LockSensor>("LockSensor");
    registry->registerSensor<OilPressureSensor>("OilPressureSensor");
    registry->registerSensor<OilTemperatureSensor>("OilTemperatureSensor");
    
    // Create sensor instances
    auto keySensor = registry->createSensor("KeySensor");
    auto lockSensor = registry->createSensor("LockSensor");
    auto pressureSensor = registry->createSensor("OilPressureSensor");
    auto tempSensor = registry->createSensor("OilTemperatureSensor");
    
    TEST_ASSERT_NOT_NULL(keySensor.get());
    TEST_ASSERT_NOT_NULL(lockSensor.get());
    TEST_ASSERT_NOT_NULL(pressureSensor.get());
    TEST_ASSERT_NOT_NULL(tempSensor.get());
    
    // Verify they implement ISensor interface
    ISensor* key_sensor_iface = dynamic_cast<ISensor*>(keySensor.get());
    ISensor* lock_sensor_iface = dynamic_cast<ISensor*>(lockSensor.get());
    ISensor* pressure_sensor_iface = dynamic_cast<ISensor*>(pressureSensor.get());
    ISensor* temp_sensor_iface = dynamic_cast<ISensor*>(tempSensor.get());
    
    TEST_ASSERT_NOT_NULL(key_sensor_iface);
    TEST_ASSERT_NOT_NULL(lock_sensor_iface);
    TEST_ASSERT_NOT_NULL(pressure_sensor_iface);
    TEST_ASSERT_NOT_NULL(temp_sensor_iface);
}

void test_component_registry_register_actual_panels(void)
{
    // Register actual panel implementations
    registry->registerPanel<SplashPanel>("SplashPanel");
    registry->registerPanel<KeyPanel>("KeyPanel");
    registry->registerPanel<LockPanel>("LockPanel");
    registry->registerPanel<OemOilPanel>("OemOilPanel");
    
    // Verify panels are registered
    TEST_ASSERT_TRUE(registry->isPanelRegistered("SplashPanel"));
    TEST_ASSERT_TRUE(registry->isPanelRegistered("KeyPanel"));
    TEST_ASSERT_TRUE(registry->isPanelRegistered("LockPanel"));
    TEST_ASSERT_TRUE(registry->isPanelRegistered("OemOilPanel"));
}

void test_component_registry_create_actual_panels_with_full_dependencies(void)
{
    // Register all required dependencies
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
    
    // Register panels
    registry->registerPanel<SplashPanel>("SplashPanel");
    registry->registerPanel<KeyPanel>("KeyPanel");
    registry->registerPanel<LockPanel>("LockPanel");
    registry->registerPanel<OemOilPanel>("OemOilPanel");
    
    // Create panel instances
    auto splashPanel = registry->createPanel("SplashPanel");
    auto keyPanel = registry->createPanel("KeyPanel");
    auto lockPanel = registry->createPanel("LockPanel");
    auto oilPanel = registry->createPanel("OemOilPanel");
    
    TEST_ASSERT_NOT_NULL(splashPanel.get());
    TEST_ASSERT_NOT_NULL(keyPanel.get());
    TEST_ASSERT_NOT_NULL(lockPanel.get());
    TEST_ASSERT_NOT_NULL(oilPanel.get());
    
    // Verify they implement IPanel interface
    IPanel* splash_panel_iface = dynamic_cast<IPanel*>(splashPanel.get());
    IPanel* key_panel_iface = dynamic_cast<IPanel*>(keyPanel.get());
    IPanel* lock_panel_iface = dynamic_cast<IPanel*>(lockPanel.get());
    IPanel* oil_panel_iface = dynamic_cast<IPanel*>(oilPanel.get());
    
    TEST_ASSERT_NOT_NULL(splash_panel_iface);
    TEST_ASSERT_NOT_NULL(key_panel_iface);
    TEST_ASSERT_NOT_NULL(lock_panel_iface);
    TEST_ASSERT_NOT_NULL(oil_panel_iface);
}

// =================================================================
// DEPENDENCY INJECTION INTEGRATION TESTS
// =================================================================

void test_component_registry_sensor_component_integration(void)
{
    // Setup dependencies
    auto gpioProvider = std::make_shared<TestGpioProvider>();
    container->registerSingleton<IGpioProvider>([gpioProvider]() {
        return std::unique_ptr<IGpioProvider>(gpioProvider.get(), [](IGpioProvider*){});
    });
    
    // Register sensor and component
    registry->registerSensor<KeySensor>("KeySensor");
    registry->registerComponent<KeyComponent>("KeyComponent");
    
    // Create instances
    auto sensor = registry->createSensor("KeySensor");
    auto component = registry->createComponent("KeyComponent");
    
    TEST_ASSERT_NOT_NULL(sensor.get());
    TEST_ASSERT_NOT_NULL(component.get());
    
    // Test sensor-component interaction
    gpioProvider->setTestGpioState(25, true); // Simulate key present
    
    ISensor* sensorInterface = dynamic_cast<ISensor*>(sensor.get());
    TEST_ASSERT_NOT_NULL(sensorInterface);
    
    Reading reading = sensorInterface->read();
    // Component should be able to use this reading
    TEST_ASSERT_TRUE(reading.isValid());
}

void test_component_registry_oil_sensor_component_integration(void)
{
    // Setup dependencies
    auto gpioProvider = std::make_shared<TestGpioProvider>();
    container->registerSingleton<IGpioProvider>([gpioProvider]() {
        return std::unique_ptr<IGpioProvider>(gpioProvider.get(), [](IGpioProvider*){});
    });
    
    // Register oil sensors and components
    registry->registerSensor<OilPressureSensor>("OilPressureSensor");
    registry->registerSensor<OilTemperatureSensor>("OilTemperatureSensor");
    registry->registerComponent<OemOilPressureComponent>("OemOilPressureComponent");
    registry->registerComponent<OemOilTemperatureComponent>("OemOilTemperatureComponent");
    
    // Create instances
    auto pressureSensor = registry->createSensor("OilPressureSensor");
    auto tempSensor = registry->createSensor("OilTemperatureSensor");
    auto pressureComponent = registry->createComponent("OemOilPressureComponent");
    auto tempComponent = registry->createComponent("OemOilTemperatureComponent");
    
    TEST_ASSERT_NOT_NULL(pressureSensor.get());
    TEST_ASSERT_NOT_NULL(tempSensor.get());
    TEST_ASSERT_NOT_NULL(pressureComponent.get());
    TEST_ASSERT_NOT_NULL(tempComponent.get());
    
    // Test sensor readings with components
    gpioProvider->setTestAnalogValue(34, 2048); // Normal oil pressure
    gpioProvider->setTestAnalogValue(35, 1500); // Normal oil temperature
    
    ISensor* pressureSensorInterface = dynamic_cast<ISensor*>(pressureSensor.get());
    ISensor* tempSensorInterface = dynamic_cast<ISensor*>(tempSensor.get());
    
    TEST_ASSERT_NOT_NULL(pressureSensorInterface);
    TEST_ASSERT_NOT_NULL(tempSensorInterface);
    
    Reading pressureReading = pressureSensorInterface->read();
    Reading tempReading = tempSensorInterface->read();
    
    TEST_ASSERT_TRUE(pressureReading.isValid());
    TEST_ASSERT_TRUE(tempReading.isValid());
}

void test_component_registry_full_panel_integration(void)
{
    // Setup all dependencies
    auto gpioProvider = std::make_shared<TestGpioProvider>();
    auto displayProvider = std::make_shared<TestDisplayProvider>();
    auto styleService = std::make_shared<TestStyleService>();
    auto prefService = std::make_shared<TestPreferenceService>();
    
    container->registerSingleton<IGpioProvider>([gpioProvider]() {
        return std::unique_ptr<IGpioProvider>(gpioProvider.get(), [](IGpioProvider*){});
    });
    
    container->registerSingleton<IDisplayProvider>([displayProvider]() {
        return std::unique_ptr<IDisplayProvider>(displayProvider.get(), [](IDisplayProvider*){});
    });
    
    container->registerSingleton<IStyleService>([styleService]() {
        return std::unique_ptr<IStyleService>(styleService.get(), [](IStyleService*){});
    });
    
    container->registerSingleton<IPreferenceService>([prefService]() {
        return std::unique_ptr<IPreferenceService>(prefService.get(), [](IPreferenceService*){});
    });
    
    // Register full OEM Oil Panel with all its dependencies
    registry->registerPanel<OemOilPanel>("OemOilPanel");
    registry->registerComponent<OemOilPressureComponent>("OemOilPressureComponent");
    registry->registerComponent<OemOilTemperatureComponent>("OemOilTemperatureComponent");
    registry->registerSensor<OilPressureSensor>("OilPressureSensor");
    registry->registerSensor<OilTemperatureSensor>("OilTemperatureSensor");
    
    // Create full panel with components and sensors
    auto oilPanel = registry->createPanel("OemOilPanel");
    auto pressureComponent = registry->createComponent("OemOilPressureComponent");
    auto tempComponent = registry->createComponent("OemOilTemperatureComponent");
    auto pressureSensor = registry->createSensor("OilPressureSensor");
    auto tempSensor = registry->createSensor("OilTemperatureSensor");
    
    TEST_ASSERT_NOT_NULL(oilPanel.get());
    TEST_ASSERT_NOT_NULL(pressureComponent.get());
    TEST_ASSERT_NOT_NULL(tempComponent.get());
    TEST_ASSERT_NOT_NULL(pressureSensor.get());
    TEST_ASSERT_NOT_NULL(tempSensor.get());
    
    // Test full integration - panel should be able to use all components
    IPanel* panelInterface = dynamic_cast<IPanel*>(oilPanel.get());
    TEST_ASSERT_NOT_NULL(panelInterface);
    
    // Initialize display provider
    displayProvider->initialize();
    TEST_ASSERT_TRUE(displayProvider->isInitialized());
    
    // Panel should initialize successfully with all dependencies
    panelInterface->init();
    // Panel should be able to load without errors
    panelInterface->load();
}

// =================================================================
// MANAGER INTEGRATION TESTS
// =================================================================

void test_component_registry_manager_integration(void)
{
    // Setup service dependencies
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
    
    // Register managers as singletons in the service container
    container->registerSingleton<PanelManager>([]() {
        return std::make_unique<PanelManager>();
    });
    
    container->registerSingleton<StyleManager>([]() {
        return std::make_unique<StyleManager>();
    });
    
    container->registerSingleton<PreferenceManager>([]() {
        return std::make_unique<PreferenceManager>();
    });
    
    // Test manager resolution
    PanelManager* panelMgr = container->resolve<PanelManager>();
    StyleManager* styleMgr = container->resolve<StyleManager>();
    PreferenceManager* prefMgr = container->resolve<PreferenceManager>();
    
    TEST_ASSERT_NOT_NULL(panelMgr);
    TEST_ASSERT_NOT_NULL(styleMgr);
    TEST_ASSERT_NOT_NULL(prefMgr);
    
    // Test singleton behavior
    PanelManager* panelMgr2 = container->resolve<PanelManager>();
    TEST_ASSERT_EQUAL_PTR(panelMgr, panelMgr2);
}

void test_component_registry_full_system_integration(void)
{
    // This test verifies that the component registry can create a complete
    // working system with all actual Clarity components working together
    
    // Setup all system dependencies
    auto gpioProvider = std::make_shared<TestGpioProvider>();
    auto displayProvider = std::make_shared<TestDisplayProvider>();
    auto styleService = std::make_shared<TestStyleService>();
    auto prefService = std::make_shared<TestPreferenceService>();
    
    container->registerSingleton<IGpioProvider>([gpioProvider]() {
        return std::unique_ptr<IGpioProvider>(gpioProvider.get(), [](IGpioProvider*){});
    });
    
    container->registerSingleton<IDisplayProvider>([displayProvider]() {
        return std::unique_ptr<IDisplayProvider>(displayProvider.get(), [](IDisplayProvider*){});
    });
    
    container->registerSingleton<IStyleService>([styleService]() {
        return std::unique_ptr<IStyleService>(styleService.get(), [](IStyleService*){});
    });
    
    container->registerSingleton<IPreferenceService>([prefService]() {
        return std::unique_ptr<IPreferenceService>(prefService.get(), [](IPreferenceService*){});
    });
    
    // Register all panels
    registry->registerPanel<SplashPanel>("SplashPanel");
    registry->registerPanel<KeyPanel>("KeyPanel");
    registry->registerPanel<LockPanel>("LockPanel");
    registry->registerPanel<OemOilPanel>("OemOilPanel");
    
    // Register all components
    registry->registerComponent<ClarityComponent>("ClarityComponent");
    registry->registerComponent<KeyComponent>("KeyComponent");
    registry->registerComponent<LockComponent>("LockComponent");
    registry->registerComponent<OemOilPressureComponent>("OemOilPressureComponent");
    registry->registerComponent<OemOilTemperatureComponent>("OemOilTemperatureComponent");
    
    // Register all sensors
    registry->registerSensor<KeySensor>("KeySensor");
    registry->registerSensor<LockSensor>("LockSensor");
    registry->registerSensor<OilPressureSensor>("OilPressureSensor");
    registry->registerSensor<OilTemperatureSensor>("OilTemperatureSensor");
    
    // Create complete system
    auto splashPanel = registry->createPanel("SplashPanel");
    auto keyPanel = registry->createPanel("KeyPanel");
    auto lockPanel = registry->createPanel("LockPanel");
    auto oilPanel = registry->createPanel("OemOilPanel");
    
    auto clarityComp = registry->createComponent("ClarityComponent");
    auto keyComp = registry->createComponent("KeyComponent");
    auto lockComp = registry->createComponent("LockComponent");
    auto pressureComp = registry->createComponent("OemOilPressureComponent");
    auto tempComp = registry->createComponent("OemOilTemperatureComponent");
    
    auto keySensor = registry->createSensor("KeySensor");
    auto lockSensor = registry->createSensor("LockSensor");
    auto pressureSensor = registry->createSensor("OilPressureSensor");
    auto tempSensor = registry->createSensor("OilTemperatureSensor");
    
    // Verify all components were created
    TEST_ASSERT_NOT_NULL(splashPanel.get());
    TEST_ASSERT_NOT_NULL(keyPanel.get());
    TEST_ASSERT_NOT_NULL(lockPanel.get());
    TEST_ASSERT_NOT_NULL(oilPanel.get());
    
    TEST_ASSERT_NOT_NULL(clarityComp.get());
    TEST_ASSERT_NOT_NULL(keyComp.get());
    TEST_ASSERT_NOT_NULL(lockComp.get());
    TEST_ASSERT_NOT_NULL(pressureComp.get());
    TEST_ASSERT_NOT_NULL(tempComp.get());
    
    TEST_ASSERT_NOT_NULL(keySensor.get());
    TEST_ASSERT_NOT_NULL(lockSensor.get());
    TEST_ASSERT_NOT_NULL(pressureSensor.get());
    TEST_ASSERT_NOT_NULL(tempSensor.get());
    
    // Initialize system
    displayProvider->initialize();
    prefService->init();
    
    // Test that panels can initialize with their dependencies
    IPanel* splashPanelInterface = dynamic_cast<IPanel*>(splashPanel.get());
    IPanel* keyPanelInterface = dynamic_cast<IPanel*>(keyPanel.get());
    IPanel* lockPanelInterface = dynamic_cast<IPanel*>(lockPanel.get());
    IPanel* oilPanelInterface = dynamic_cast<IPanel*>(oilPanel.get());
    
    TEST_ASSERT_NOT_NULL(splashPanelInterface);
    TEST_ASSERT_NOT_NULL(keyPanelInterface);
    TEST_ASSERT_NOT_NULL(lockPanelInterface);
    TEST_ASSERT_NOT_NULL(oilPanelInterface);
    
    // Initialize all panels (should succeed with proper dependencies)
    splashPanelInterface->init();
    keyPanelInterface->init();
    lockPanelInterface->init();
    oilPanelInterface->init();
    
    // Test sensor readings work
    gpioProvider->setTestGpioState(25, true);  // Key present
    gpioProvider->setTestGpioState(27, false); // Lock not active
    gpioProvider->setTestAnalogValue(34, 2048); // Normal oil pressure
    gpioProvider->setTestAnalogValue(35, 1500); // Normal oil temperature
    
    ISensor* keySensorInterface = dynamic_cast<ISensor*>(keySensor.get());
    ISensor* lockSensorInterface = dynamic_cast<ISensor*>(lockSensor.get());
    ISensor* pressureSensorInterface = dynamic_cast<ISensor*>(pressureSensor.get());
    ISensor* tempSensorInterface = dynamic_cast<ISensor*>(tempSensor.get());
    
    TEST_ASSERT_NOT_NULL(keySensorInterface);
    TEST_ASSERT_NOT_NULL(lockSensorInterface);
    TEST_ASSERT_NOT_NULL(pressureSensorInterface);
    TEST_ASSERT_NOT_NULL(tempSensorInterface);
    
    Reading keyReading = keySensorInterface->read();
    Reading lockReading = lockSensorInterface->read();
    Reading pressureReading = pressureSensorInterface->read();
    Reading tempReading = tempSensorInterface->read();
    
    TEST_ASSERT_TRUE(keyReading.isValid());
    TEST_ASSERT_TRUE(lockReading.isValid());
    TEST_ASSERT_TRUE(pressureReading.isValid());
    TEST_ASSERT_TRUE(tempReading.isValid());
    
    // System integration successful
    TEST_ASSERT_TRUE(true);
}

// Unity test runner setup
void run_component_registry_deep_tests(void)
{
    RUN_TEST(test_component_registry_register_actual_clarity_components);
    RUN_TEST(test_component_registry_create_actual_clarity_components);
    RUN_TEST(test_component_registry_register_actual_sensors);
    RUN_TEST(test_component_registry_create_actual_sensors_with_dependencies);
    RUN_TEST(test_component_registry_register_actual_panels);
    RUN_TEST(test_component_registry_create_actual_panels_with_full_dependencies);
    RUN_TEST(test_component_registry_sensor_component_integration);
    RUN_TEST(test_component_registry_oil_sensor_component_integration);
    RUN_TEST(test_component_registry_full_panel_integration);
    RUN_TEST(test_component_registry_manager_integration);
    RUN_TEST(test_component_registry_full_system_integration);
}

// Note: PlatformIO will automatically discover and run test_ functions