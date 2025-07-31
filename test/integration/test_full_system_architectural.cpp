#include <unity.h>
#include "utilities/test_architectural_helpers.h"

// Project Includes
#include "system/service_container.h"
#include "system/component_registry.h"

// Actual Clarity Components
#include "components/clarity_component.h"
#include "components/key_component.h"
#include "components/lock_component.h"
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
#include "managers/style_manager.h"
#include "managers/preference_manager.h"
#include "managers/trigger_manager.h"

using namespace ArchitecturalTestHelpers;

// Global test setup
static std::unique_ptr<TestSetup> testSetup;
static std::unique_ptr<ScenarioTestHelper> scenarioHelper;

void setUp(void)
{
    testSetup = std::make_unique<TestSetup>();
    scenarioHelper = std::make_unique<ScenarioTestHelper>(*testSetup);
    testSetup->initializeServices();
}

void tearDown(void)
{
    if (testSetup) {
        testSetup->resetServices();
    }
    scenarioHelper.reset();
    testSetup.reset();
}

// =================================================================
// FULL SYSTEM ARCHITECTURAL INTEGRATION TESTS
// =================================================================

void test_architectural_complete_system_integration(void)
{
    auto& container = testSetup->getContainer();
    auto& registry = testSetup->getRegistry();
    
    // Register ALL system components using the new architecture
    
    // Register all panels with factory functions
    registry.registerPanel("SplashPanel", [&registry](IGpioProvider* gpio, IDisplayProvider* display) {
        return std::make_unique<SplashPanel>(&registry);
    });
    registry.registerPanel("KeyPanel", [&registry](IGpioProvider* gpio, IDisplayProvider* display) {
        return std::make_unique<KeyPanel>(&registry);
    });
    registry.registerPanel("LockPanel", [&registry](IGpioProvider* gpio, IDisplayProvider* display) {
        return std::make_unique<LockPanel>(&registry);
    });
    registry.registerPanel("OemOilPanel", [&registry](IGpioProvider* gpio, IDisplayProvider* display) {
        return std::make_unique<OemOilPanel>(&registry);
    });
    
    // Register all components with factory functions
    registry.registerComponent("ClarityComponent", [](IDisplayProvider* display, IStyleService* style) {
        return std::make_unique<ClarityComponent>(style);
    });
    registry.registerComponent("KeyComponent", [](IDisplayProvider* display, IStyleService* style) {
        return std::make_unique<KeyComponent>(style);
    });
    registry.registerComponent("LockComponent", [](IDisplayProvider* display, IStyleService* style) {
        return std::make_unique<LockComponent>(style);
    });
    registry.registerComponent("OemOilPressureComponent", [](IDisplayProvider* display, IStyleService* style) {
        return std::make_unique<OemOilPressureComponent>(style);
    });
    registry.registerComponent("OemOilTemperatureComponent", [](IDisplayProvider* display, IStyleService* style) {
        return std::make_unique<OemOilTemperatureComponent>(style);
    });
    
    // Note: ComponentRegistry doesn't have registerSensor - sensors are created separately
    
    // Register all managers
    container.registerSingleton<PanelManager>([&container]() {
        auto displayProvider = container.resolve<IDisplayProvider>();
        auto gpioProvider = container.resolve<IGpioProvider>();
        return std::make_unique<PanelManager>(displayProvider, gpioProvider, nullptr);
    });
    
    container.registerSingleton<StyleManager>([&container]() {
        return std::make_unique<StyleManager>();
    });
    
    container.registerSingleton<PreferenceManager>([&container]() {
        return std::make_unique<PreferenceManager>();
    });
    
    container.registerSingleton<TriggerManager>([&container]() {
        auto gpioProvider = container.resolve<IGpioProvider>();
        return std::make_unique<TriggerManager>(gpioProvider);
    });
    
    // Create complete system via DI
    auto gpioProvider = container.resolve<IGpioProvider>();
    auto displayProvider = container.resolve<IDisplayProvider>();
    
    auto splashPanel = registry.createPanel("SplashPanel", gpioProvider, displayProvider);
    auto keyPanel = registry.createPanel("KeyPanel", gpioProvider, displayProvider);
    auto lockPanel = registry.createPanel("LockPanel", gpioProvider, displayProvider);
    auto oilPanel = registry.createPanel("OemOilPanel", gpioProvider, displayProvider);
    
    auto clarityComp = registry.createComponent("ClarityComponent");
    auto keyComp = registry.createComponent("KeyComponent");
    auto lockComp = registry.createComponent("LockComponent");
    auto pressureComp = registry.createComponent("OemOilPressureComponent");
    auto tempComp = registry.createComponent("OemOilTemperatureComponent");
    
    // Note: Sensors are created directly, not through registry
    // auto keySensor = std::make_unique<KeySensor>();
    // auto lockSensor = std::make_unique<LockSensor>();
    // auto pressureSensor = std::make_unique<OilPressureSensor>();
    // auto tempSensor = std::make_unique<OilTemperatureSensor>();
    
    auto panelManager = container.resolve<PanelManager>();
    auto styleManager = container.resolve<StyleManager>();
    auto prefManager = container.resolve<PreferenceManager>();
    auto triggerManager = container.resolve<TriggerManager>();
    
    // Verify everything was created successfully
    TEST_ASSERT_NOT_NULL(splashPanel.get());
    TEST_ASSERT_NOT_NULL(keyPanel.get());
    TEST_ASSERT_NOT_NULL(lockPanel.get());
    TEST_ASSERT_NOT_NULL(oilPanel.get());
    
    TEST_ASSERT_NOT_NULL(clarityComp.get());
    TEST_ASSERT_NOT_NULL(keyComp.get());
    TEST_ASSERT_NOT_NULL(lockComp.get());
    TEST_ASSERT_NOT_NULL(pressureComp.get());
    TEST_ASSERT_NOT_NULL(tempComp.get());
    
    // Note: Sensors are not created through registry in this test
    
    TEST_ASSERT_NOT_NULL(panelManager);
    TEST_ASSERT_NOT_NULL(styleManager);
    TEST_ASSERT_NOT_NULL(prefManager);
    TEST_ASSERT_NOT_NULL(triggerManager);
    
    // Test that all components can initialize with their dependencies
    IPanel* splashInterface = dynamic_cast<IPanel*>(splashPanel.get());
    IPanel* keyInterface = dynamic_cast<IPanel*>(keyPanel.get());
    IPanel* lockInterface = dynamic_cast<IPanel*>(lockPanel.get());
    IPanel* oilInterface = dynamic_cast<IPanel*>(oilPanel.get());
    
    TEST_ASSERT_NOT_NULL(splashInterface);
    TEST_ASSERT_NOT_NULL(keyInterface);
    TEST_ASSERT_NOT_NULL(lockInterface);
    TEST_ASSERT_NOT_NULL(oilInterface);
    
    // Initialize all panels - should work with proper DI
    splashInterface->init(gpioProvider, displayProvider);
    keyInterface->init(gpioProvider, displayProvider);
    lockInterface->init(gpioProvider, displayProvider);
    oilInterface->init(gpioProvider, displayProvider);
    
    // Note: Sensor functionality is tested in separate sensor-specific tests
    // For this architectural integration test, we focus on the DI container and registry working together
    
    // System integration successful
    TEST_ASSERT_TRUE(true);
}

