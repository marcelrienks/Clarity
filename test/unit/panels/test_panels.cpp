#include <unity.h>
#include <panel_manager.h>
#include <panels/key_panel.h>
#include <panels/lock_panel.h>
#include <panels/oem_oil_panel.h>
#include <panels/splash_panel.h>
#include "../mocks/mock_managers.h"

void test_panel_manager_initialization(void) {
    // Setup
    PanelManager panelManager;
    bool initSuccess = false;

    // Test
    initSuccess = panelManager.initialize();

    // Verify
    TEST_ASSERT_TRUE(initSuccess);
    TEST_ASSERT_NOT_NULL(panelManager.getCurrentPanel());
}

void test_panel_registration(void) {
    // Setup
    PanelManager panelManager;
    KeyPanel* keyPanel = new KeyPanel();
    LockPanel* lockPanel = new LockPanel();

    // Test
    bool registerKeySuccess = panelManager.registerPanel(keyPanel);
    bool registerLockSuccess = panelManager.registerPanel(lockPanel);

    // Verify
    TEST_ASSERT_TRUE(registerKeySuccess);
    TEST_ASSERT_TRUE(registerLockSuccess);
    TEST_ASSERT_EQUAL(2, panelManager.getRegisteredPanelCount());
}

void test_panel_creation_and_loading(void) {
    // Setup
    PanelManager panelManager;
    panelManager.initialize();

    // Test
    IPanel* newPanel = panelManager.createPanel("KeyPanel");

    // Verify
    TEST_ASSERT_NOT_NULL(newPanel);
    TEST_ASSERT_TRUE(panel_loaded);
    TEST_ASSERT_TRUE(panelManager.getCurrentPanel() == newPanel);
}

void test_panel_cleanup_on_switch(void) {
    // Setup
    PanelManager panelManager;
    panelManager.initialize();
    IPanel* firstPanel = panelManager.createPanel("KeyPanel");
    
    // Test
    IPanel* secondPanel = panelManager.createPanel("LockPanel");
    
    // Verify
    TEST_ASSERT_NULL(firstPanel->getParent());
    TEST_ASSERT_FALSE(firstPanel->isVisible());
    TEST_ASSERT_TRUE(secondPanel->isVisible());
}

void test_panel_lifecycle_init_load_update(void) {
    // Setup
    PanelManager panelManager;
    panelManager.initialize();
    
    // Test
    IPanel* panel = panelManager.createPanel("KeyPanel");
    
    // Verify initialization
    TEST_ASSERT_TRUE(panel_initialized);
    TEST_ASSERT_TRUE(panel_loaded);
    
    // Test update
    panel->update();
    TEST_ASSERT_TRUE(panel->isActive());
}

void test_splash_panel_lifecycle(void) {
    // Setup
    PanelManager panelManager;
    panelManager.initialize();
    
    // Test
    IPanel* splashPanel = panelManager.createPanel("SplashPanel");
    
    // Verify
    TEST_ASSERT_NOT_NULL(splashPanel);
    TEST_ASSERT_TRUE(splashPanel->isActive());
    
    // Test automatic transition
    delay(3000); // Simulated delay for splash screen
    TEST_ASSERT_FALSE(splashPanel->isActive());
}

void test_panel_state_consistency(void) {
    // Setup
    PanelManager panelManager;
    panelManager.initialize();
    
    // Test
    IPanel* panel = panelManager.createPanel("KeyPanel");
    panel->activate();
    
    // Verify
    TEST_ASSERT_TRUE(panel->isActive());
    TEST_ASSERT_TRUE(panel->isVisible());
    
    panel->deactivate();
    TEST_ASSERT_FALSE(panel->isActive());
    TEST_ASSERT_FALSE(panel->isVisible());
}

void test_invalid_panel_creation(void) {
    // Setup
    PanelManager panelManager;
    panelManager.initialize();
    
    // Test
    IPanel* invalidPanel = panelManager.createPanel("NonexistentPanel");
    
    // Verify
    TEST_ASSERT_NULL(invalidPanel);
    TEST_ASSERT_FALSE(panel_creation_history.empty());
    TEST_ASSERT_EQUAL("ERROR", panel_creation_history.back());
}

void test_panel_memory_management(void) {
    // Setup
    PanelManager panelManager;
    panelManager.initialize();
    
    // Test
    for(int i = 0; i < 10; i++) {
        IPanel* panel = panelManager.createPanel("KeyPanel");
        TEST_ASSERT_NOT_NULL(panel);
        panel->deactivate();
    }
    
    // Verify no memory leaks through external tool
    // Memory check is handled by the test framework
}

void test_panel_creation_failure_recovery(void) {
    // Setup
    PanelManager panelManager;
    panelManager.initialize();
    
    // Simulate resource exhaustion
    MockResourceManager::simulateResourceExhaustion(true);
    
    // Test
    IPanel* failedPanel = panelManager.createPanel("KeyPanel");
    
    // Verify
    TEST_ASSERT_NULL(failedPanel);
    TEST_ASSERT_NOT_NULL(panelManager.getCurrentPanel());
    
    // Test recovery
    MockResourceManager::simulateResourceExhaustion(false);
    IPanel* recoveryPanel = panelManager.createPanel("KeyPanel");
    TEST_ASSERT_NOT_NULL(recoveryPanel);
}

void test_panel_restoration_chain(void) {
    // Setup
    PanelManager panelManager;
    panelManager.initialize();
    
    // Create panel chain
    IPanel* panel1 = panelManager.createPanel("KeyPanel");
    IPanel* panel2 = panelManager.createPanel("LockPanel");
    IPanel* panel3 = panelManager.createPanel("OemOilPanel");
    
    // Test
    panel3->deactivate();
    
    // Verify
    TEST_ASSERT_TRUE(panel2->isActive());
    TEST_ASSERT_FALSE(panel3->isActive());
    
    panel2->deactivate();
    TEST_ASSERT_TRUE(panel1->isActive());
    TEST_ASSERT_FALSE(panel2->isActive());
}
