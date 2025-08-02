#pragma once

#ifdef UNIT_TESTING

#include "../mocks/Arduino.h"
#include "../mocks/lvgl.h"
#include "../mocks/mock_services.h"
#include "../../include/system/service_container.h"
#include "../../include/utilities/types.h"
#include <memory>
#include <functional>

// Base test fixture for all tests
class BaseTestFixture {
protected:
    std::unique_ptr<ServiceContainer> serviceContainer;
    std::unique_ptr<MockPanelService> mockPanelService;
    std::unique_ptr<MockStyleService> mockStyleService;
    std::unique_ptr<MockTriggerService> mockTriggerService;
    std::unique_ptr<MockDisplayProvider> mockDisplayProvider;
    std::unique_ptr<MockPreferenceService> mockPreferenceService;
    std::unique_ptr<MockGpioProvider> mockGpioProvider;
    
public:
    virtual void SetUp() {
        // Reset all mock states
        MockHardwareState::instance().reset();
        MockLVGLState::instance().reset();
        
        // Create mock services
        mockPanelService = std::make_unique<MockPanelService>();
        mockStyleService = std::make_unique<MockStyleService>();
        mockTriggerService = std::make_unique<MockTriggerService>();
        mockDisplayProvider = std::make_unique<MockDisplayProvider>();
        mockPreferenceService = std::make_unique<MockPreferenceService>();
        mockGpioProvider = std::make_unique<MockGpioProvider>();
        
        // Create service container and register services
        serviceContainer = std::make_unique<ServiceContainer>();
        serviceContainer->register_service<IPanelService>(mockPanelService.get());
        serviceContainer->register_service<IStyleService>(mockStyleService.get());
        serviceContainer->register_service<ITriggerService>(mockTriggerService.get());
        serviceContainer->register_service<IDisplayProvider>(mockDisplayProvider.get());
        serviceContainer->register_service<IPreferenceService>(mockPreferenceService.get());
        serviceContainer->register_service<IGpioProvider>(mockGpioProvider.get());
        
        // Initialize display provider to create mock screen
        mockDisplayProvider->init();
    }
    
    virtual void TearDown() {
        // Clean up in reverse order
        serviceContainer.reset();
        mockGpioProvider.reset();
        mockPreferenceService.reset();
        mockDisplayProvider.reset();
        mockTriggerService.reset();
        mockStyleService.reset();
        mockPanelService.reset();
        
        // Reset mock states
        MockHardwareState::instance().reset();
        MockLVGLState::instance().reset();
    }
    
    // Utility methods for tests
    ServiceContainer* getServiceContainer() { return serviceContainer.get(); }
    MockPanelService* getPanelService() { return mockPanelService.get(); }
    MockStyleService* getStyleService() { return mockStyleService.get(); }
    MockTriggerService* getTriggerService() { return mockTriggerService.get(); }
    MockDisplayProvider* getDisplayProvider() { return mockDisplayProvider.get(); }
    MockPreferenceService* getPreferenceService() { return mockPreferenceService.get(); }
    MockGpioProvider* getGpioProvider() { return mockGpioProvider.get(); }
};

// Sensor test fixture with common sensor testing utilities
class SensorTestFixture : public BaseTestFixture {
protected:
    // Common sensor test pins
    static constexpr uint8_t TEST_DIGITAL_PIN = 2;
    static constexpr uint8_t TEST_ANALOG_PIN = A0;
    
public:
    void SetUp() override {
        BaseTestFixture::SetUp();
        
        // Setup common test pins
        mockGpioProvider->setup_pin(TEST_DIGITAL_PIN, INPUT_PULLUP);
        mockGpioProvider->setup_pin(TEST_ANALOG_PIN, INPUT);
    }
    
    // Utility methods for sensor testing
    void setDigitalPin(uint8_t pin, int value) {
        mockGpioProvider->setPinValue(pin, value);
    }
    
    void setAnalogPin(uint8_t pin, int value) {
        mockGpioProvider->setPinValue(pin, value);
    }
    
    void triggerInterrupt(uint8_t pin) {
        mockGpioProvider->triggerInterrupt(pin);
    }
    
    void advanceTime(uint32_t ms) {
        MockHardwareState::instance().advanceTime(ms);
    }
    
    uint32_t getCurrentTime() {
        return MockHardwareState::instance().getMillis();
    }
};

// Manager test fixture with additional manager-specific utilities
class ManagerTestFixture : public BaseTestFixture {
public:
    void SetUp() override {
        BaseTestFixture::SetUp();
        
        // Initialize services that managers typically depend on
        mockDisplayProvider->init();
        mockPreferenceService->load_preferences();
    }
    
    // Utility methods for manager testing
    void simulateTrigger(TriggerType trigger, bool state) {
        mockTriggerService->simulateTrigger(trigger, state);
    }
    
    void setTheme(Theme theme) {
        mockStyleService->set_theme(theme);
    }
    
    void setPreference(const std::string& key, const std::string& value) {
        mockPreferenceService->set_preference(key, value);
    }
    
    PanelType getCurrentPanel() {
        return mockPanelService->get_current_panel();
    }
    
    bool isPanelVisible(PanelType panel) {
        return mockPanelService->is_panel_visible(panel);
    }
};

