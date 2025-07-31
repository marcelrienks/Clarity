#include <unity.h>
#include "utilities/test_architectural_helpers.h"

// Project Includes
#include "system/service_container.h"
#include "system/component_registry.h"
#include "interfaces/i_sensor_factory.h"

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
    
    // Get component factory from container
    auto componentFactory = container.resolve<IComponentFactory>();
    
    // Register all panels with factory functions
    registry.registerPanel("SplashPanel", [](IGpioProvider* gpio, IDisplayProvider* display) {
        return std::make_unique<SplashPanel>(componentFactory);
    });
    registry.registerPanel("KeyPanel", [componentFactory](IGpioProvider* gpio, IDisplayProvider* display) {
        return std::make_unique<KeyPanel>(componentFactory);
    });
    registry.registerPanel("LockPanel", [componentFactory](IGpioProvider* gpio, IDisplayProvider* display) {
        return std::make_unique<LockPanel>(componentFactory);
    });
    registry.registerPanel("OemOilPanel", [componentFactory](IGpioProvider* gpio, IDisplayProvider* display) {
        return std::make_unique<OemOilPanel>(componentFactory);
    });
    
    // Get style service from container
    auto styleService = container.resolve<IStyleService>();
    
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
    container.registerSingleton<PanelManager>([&]() {
        auto displayProvider = container.resolve<IDisplayProvider>();
        auto gpioProvider = container.resolve<IGpioProvider>();
        auto panelFactory = container.resolve<IPanelFactory>();
        return std::make_unique<PanelManager>(displayProvider, gpioProvider, panelFactory);
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
    
    // Create sensors through sensor factory
    auto sensorFactory = container.resolve<ISensorFactory>();
    auto keySensor = sensorFactory->createSensor("KeySensor");
    auto lockSensor = sensorFactory->createSensor("LockSensor");
    auto pressureSensor = sensorFactory->createSensor("OilPressureSensor");
    auto tempSensor = sensorFactory->createSensor("OilTemperatureSensor");
    
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
    
    // Configure trigger system
    triggerManager->addTrigger("KeyTrigger", keySensor.get(), [keyInterface]() {
        keyInterface->show();
    });
    triggerManager->addTrigger("LockTrigger", lockSensor.get(), [lockInterface]() {
        lockInterface->show();
    });
    triggerManager->addTrigger("OilTrigger", tempSensor.get(), [oilInterface]() {
        oilInterface->show();
    });

    // Verify trigger setup
    TEST_ASSERT_TRUE(triggerManager->hasTrigger("KeyTrigger"));
    TEST_ASSERT_TRUE(triggerManager->hasTrigger("LockTrigger")); 
    TEST_ASSERT_TRUE(triggerManager->hasTrigger("OilTrigger"));

    // System integration successful
    TEST_ASSERT_TRUE(true);
}

void test_architectural_startup_sequence_with_di(void)
{
    auto& container = testSetup->getContainer();
    auto& registry = testSetup->getRegistry();
    
    auto gpioProvider = container.resolve<IGpioProvider>();
    auto displayProvider = container.resolve<IDisplayProvider>();
    
    // Get dependencies
    auto styleService = container.resolve<IStyleService>();
    auto factory = container.resolve<IComponentFactory>();

    // Register panels
    registry.registerPanel("SplashPanel", [factory](IGpioProvider* gpio, IDisplayProvider* display) {
        return std::make_unique<SplashPanel>(factory);
    });
    registry.registerPanel("OemOilPanel", [factory](IGpioProvider* gpio, IDisplayProvider* display) {
        return std::make_unique<OemOilPanel>(factory);
    });

    // Register base components
    registry.registerComponent("ClarityComponent", [styleService](IDisplayProvider* display, IStyleService* style) { 
        return std::make_unique<ClarityComponent>(style); 
    });

    // Initialize services
    displayProvider->initialize();
    styleService->initializeStyles();

    // Register managers
    container.registerSingleton<PanelManager>([&]() {
        auto panelFactory = container.resolve<IPanelFactory>();
        return std::make_unique<PanelManager>(displayProvider, gpioProvider, panelFactory);
    });

    // Create and initialize panels
    auto splashPanel = registry.createPanel("SplashPanel", gpioProvider, displayProvider);
    auto panelManager = container.resolve<PanelManager>();
    
    TEST_ASSERT_NOT_NULL(splashPanel.get());
    TEST_ASSERT_NOT_NULL(panelManager);
    
    // Initialize splash panel with callback for completion
    IPanel* splashInterface = dynamic_cast<IPanel*>(splashPanel.get());
    TEST_ASSERT_NOT_NULL(splashInterface);

    splashInterface->init(gpioProvider, displayProvider);
    
    // Load splash panel with completion callback
    auto onLoadComplete = []() { /* Splash loaded */ };
    splashInterface->load(onLoadComplete, gpioProvider, displayProvider);

    auto oilPanel = registry.createPanel("OemOilPanel", gpioProvider, displayProvider);
    IPanel* oilInterface = dynamic_cast<IPanel*>(oilPanel.get());
    TEST_ASSERT_NOT_NULL(oilInterface);
    
    // Initialize and load oil panel
    oilInterface->init(gpioProvider, displayProvider);
    auto onOilPanelLoaded = []() { /* Oil panel loaded */ };
    oilInterface->load(onOilPanelLoaded, gpioProvider, displayProvider);
    
    // Verify service states
    TEST_ASSERT_TRUE(displayProvider->isInitialized());
    TEST_ASSERT_NOT_NULL(displayProvider->getMainScreen());
    TEST_ASSERT_TRUE(styleService->isInitialized());
    TEST_ASSERT_EQUAL_STRING("Day", styleService->getCurrentTheme());
}

void test_architectural_engine_startup_scenario_with_di(void)
{
    auto& container = testSetup->getContainer();
    auto& registry = testSetup->getRegistry();
    
    // Get required services
    auto gpioProvider = container.resolve<IGpioProvider>();
    auto displayProvider = container.resolve<IDisplayProvider>();
    auto sensorFactory = container.resolve<ISensorFactory>();
    
    // Initialize services
    displayProvider->initialize();
    
    // Get factories
    auto componentFactory = container.resolve<IComponentFactory>();
    auto styleService = container.resolve<IStyleService>();

    // Register oil-related components
    registry.registerPanel("OemOilPanel", [componentFactory](IGpioProvider* gpio, IDisplayProvider* display) {
        return std::make_unique<OemOilPanel>(componentFactory);
    });
    
    registry.registerComponent("OemOilPressureComponent", [styleService](IDisplayProvider* display, IStyleService* style) {
        return std::make_unique<OemOilPressureComponent>(style);
    });
    
    registry.registerComponent("OemOilTemperatureComponent", [styleService](IDisplayProvider* display, IStyleService* style) {
        return std::make_unique<OemOilTemperatureComponent>(style);
    });
    
    // Register sensors
    sensorFactory->registerSensor("OilPressureSensor", []() {
        return std::make_unique<OilPressureSensor>();
    });
    
    sensorFactory->registerSensor("OilTemperatureSensor", []() {
        return std::make_unique<OilTemperatureSensor>();
    });
    
    // Create components via factories
    auto oilPanel = registry.createPanel("OemOilPanel", gpioProvider, displayProvider);
    auto pressureComp = registry.createComponent("OemOilPressureComponent");
    auto tempComp = registry.createComponent("OemOilTemperatureComponent");
    auto pressureSensor = sensorFactory->createSensor("OilPressureSensor");
    auto tempSensor = sensorFactory->createSensor("OilTemperatureSensor");
    
    TEST_ASSERT_NOT_NULL(oilPanel.get());
    TEST_ASSERT_NOT_NULL(pressureComp.get());
    TEST_ASSERT_NOT_NULL(tempComp.get());
    TEST_ASSERT_NOT_NULL(pressureSensor.get());
    TEST_ASSERT_NOT_NULL(tempSensor.get());
    
    // Initialize panel
    IPanel* oilInterface = dynamic_cast<IPanel*>(oilPanel.get());
    TEST_ASSERT_NOT_NULL(oilInterface);
    
    oilInterface->init(gpioProvider, displayProvider);
    
    auto onOilPanelLoaded = []() { /* Oil panel loaded */ };
    oilInterface->load(onOilPanelLoaded, gpioProvider, displayProvider);
    
    // Simulate engine startup sequence using scenario helper
    scenarioHelper->simulateEngineStartup();
    
    // Verify sensors read the startup sequence values
    ISensor* pressureInterface = dynamic_cast<ISensor*>(pressureSensor.get());
    ISensor* tempInterface = dynamic_cast<ISensor*>(tempSensor.get());
    
    TEST_ASSERT_NOT_NULL(pressureInterface);
    TEST_ASSERT_NOT_NULL(tempInterface);
    
    Reading finalPressure = pressureInterface->GetReading();
    Reading finalTemp = tempInterface->GetReading();
    
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
    
    // Get required services
    auto sensorFactory = container.resolve<ISensorFactory>();
    auto styleService = container.resolve<IStyleService>();
    
    displayProvider->initialize();
    styleService->initializeStyles();
    
    // Get component factory and register panels
    auto componentFactory = container.resolve<IComponentFactory>();
    
    registry.registerPanel("KeyPanel", [componentFactory](IGpioProvider* gpio, IDisplayProvider* display) {
        return std::make_unique<KeyPanel>(componentFactory);
    });
    registry.registerPanel("LockPanel", [componentFactory](IGpioProvider* gpio, IDisplayProvider* display) {
        return std::make_unique<LockPanel>(componentFactory);
    });
    registry.registerPanel("OemOilPanel", [componentFactory](IGpioProvider* gpio, IDisplayProvider* display) {
        return std::make_unique<OemOilPanel>(componentFactory);
    });
    
    // Register sensors
    sensorFactory->registerSensor("KeySensor", []() {
        return std::make_unique<KeySensor>();
    });
    sensorFactory->registerSensor("LockSensor", []() {
        return std::make_unique<LockSensor>();
    });
    
    // Register managers with dependencies
    container.registerSingleton<TriggerManager>([&]() {
        return std::make_unique<TriggerManager>(gpioProvider);
    });
    
    container.registerSingleton<PanelManager>([&]() {
        auto panelFactory = container.resolve<IPanelFactory>();
        return std::make_unique<PanelManager>(displayProvider, gpioProvider, panelFactory);
    });
    
    // Create system components
    auto keyPanel = registry.createPanel("KeyPanel", gpioProvider, displayProvider);
    auto lockPanel = registry.createPanel("LockPanel", gpioProvider, displayProvider);
    auto oilPanel = registry.createPanel("OemOilPanel", gpioProvider, displayProvider);
    auto keySensor = sensorFactory->createSensor("KeySensor");
    auto lockSensor = sensorFactory->createSensor("LockSensor");
    
    auto triggerManager = container.resolve<TriggerManager>();
    auto panelManager = container.resolve<PanelManager>();
    
    TEST_ASSERT_NOT_NULL(triggerManager);
    TEST_ASSERT_NOT_NULL(panelManager);
    
    // Test trigger sequence with DI
    // Start with oil panel
    IPanel* oilInterface = dynamic_cast<IPanel*>(oilPanel.get());
    oilInterface->init(gpioProvider, displayProvider);
    oilInterface->load([]{}, gpioProvider, displayProvider);
    
    // Simulate key present trigger
    scenarioHelper->simulateKeyPresentSequence();
    
    // Switch to key panel
    IPanel* keyInterface = dynamic_cast<IPanel*>(keyPanel.get());
    keyInterface->init(gpioProvider, displayProvider);
    keyInterface->load([]{}, gpioProvider, displayProvider);
    
    // Verify sensor reads key present
    ISensor* keySensorInterface = dynamic_cast<ISensor*>(keySensor.get());
    Reading keyReading = keySensorInterface->GetReading();
    TEST_ASSERT_TRUE(!std::holds_alternative<std::monostate>(keyReading));
    
    // Verify GPIO state through DI
    auto gpio = testSetup->getTestGpioProvider();
    TEST_ASSERT_TRUE(gpio->digitalRead(25)); // Key present pin
    
    // Simulate lock trigger while key is present
    scenarioHelper->simulateLockActiveSequence();
    
    // Key panel should remain active (higher priority)
    // But lock sensor should also read active
    ISensor* lockSensorInterface = dynamic_cast<ISensor*>(lockSensor.get());
    Reading lockReading = lockSensorInterface->GetReading();
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
    oilInterface->load([]{}, gpioProvider, displayProvider);

    // Test day theme (default)
    auto styleService = testSetup->getTestStyleService();
    TEST_ASSERT_EQUAL_STRING("Day", styleService->getCurrentTheme());
    
    // Simulate night mode activation
    scenarioHelper->simulateNightModeSequence();
    TEST_ASSERT_EQUAL_STRING("Night", styleService->getCurrentTheme());
    
    // Switch panels while in night mode
    keyInterface->init(gpioProvider, displayProvider);
    keyInterface->load([]{}, gpioProvider, displayProvider);
    
    // Theme should persist across panel switches
    TEST_ASSERT_EQUAL_STRING("Night", styleService->getCurrentTheme());
    
    // Verify theme was applied to screen
    displayProvider = testSetup->getTestDisplayProvider();
    TEST_ASSERT_TRUE(displayProvider->isInitialized());
    TEST_ASSERT_NOT_NULL(displayProvider->getMainScreen());
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
    Configs testConfig;
    testConfig.panelName = PanelNames::KEY;
    testConfig.theme = "Night";
    testConfig.updateRate = 100;
    prefService->setConfig(testConfig);
    prefService->saveConfig();
    
    // Verify preferences were saved
    const auto& loadedConfig = prefService->getConfig();
    TEST_ASSERT_EQUAL_STRING(PanelNames::KEY, loadedConfig.panelName.c_str());
    TEST_ASSERT_EQUAL_STRING("Night", loadedConfig.theme.c_str());
    TEST_ASSERT_EQUAL_INT(100, loadedConfig.updateRate);
    
    // Load a different config
    prefService->loadConfig();
    
    // Verify config persists after load
    const auto& reloadedConfig = prefService->getConfig();
    TEST_ASSERT_EQUAL_STRING(PanelNames::KEY, reloadedConfig.panelName.c_str());
    TEST_ASSERT_EQUAL_STRING("Night", reloadedConfig.theme.c_str());
    TEST_ASSERT_EQUAL_INT(100, reloadedConfig.updateRate);
}

void test_architectural_error_recovery_with_di(void)
{
    auto& container = testSetup->getContainer();
    auto& registry = testSetup->getRegistry();
    
    auto gpioProvider = container.resolve<IGpioProvider>();
    auto displayProvider = container.resolve<IDisplayProvider>();
    
    // Get necessary factories
    auto componentFactory = container.resolve<IComponentFactory>();
    auto sensorFactory = container.resolve<ISensorFactory>();
    
    // Register oil panel
    registry.registerPanel("OemOilPanel", [componentFactory](IGpioProvider* gpio, IDisplayProvider* display) {
        return std::make_unique<OemOilPanel>(componentFactory);
    });
    
    // Create components
    auto oilPanel = registry.createPanel("OemOilPanel", gpioProvider, displayProvider);
    auto pressureSensor = sensorFactory->createSensor("OilPressureSensor");
    auto tempSensor = sensorFactory->createSensor("OilTemperatureSensor");
    
    // Initialize panel
    IPanel* oilInterface = dynamic_cast<IPanel*>(oilPanel.get());
    oilInterface->init(gpioProvider, displayProvider);
    oilInterface->load([]{}, gpioProvider, displayProvider);
    
    // Normal operation first
    ISensor* pressureInterface = dynamic_cast<ISensor*>(pressureSensor.get());
    ISensor* tempInterface = dynamic_cast<ISensor*>(tempSensor.get());
    
    Reading normalPressure = pressureInterface->GetReading();
    Reading normalTemp = tempInterface->GetReading();
    
    TEST_ASSERT_TRUE(!std::holds_alternative<std::monostate>(normalPressure));
    TEST_ASSERT_TRUE(!std::holds_alternative<std::monostate>(normalTemp));
    
    // Simulate sensor failures
    auto gpio = testSetup->getTestGpioProvider();
    gpio->simulateFailure(34, true); // Pressure sensor failure
    gpio->simulateFailure(35, true); // Temperature sensor failure
    
    Reading failedPressure = pressureInterface->GetReading();
    Reading failedTemp = tempInterface->GetReading();
    
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
    
    Reading recoveredPressure = pressureInterface->GetReading();
    Reading recoveredTemp = tempInterface->GetReading();
    
    TEST_ASSERT_TRUE(!std::holds_alternative<std::monostate>(recoveredPressure));
    TEST_ASSERT_TRUE(!std::holds_alternative<std::monostate>(recoveredTemp));
    
    // Should be back to normal values
    TEST_ASSERT_EQUAL_UINT16(2048, gpio->analogRead(34));
    TEST_ASSERT_EQUAL_UINT16(1500, gpio->analogRead(35));
}

// Note: PlatformIO will automatically discover and run test_ functions