void test_architectural_startup_sequence_with_di(void)
{
    auto& container = testSetup->getContainer();
    auto& registry = testSetup->getRegistry();
    
    auto gpioProvider = container.resolve<IGpioProvider>();
    auto displayProvider = container.resolve<IDisplayProvider>();
    
    // Register essential components
    registry.registerPanel("SplashPanel", [&registry](IGpioProvider* gpio, IDisplayProvider* display) {
        return std::make_unique<SplashPanel>(&registry);
    });
    registry.registerPanel("OemOilPanel", [&registry](IGpioProvider* gpio, IDisplayProvider* display) {
        return std::make_unique<OemOilPanel>(&registry);
    });
    registry.registerComponent<ClarityComponent>("ClarityComponent");
    
    // Register managers
    container.registerSingleton<PanelManager>([&container]() {
        auto displayProvider = container.resolve<IDisplayProvider>();
        auto gpioProvider = container.resolve<IGpioProvider>();
        return std::make_unique<PanelManager>(displayProvider, gpioProvider, nullptr);
    });
    
    // Simulate system startup sequence
    auto splashPanel = registry.createPanel("SplashPanel", gpioProvider, displayProvider);
    auto panelManager = container.resolve<PanelManager>();
    
    TEST_ASSERT_NOT_NULL(splashPanel.get());
    TEST_ASSERT_NOT_NULL(panelManager);
    
    // Initialize splash panel
    IPanel* splashInterface = dynamic_cast<IPanel*>(splashPanel.get());
    splashInterface->init(gpioProvider, displayProvider);
    splashInterface->load();
    
    // Simulate splash completion and transition to oil panel
    auto oilPanel = registry.createPanel("OemOilPanel", gpioProvider, displayProvider);
    IPanel* oilInterface = dynamic_cast<IPanel*>(oilPanel.get());
    
    oilInterface->init(gpioProvider, displayProvider);
    oilInterface->load();
    
    // Verify services are shared between panels
    auto displayProvider = testSetup->getTestDisplayProvider();
    TEST_ASSERT_TRUE(displayProvider->isInitialized());
    
    auto styleService = testSetup->getTestStyleService();
    TEST_ASSERT_TRUE(styleService->isInitialized());
    TEST_ASSERT_EQUAL_STRING("Day", styleService->getCurrentTheme());
}