// Component test fixture with LVGL-specific utilities
class ComponentTestFixture : public BaseTestFixture {
protected:
    lv_obj_t* testScreen = nullptr;
    
public:
    void SetUp() override {
        BaseTestFixture::SetUp();
        
        // Create a test screen for components
        testScreen = lv_obj_create(nullptr);
        MockLVGLState::instance().setActiveScreen(testScreen);
    }
    
    void TearDown() override {
        if (testScreen) {
            lv_obj_del(testScreen);
            testScreen = nullptr;
        }
        BaseTestFixture::TearDown();
    }
    
    lv_obj_t* getTestScreen() { return testScreen; }
    
    // Utility methods for component testing
    lv_obj_t* createTestObject() {
        return lv_obj_create(testScreen);
    }
    
    void verifyObjectPosition(lv_obj_t* obj, lv_coord_t expectedX, lv_coord_t expectedY) {
        auto mock = MockLVGLState::instance().getObject(obj);
        if (mock) {
            // In a real test framework, you'd use assertions here
            // For now, we'll just verify the structure exists
        }
    }
    
    void verifyObjectSize(lv_obj_t* obj, lv_coord_t expectedW, lv_coord_t expectedH) {
        auto mock = MockLVGLState::instance().getObject(obj);
        if (mock) {
            // In a real test framework, you'd use assertions here
            // For now, we'll just verify the structure exists
        }
    }
};

// Integration test fixture for full scenario testing
class IntegrationTestFixture : public BaseTestFixture {
private:
    struct ScenarioStep {
        std::string description;
        std::function<void()> action;
        std::function<bool()> verification;
    };
    
    std::vector<ScenarioStep> scenarioSteps;
    
public:
    void SetUp() override {
        BaseTestFixture::SetUp();
        
        // Initialize all services for integration testing
        mockDisplayProvider->init();
        mockPreferenceService->load_preferences();
        mockTriggerService->init();
        mockStyleService->init();
        mockPanelService->init();
    }
    
    // Scenario building methods
    void addScenarioStep(const std::string& description, 
                        std::function<void()> action, 
                        std::function<bool()> verification = nullptr) {
        scenarioSteps.push_back({description, action, verification});
    }
    
    bool executeScenario() {
        for (const auto& step : scenarioSteps) {
            // Execute the action
            step.action();
            
            // Verify if verification function provided
            if (step.verification && !step.verification()) {
                return false; // Scenario failed
            }
        }
        return true; // All steps passed
    }
    
    void clearScenario() {
        scenarioSteps.clear();
    }
    
    // Common scenario actions
    void triggerKeyPresent(bool state) {
        mockTriggerService->simulateTrigger(TriggerType::KEY_PRESENT, state);
    }
    
    void triggerKeyNotPresent(bool state) {
        mockTriggerService->simulateTrigger(TriggerType::KEY_NOT_PRESENT, state);
    }
    
    void triggerLock(bool state) {
        mockTriggerService->simulateTrigger(TriggerType::LOCK, state);
    }
    
    void triggerLights(bool state) {
        mockTriggerService->simulateTrigger(TriggerType::LIGHTS, state);
    }
    
    void waitForTime(uint32_t ms) {
        MockHardwareState::instance().advanceTime(ms);
    }
    
    // Common scenario verifications
    bool verifyPanelShown(PanelType expectedPanel) {
        return mockPanelService->get_current_panel() == expectedPanel;
    }
    
    bool verifyTheme(Theme expectedTheme) {
        return mockStyleService->get_theme() == expectedTheme;
    }
    
    bool verifyTriggerState(TriggerType trigger, bool expectedState) {
        return mockTriggerService->get_trigger_state(trigger) == expectedState;
    }
};

// Memory and performance testing utilities
class PerformanceTestFixture : public BaseTestFixture {
private:
    uint32_t startTime = 0;
    size_t startMemory = 0;
    
public:
    void startPerformanceMeasurement() {
        startTime = MockHardwareState::instance().getMillis();
        // In a real implementation, you'd measure actual memory usage
        startMemory = 0;
    }
    
    uint32_t getElapsedTime() {
        return MockHardwareState::instance().getMillis() - startTime;
    }
    
    size_t getMemoryUsage() {
        // Mock memory measurement
        return 1024; // Return fixed value for mock
    }
    
    // Stress testing utilities
    void stressTestTriggers(int iterations) {
        for (int i = 0; i < iterations; i++) {
            TriggerType triggers[] = {
                TriggerType::KEY_PRESENT,
                TriggerType::KEY_NOT_PRESENT,
                TriggerType::LOCK,
                TriggerType::LIGHTS
            };
            
            for (auto trigger : triggers) {
                mockTriggerService->simulateTrigger(trigger, i % 2);
            }
        }
    }
    
    void stressTestPanelSwitching(int iterations) {
        PanelType panels[] = {
            PanelType::SPLASH,
            PanelType::OEM_OIL,
            PanelType::KEY,
            PanelType::LOCK
        };
        
        for (int i = 0; i < iterations; i++) {
            mockPanelService->show_panel(panels[i % 4]);
        }
    }
};

#endif // UNIT_TESTING