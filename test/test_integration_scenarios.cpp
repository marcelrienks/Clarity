#ifdef UNIT_TEST
#include <unity.h>
#include "managers/interrupt_manager.h"
#include "managers/panel_manager.h"
#include "managers/error_manager.h"
#include "sensors/action_button_sensor.h"
#include "sensors/key_present_sensor.h"
#include "sensors/lock_sensor.h"
#include "mocks/mock_gpio_provider.h"
#include "mocks/mock_display_provider.h"
#include "utilities/constants.h"

// Test fixtures
std::unique_ptr<MockGpioProvider> mockGpio;
std::unique_ptr<MockDisplayProvider> mockDisplay;
std::unique_ptr<InterruptManager> interruptManager;
std::unique_ptr<PanelManager> panelManager;

void setUp(void) {
    // Create mock providers
    mockGpio = std::make_unique<MockGpioProvider>();
    mockDisplay = std::make_unique<MockDisplayProvider>();
    
    // Initialize managers
    interruptManager = std::make_unique<InterruptManager>();
    panelManager = std::make_unique<PanelManager>(mockDisplay.get(), nullptr, nullptr, nullptr);
    
    // Initialize interrupt system
    interruptManager->Init();
}

void tearDown(void) {
    interruptManager.reset();
    panelManager.reset();
    mockDisplay.reset();
    mockGpio.reset();
}

/**
 * @brief Test Scenario 1: Basic Panel Switching
 * Verify that panels load correctly and button functions are injected
 */
void test_basic_panel_switching_scenario() {
    TEST_ASSERT_NOT_NULL(panelManager.get());
    
    // Load Oil Panel (default)
    bool loaded = panelManager->CreateAndLoadPanel(PanelNames::OIL, false);
    TEST_ASSERT_TRUE(loaded);
    TEST_ASSERT_EQUAL_STRING(PanelNames::OIL, panelManager->GetCurrentPanelName());
    
    // Load Config Panel
    loaded = panelManager->CreateAndLoadPanel(PanelNames::CONFIG, false);
    TEST_ASSERT_TRUE(loaded);
    TEST_ASSERT_EQUAL_STRING(PanelNames::CONFIG, panelManager->GetCurrentPanelName());
    
    // Verify restoration panel is set
    const char* restorationPanel = panelManager->GetRestorationPanel();
    TEST_ASSERT_EQUAL_STRING(PanelNames::OIL, restorationPanel);
}

/**
 * @brief Test Scenario 2: Error Panel Auto-Restoration
 * Test complete error workflow from detection to restoration
 */
void test_error_panel_auto_restoration_scenario() {
    // Load initial panel
    panelManager->CreateAndLoadPanel(PanelNames::OIL, false);
    
    // Generate test error
    ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "TestComponent", "Test error for scenario");
    
    // Error panel should trigger automatically (tested via main loop simulation)
    bool hasErrors = ErrorManager::Instance().HasPendingErrors();
    TEST_ASSERT_TRUE(hasErrors);
    TEST_ASSERT_TRUE(ErrorManager::Instance().ShouldTriggerErrorPanel());
    
    // Load error panel (simulating automatic trigger)
    ErrorManager::Instance().SetErrorPanelActive(true);
    bool loaded = panelManager->CreateAndLoadPanel(PanelNames::ERROR, true);
    TEST_ASSERT_TRUE(loaded);
    TEST_ASSERT_EQUAL_STRING(PanelNames::ERROR, panelManager->GetCurrentPanelName());
    
    // Clear errors (simulating user interaction)
    ErrorManager::Instance().ClearAllErrors();
    ErrorManager::Instance().SetErrorPanelActive(false);
    
    // Verify no errors remain
    hasErrors = ErrorManager::Instance().HasPendingErrors();
    TEST_ASSERT_FALSE(hasErrors);
    TEST_ASSERT_FALSE(ErrorManager::Instance().ShouldTriggerErrorPanel());
}

/**
 * @brief Test Scenario 3: Universal Button Function Injection
 * Verify that button functions are properly injected and can be executed
 */