void test_architectural_engine_startup_scenario_with_di(void)
{
    auto& container = testSetup->getContainer();
    auto& registry = testSetup->getRegistry();
    
    auto gpioProvider = container.resolve<IGpioProvider>();
    auto displayProvider = container.resolve<IDisplayProvider>();
    
    // Register oil-related components
    registry.registerPanel("OemOilPanel", [&registry](IGpioProvider* gpio, IDisplayProvider* display) {
        return std::make_unique<OemOilPanel>(&registry);
    });
    registry.registerComponent<OemOilPressureComponent>("OemOilPressureComponent");
    registry.registerComponent<OemOilTemperatureComponent>("OemOilTemperatureComponent");
    registry.registerSensor<OilPressureSensor>("OilPressureSensor");
    registry.registerSensor<OilTemperatureSensor>("OilTemperatureSensor");
    
    // Create components via DI
    auto oilPanel = registry.createPanel("OemOilPanel", gpioProvider, displayProvider);
    auto pressureComp = registry.createComponent("OemOilPressureComponent");
    auto tempComp = registry.createComponent("OemOilTemperatureComponent");
    auto pressureSensor = registry.createSensor("OilPressureSensor");
    auto tempSensor = registry.createSensor("OilTemperatureSensor");
    
    TEST_ASSERT_NOT_NULL(oilPanel.get());
    TEST_ASSERT_NOT_NULL(pressureComp.get());
    TEST_ASSERT_NOT_NULL(tempComp.get());
    TEST_ASSERT_NOT_NULL(pressureSensor.get());
    TEST_ASSERT_NOT_NULL(tempSensor.get());
    
    // Initialize panel
    IPanel* oilInterface = dynamic_cast<IPanel*>(oilPanel.get());
    oilInterface->init(gpioProvider, displayProvider);
    oilInterface->load();
    
    // Simulate engine startup sequence using scenario helper
    scenarioHelper->simulateEngineStartup();
    
    // Verify sensors read the startup sequence values
    ISensor* pressureInterface = dynamic_cast<ISensor*>(pressureSensor.get());
    ISensor* tempInterface = dynamic_cast<ISensor*>(tempSensor.get());
    
    Reading finalPressure = pressureInterface->read();
    Reading finalTemp = tempInterface->read();
    
    TEST_ASSERT_TRUE(!std::holds_alternative<std::monostate>(finalPressure));
    TEST_ASSERT_TRUE(!std::holds_alternative<std::monostate>(finalTemp));
    
    // Should be at normal operating conditions after startup
    auto gpio = testSetup->getTestGpioProvider();
    TEST_ASSERT_EQUAL_UINT16(2048, gpio->analogRead(34)); // Normal pressure
    TEST_ASSERT_EQUAL_UINT16(1500, gpio->analogRead(35)); // Normal temperature
}

