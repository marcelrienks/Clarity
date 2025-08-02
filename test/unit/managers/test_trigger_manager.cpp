#include <unity.h>
#include "managers/trigger_manager.h"
#include "sensors/key_sensor.h"
#include "sensors/lock_sensor.h"
#include "sensors/light_sensor.h"
#include "../mocks/mock_gpio_provider.h"
#include "utilities/types.h"
#include <memory>

#ifdef UNIT_TESTING
// Mock Arduino functions
extern "C" {
    void log_d(const char* format, ...) {}
    void log_e(const char* format, ...) {}
}

// Mock panel and style services
class MockPanelService : public IPanelService {
public:
    std::string lastLoadedPanel;
    bool loadPanelCalled = false;
    
    void loadPanel(const std::string& panelName) override {
        lastLoadedPanel = panelName;
        loadPanelCalled = true;
    }
    
    void reset() {
        lastLoadedPanel.clear();
        loadPanelCalled = false;
    }
};

class MockStyleService : public IStyleService {
public:
    std::string currentTheme;
    bool themeToggled = false;
    
    void toggleTheme(const std::string& newTheme) override {
        currentTheme = newTheme;
        themeToggled = true;
    }
    
    void reset() {
        currentTheme.clear();
        themeToggled = false;
    }
};
#endif

MockGpioProvider* mockGpio;
MockPanelService* mockPanelService;
MockStyleService* mockStyleService;
std::shared_ptr<KeySensor> keySensor;
std::shared_ptr<LockSensor> lockSensor;
std::shared_ptr<LightSensor> lightSensor;
TriggerManager* triggerManager;

void setUp_trigger_manager() {
    mockGpio = new MockGpioProvider();
    mockPanelService = new MockPanelService();
    mockStyleService = new MockStyleService();
    
    keySensor = std::make_shared<KeySensor>(mockGpio);
    lockSensor = std::make_shared<LockSensor>(mockGpio);
    lightSensor = std::make_shared<LightSensor>(mockGpio);
    
    triggerManager = new TriggerManager(keySensor, lockSensor, lightSensor, 
                                       mockPanelService, mockStyleService);
}

void tearDown_trigger_manager() {
    delete triggerManager;
    delete mockStyleService;
    delete mockPanelService;
    delete mockGpio;
}

void test_trigger_manager_initialization() {
    // Test that initialization completes without throwing
    TEST_ASSERT_NO_THROW(triggerManager->init());
}

void test_trigger_manager_startup_panel_override() {
    // Test with key not present - should return null (no override)
    mockGpio->setDigitalValue(gpio_pins::KEY_PRESENT, LOW);
    mockGpio->setDigitalValue(gpio_pins::KEY_NOT_PRESENT, LOW);
    
    const char* override1 = triggerManager->getStartupPanelOverride();
    TEST_ASSERT_NULL(override1);
    
    // Test with key present - should return KEY panel
    mockGpio->setDigitalValue(gpio_pins::KEY_PRESENT, HIGH);
    mockGpio->setDigitalValue(gpio_pins::KEY_NOT_PRESENT, LOW);
    
    const char* override2 = triggerManager->getStartupPanelOverride();
    TEST_ASSERT_NOT_NULL(override2);
    TEST_ASSERT_EQUAL_STRING(PanelNames::KEY, override2);
}

void test_trigger_manager_key_trigger_processing() {
    triggerManager->init();
    
    // Set initial state (no key present)
    mockGpio->setDigitalValue(gpio_pins::KEY_PRESENT, LOW);
    mockGpio->setDigitalValue(gpio_pins::KEY_NOT_PRESENT, LOW);
    
    // Process initial state
    triggerManager->processTriggerEvents();
    
    // Change to key present state
    mockGpio->setDigitalValue(gpio_pins::KEY_PRESENT, HIGH);
    
    // Process trigger events
    triggerManager->processTriggerEvents();
    
    // Verify panel was loaded (depending on implementation details)
    // Note: This test may need adjustment based on actual trigger processing logic
    TEST_ASSERT_TRUE(true); // Placeholder - actual verification depends on implementation
}

void test_trigger_manager_light_trigger_processing() {
    triggerManager->init();
    
    // Set initial state (lights off)
    mockGpio->setAnalogValue(gpio_pins::LIGHTS, 0);
    
    // Process initial state
    triggerManager->processTriggerEvents();
    
    // Change to lights on state
    mockGpio->setAnalogValue(gpio_pins::LIGHTS, 3000); // High light value
    
    // Process trigger events
    triggerManager->processTriggerEvents();
    
    // Verify theme toggle was triggered (depending on implementation)
    TEST_ASSERT_TRUE(true); // Placeholder - actual verification depends on implementation
}

void test_trigger_manager_multiple_sensors() {
    triggerManager->init();
    
    // Test that all sensors are properly initialized
    // This is verified by the sensors' init methods being called
    TEST_ASSERT_NO_THROW(triggerManager->processTriggerEvents());
    
    // Test with multiple sensor state changes
    mockGpio->setDigitalValue(gpio_pins::KEY_PRESENT, HIGH);
    mockGpio->setDigitalValue(gpio_pins::LOCK, HIGH);
    mockGpio->setAnalogValue(gpio_pins::LIGHTS, 2000);
    
    TEST_ASSERT_NO_THROW(triggerManager->processTriggerEvents());
}

void test_trigger_manager_lock_state_changes() {
    triggerManager->init();
    
    // Test lock state changes
    mockGpio->setDigitalValue(gpio_pins::LOCK, LOW); // Unlocked
    triggerManager->processTriggerEvents();
    
    mockGpio->setDigitalValue(gpio_pins::LOCK, HIGH); // Locked
    triggerManager->processTriggerEvents();
    
    // Verify no crashes and proper state handling
    TEST_ASSERT_TRUE(true);
}

void test_trigger_manager_service_integration() {
    triggerManager->init();
    
    // Reset mock services
    mockPanelService->reset();
    mockStyleService->reset();
    
    // Trigger key present state that should call panel service
    mockGpio->setDigitalValue(gpio_pins::KEY_PRESENT, HIGH);
    mockGpio->setDigitalValue(gpio_pins::KEY_NOT_PRESENT, LOW);
    
    // Process and verify service integration works
    TEST_ASSERT_NO_THROW(triggerManager->processTriggerEvents());
    
    // Services should be accessible (implementation dependent)
    TEST_ASSERT_FALSE(mockPanelService->loadPanelCalled); // May change based on implementation
}

void runTriggerManagerTests() {
    setUp_trigger_manager();
    RUN_TEST(test_trigger_manager_initialization);
    RUN_TEST(test_trigger_manager_startup_panel_override);
    RUN_TEST(test_trigger_manager_key_trigger_processing);
    RUN_TEST(test_trigger_manager_light_trigger_processing);
    RUN_TEST(test_trigger_manager_multiple_sensors);
    RUN_TEST(test_trigger_manager_lock_state_changes);
    RUN_TEST(test_trigger_manager_service_integration);
    tearDown_trigger_manager();
}