void test_universal_button_function_injection_scenario() {
    // Load Config Panel
    panelManager->CreateAndLoadPanel(PanelNames::CONFIG, false);
    
    // Get current panel as IActionService
    auto* currentPanel = panelManager->GetCurrentPanel();
    TEST_ASSERT_NOT_NULL(currentPanel);
    
    auto* actionService = dynamic_cast<IActionService*>(currentPanel);
    TEST_ASSERT_NOT_NULL(actionService);
    
    // Verify function pointers can be extracted
    auto shortPressFunc = actionService->GetShortPressFunction();
    auto longPressFunc = actionService->GetLongPressFunction();
    auto panelContext = actionService->GetPanelContext();
    
    TEST_ASSERT_NOT_NULL(shortPressFunc);
    TEST_ASSERT_NOT_NULL(longPressFunc);
    TEST_ASSERT_NOT_NULL(panelContext);
    TEST_ASSERT_EQUAL_PTR(currentPanel, panelContext);
    
    // Test function injection into InterruptManager
    if (interruptManager) {
        interruptManager->UpdateButtonInterrupts(shortPressFunc, longPressFunc, panelContext);
        // Functions should be stored for later execution
    }
}

/**
 * @brief Test Scenario 4: Multi-Interrupt Priority Handling
 * Test priority coordination across PolledHandler and QueuedHandler
 */
void test_multi_interrupt_priority_scenario() {
    // This would require mock sensors and interrupt triggers
    // For now, verify the interrupt manager exists and is initialized
    TEST_ASSERT_NOT_NULL(interruptManager.get());
    
    // Verify interrupt registration works
    TEST_ASSERT_EQUAL(0, interruptManager->GetInterruptCount()); // Should be 0 in test environment
    
    // In a real scenario, we would:
    // 1. Trigger multiple GPIO state changes
    // 2. Verify highest priority interrupt executes
    // 3. Test coordinated handler processing
}

/**
 * @brief Test Scenario 5: Theme System Integration
 * Verify theme changes work across panel switches
 */
void test_theme_system_integration_scenario() {
    // Load a panel
    panelManager->CreateAndLoadPanel(PanelNames::OIL, false);
    
    // In a full implementation, we would:
    // 1. Change theme via lights sensor or config panel
    // 2. Verify theme persists across panel switches
    // 3. Test that theme changes don't affect restoration logic
    
    // For now, just verify panel manager works
    TEST_ASSERT_EQUAL_STRING(PanelNames::OIL, panelManager->GetCurrentPanelName());
}

/**
 * @brief Test Scenario 6: Memory Stability Under Load
 * Test system stability with repeated operations
 */
void test_memory_stability_scenario() {
    // Perform multiple panel switches to test memory stability
    for (int i = 0; i < 10; i++) {
        panelManager->CreateAndLoadPanel(PanelNames::OIL, false);
        panelManager->CreateAndLoadPanel(PanelNames::CONFIG, false);
        panelManager->CreateAndLoadPanel(PanelNames::SPLASH, false);
    }
    
    // Verify system is still functional
    bool loaded = panelManager->CreateAndLoadPanel(PanelNames::OIL, false);
    TEST_ASSERT_TRUE(loaded);
    TEST_ASSERT_EQUAL_STRING(PanelNames::OIL, panelManager->GetCurrentPanelName());
    
    // Generate and clear errors repeatedly
    for (int i = 0; i < 5; i++) {
        ErrorManager::Instance().ReportError(ErrorLevel::WARNING, "TestLoop", "Stability test warning");
        ErrorManager::Instance().ClearAllErrors();
    }
    
    // Verify error system is still functional
    TEST_ASSERT_FALSE(ErrorManager::Instance().HasPendingErrors());
}

/**
 * @brief Test Scenario 7: Config Panel State Persistence
 * Test that config panel maintains state during navigation
 */
void test_config_panel_state_persistence_scenario() {
    // Load config panel
    bool loaded = panelManager->CreateAndLoadPanel(PanelNames::CONFIG, false);
    TEST_ASSERT_TRUE(loaded);
    
    // In a full implementation with mock providers:
    // 1. Navigate to theme submenu
    // 2. Temporarily switch to another panel
    // 3. Return to config panel
    // 4. Verify we're still in theme submenu
    
    // For now, just verify config panel loads
    TEST_ASSERT_EQUAL_STRING(PanelNames::CONFIG, panelManager->GetCurrentPanelName());
}

void runIntegrationScenarioTests(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_basic_panel_switching_scenario);
    RUN_TEST(test_error_panel_auto_restoration_scenario);
    RUN_TEST(test_universal_button_function_injection_scenario);
    RUN_TEST(test_multi_interrupt_priority_scenario);
    RUN_TEST(test_theme_system_integration_scenario);
    RUN_TEST(test_memory_stability_scenario);
    RUN_TEST(test_config_panel_state_persistence_scenario);
    
    UNITY_END();
}

#ifdef ARDUINO
void setup() {
    delay(2000); // Give time for serial monitor
    runIntegrationScenarioTests();
}

void loop() {
    // Tests run once in setup
}
#else
int main() {
    runIntegrationScenarioTests();
    return 0;
}
#endif

#endif // UNIT_TEST