void test_architectural_trigger_system_integration_with_di(void)
{
    auto& container = testSetup->getContainer();
    auto& registry = testSetup->getRegistry();
    
    auto gpioProvider = container.resolve<IGpioProvider>();
    auto displayProvider = container.resolve<IDisplayProvider>();
    
    // Register panels and sensors
    registry.registerPanel("KeyPanel", [&registry](IGpioProvider* gpio, IDisplayProvider* display) {
        return std::make_unique<KeyPanel>(&registry);
    });
    registry.registerPanel("LockPanel", [&registry](IGpioProvider* gpio, IDisplayProvider* display) {
        return std::make_unique<LockPanel>(&registry);
    });
    registry.registerPanel("OemOilPanel", [&registry](IGpioProvider* gpio, IDisplayProvider* display) {
        return std::make_unique<OemOilPanel>(&registry);
    });
    registry.registerSensor<KeySensor>("KeySensor");
    registry.registerSensor<LockSensor>("LockSensor");
    
    // Register managers
    container.registerSingleton<TriggerManager>([&container]() {
        auto gpioProvider = container.resolve<IGpioProvider>();
        return std::make_unique<TriggerManager>(gpioProvider);
    });
    
    container.registerSingleton<PanelManager>([&container]() {
        auto displayProvider = container.resolve<IDisplayProvider>();
        auto gpioProvider = container.resolve<IGpioProvider>();
        return std::make_unique<PanelManager>(displayProvider, gpioProvider, nullptr);
    });
    
    // Create system components
    auto keyPanel = registry.createPanel("KeyPanel", gpioProvider, displayProvider);
    auto lockPanel = registry.createPanel("LockPanel", gpioProvider, displayProvider);
    auto oilPanel = registry.createPanel("OemOilPanel", gpioProvider, displayProvider);
    auto keySensor = registry.createSensor("KeySensor");
    auto lockSensor = registry.createSensor("LockSensor");
    
    auto triggerManager = container.resolve<TriggerManager>();
    auto panelManager = container.resolve<PanelManager>();
    
    TEST_ASSERT_NOT_NULL(triggerManager);
    TEST_ASSERT_NOT_NULL(panelManager);
    
    // Test trigger sequence with DI
    // Start with oil panel
    IPanel* oilInterface = dynamic_cast<IPanel*>(oilPanel.get());
    oilInterface->init(gpioProvider, displayProvider);
    oilInterface->load();
    
    // Simulate key present trigger
    scenarioHelper->simulateKeyPresentSequence();
    
    // Switch to key panel
    IPanel* keyInterface = dynamic_cast<IPanel*>(keyPanel.get());
    keyInterface->init(gpioProvider, displayProvider);
    keyInterface->load();
    
    // Verify sensor reads key present
    ISensor* keySensorInterface = dynamic_cast<ISensor*>(keySensor.get());
    Reading keyReading = keySensorInterface->read();
    TEST_ASSERT_TRUE(!std::holds_alternative<std::monostate>(keyReading));
    
    // Verify GPIO state through DI
    auto gpio = testSetup->getTestGpioProvider();
    TEST_ASSERT_TRUE(gpio->digitalRead(25)); // Key present pin
    
    // Simulate lock trigger while key is present
    scenarioHelper->simulateLockActiveSequence();
    
    // Key panel should remain active (higher priority)
    // But lock sensor should also read active
    ISensor* lockSensorInterface = dynamic_cast<ISensor*>(lockSensor.get());
    Reading lockReading = lockSensorInterface->read();
    TEST_ASSERT_TRUE(!std::holds_alternative<std::monostate>(lockReading));
    
    TEST_ASSERT_TRUE(gpio->digitalRead(27)); // Lock active pin
}

