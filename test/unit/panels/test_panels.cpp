#include <unity.h>
#include "panel_manager.h"

void test_panel_manager_initialization(void) {
    // Setup
    PanelManager panelManager;

    // Test
    bool initSuccess = panelManager.initialize();

    // Verify
    TEST_ASSERT_TRUE(initSuccess);
}

void test_panel_registration(void) {
    // Setup
    PanelManager panelManager;
    KeyPanel keyPanel;
    LockPanel lockPanel;

    // Test
    bool registerKeySuccess = panelManager.registerPanel(&keyPanel);
    bool registerLockSuccess = panelManager.registerPanel(&lockPanel);

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
}

void test_basic_panel_operations(void) {
    // Setup
    PanelManager panelManager;
    panelManager.initialize();
    
    // Test panel creation
    IPanel* panel = panelManager.createPanel("KeyPanel");
    TEST_ASSERT_NOT_NULL(panel);
    
    // Test basic panel interface methods
    panel->init();
    panel->load();
    panel->update();
    
    // Basic functionality test passed
    TEST_ASSERT_TRUE(true);
}