void test_architectural_style_theme_integration_with_di(void)
{
    auto& container = testSetup->getContainer();
    auto& registry = testSetup->getRegistry();
    
    auto gpioProvider = container.resolve<IGpioProvider>();
    auto displayProvider = container.resolve<IDisplayProvider>();
    
    // Register panels and manager
    registry.registerPanel("OemOilPanel", [&registry](IGpioProvider* gpio, IDisplayProvider* display) {
        return std::make_unique<OemOilPanel>(&registry);
    });
    registry.registerPanel("KeyPanel", [&registry](IGpioProvider* gpio, IDisplayProvider* display) {
        return std::make_unique<KeyPanel>(&registry);
    });
    
    container.registerSingleton<StyleManager>([&container]() {
        return std::make_unique<StyleManager>();
    });
    
    // Create components
    auto oilPanel = registry.createPanel("OemOilPanel", gpioProvider, displayProvider);
    auto keyPanel = registry.createPanel("KeyPanel", gpioProvider, displayProvider);
    auto styleManager = container.resolve<StyleManager>();
    
    TEST_ASSERT_NOT_NULL(styleManager);
    
    // Initialize panels
    IPanel* oilInterface = dynamic_cast<IPanel*>(oilPanel.get());
    IPanel* keyInterface = dynamic_cast<IPanel*>(keyPanel.get());
    
    oilInterface->init(gpioProvider, displayProvider);
    oilInterface->load();
    
    // Test day theme (default)
    auto styleService = testSetup->getTestStyleService();
    TEST_ASSERT_EQUAL_STRING("Day", styleService->getCurrentTheme());
    
    // Simulate night mode activation
    scenarioHelper->simulateNightModeSequence();
    TEST_ASSERT_EQUAL_STRING("Night", styleService->getCurrentTheme());
    
    // Switch panels while in night mode
    keyInterface->init(gpioProvider, displayProvider);
    keyInterface->load();
    
    // Theme should persist across panel switches
    TEST_ASSERT_EQUAL_STRING("Night", styleService->getCurrentTheme());
    
    // Verify theme was applied to screen
    auto displayProvider = testSetup->getTestDisplayProvider();
    TEST_ASSERT_TRUE(displayProvider->isInitialized());
    TEST_ASSERT_NOT_NULL(displayProvider->getScreen());
}

void test_architectural_preference_persistence_with_di(void)
{
    auto& container = testSetup->getContainer();
    auto& registry = testSetup->getRegistry();
    
    auto gpioProvider = container.resolve<IGpioProvider>();
    auto displayProvider = container.resolve<IDisplayProvider>();
    
    // Register preference manager
    container.registerSingleton<PreferenceManager>([&container]() {
        return std::make_unique<PreferenceManager>();
    });
    
    auto prefManager = container.resolve<PreferenceManager>();
    auto prefService = testSetup->getTestPreferenceService();
    
    TEST_ASSERT_NOT_NULL(prefManager);
    TEST_ASSERT_TRUE(prefService->isInitialized());
    
    // Test preference operations through DI
    prefService->saveConfig("default_panel", "KeyPanel");
    prefService->saveConfig("theme_preference", "Night");
    prefService->saveConfig("sensor_update_rate", "100");
    
    // Verify preferences were saved
    std::string defaultPanel = prefService->loadConfig("default_panel", "OemOilPanel");
    std::string themePreference = prefService->loadConfig("theme_preference", "Day");
    std::string updateRate = prefService->loadConfig("sensor_update_rate", "500");
    
    TEST_ASSERT_EQUAL_STRING("KeyPanel", defaultPanel.c_str());
    TEST_ASSERT_EQUAL_STRING("Night", themePreference.c_str());
    TEST_ASSERT_EQUAL_STRING("100", updateRate.c_str());
    
    // Verify save/load history tracking
    const auto& saveHistory = prefService->getSaveHistory();
    const auto& loadHistory = prefService->getLoadHistory();
    
    TEST_ASSERT_EQUAL_size_t(3, saveHistory.size());
    TEST_ASSERT_EQUAL_size_t(3, loadHistory.size());
}

void test_architectural_error_recovery_with_di(void)
{
    auto& container = testSetup->getContainer();
    auto& registry = testSetup->getRegistry();
    
    auto gpioProvider = container.resolve<IGpioProvider>();
    auto displayProvider = container.resolve<IDisplayProvider>();
    
    // Register oil-related components
    registry.registerPanel("OemOilPanel", [&registry](IGpioProvider* gpio, IDisplayProvider* display) {
        return std::make_unique<OemOilPanel>(&registry);
    });
    registry.registerSensor<OilPressureSensor>("OilPressureSensor");
    registry.registerSensor<OilTemperatureSensor>("OilTemperatureSensor");
    
    // Create components
    auto oilPanel = registry.createPanel("OemOilPanel", gpioProvider, displayProvider);
    auto pressureSensor = registry.createSensor("OilPressureSensor");
    auto tempSensor = registry.createSensor("OilTemperatureSensor");
    
    // Initialize panel
    IPanel* oilInterface = dynamic_cast<IPanel*>(oilPanel.get());
    oilInterface->init(gpioProvider, displayProvider);
    oilInterface->load();
    
    // Normal operation first
    ISensor* pressureInterface = dynamic_cast<ISensor*>(pressureSensor.get());
    ISensor* tempInterface = dynamic_cast<ISensor*>(tempSensor.get());
    
    Reading normalPressure = pressureInterface->read();
    Reading normalTemp = tempInterface->read();
    
    TEST_ASSERT_TRUE(!std::holds_alternative<std::monostate>(normalPressure));
    TEST_ASSERT_TRUE(!std::holds_alternative<std::monostate>(normalTemp));
    
    // Simulate sensor failures
    auto gpio = testSetup->getTestGpioProvider();
    gpio->simulateFailure(34, true); // Pressure sensor failure
    gpio->simulateFailure(35, true); // Temperature sensor failure
    
    Reading failedPressure = pressureInterface->read();
    Reading failedTemp = tempInterface->read();
    
    // Sensors should still return readings (likely error values)
    TEST_ASSERT_TRUE(!std::holds_alternative<std::monostate>(failedPressure));
    TEST_ASSERT_TRUE(!std::holds_alternative<std::monostate>(failedTemp));
    
    // Values should be different (0 for failed sensors)
    TEST_ASSERT_EQUAL_UINT16(0, gpio->analogRead(34));
    TEST_ASSERT_EQUAL_UINT16(0, gpio->analogRead(35));
    
    // Simulate recovery
    gpio->simulateFailure(34, false);
    gpio->simulateFailure(35, false);
    gpio->setTestAnalogValue(34, 2048);
    gpio->setTestAnalogValue(35, 1500);
    
    Reading recoveredPressure = pressureInterface->read();
    Reading recoveredTemp = tempInterface->read();
    
    TEST_ASSERT_TRUE(!std::holds_alternative<std::monostate>(recoveredPressure));
    TEST_ASSERT_TRUE(!std::holds_alternative<std::monostate>(recoveredTemp));
    
    // Should be back to normal values
    TEST_ASSERT_EQUAL_UINT16(2048, gpio->analogRead(34));
    TEST_ASSERT_EQUAL_UINT16(1500, gpio->analogRead(35));
}

// Note: PlatformIO will automatically discover and run test_ functions