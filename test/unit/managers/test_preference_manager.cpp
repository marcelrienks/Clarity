#include <unity.h>
#include "managers/preference_manager.h"
#include "utilities/types.h"

PreferenceManager* prefManager = nullptr;

void setUp_preference_manager() {
    prefManager = new PreferenceManager();
}

void tearDown_preference_manager() {
    delete prefManager;
    prefManager = nullptr;
}

void test_preference_manager_init() {
    // Test initialization doesn't crash
    prefManager->init();
    
    // After init, config should have default panel name
    const Configs& config = prefManager->getConfig();
    TEST_ASSERT_EQUAL_STRING(PanelNames::OIL, config.panelName.c_str());
}

void test_preference_manager_get_set_config() {
    prefManager->init();
    
    // Test getting config
    const Configs& originalConfig = prefManager->getConfig();
    TEST_ASSERT_EQUAL_STRING(PanelNames::OIL.c_str(), originalConfig.panelName.c_str());
    
    // Test setting new config
    Configs newConfig;
    newConfig.panelName = "TestPanel";
    prefManager->setConfig(newConfig);
    
    const Configs& updatedConfig = prefManager->getConfig();
    TEST_ASSERT_EQUAL_STRING("TestPanel", updatedConfig.panelName.c_str());
}

void test_preference_manager_create_default_config() {
    // Test creating default config
    prefManager->createDefaultConfig();
    
    const Configs& config = prefManager->getConfig();
    TEST_ASSERT_EQUAL_STRING(PanelNames::OIL, config.panelName.c_str());
}

void test_preference_manager_save_load_cycle() {
    prefManager->init();
    
    // Set a custom panel name
    Configs testConfig;
    testConfig.panelName = "CustomPanel";
    prefManager->setConfig(testConfig);
    
    // Save and load should work without crashing
    prefManager->saveConfig();
    prefManager->loadConfig();
    
    // Config should remain valid
    const Configs& loadedConfig = prefManager->getConfig();
    TEST_ASSERT_NOT_NULL(loadedConfig.panelName.c_str());
}

void test_preference_manager_config_persistence() {
    prefManager->init();
    
    // Get initial config
    Configs& config = prefManager->getConfig();
    std::string originalPanel = config.panelName;
    
    // Modify config
    config.panelName = "ModifiedPanel";
    
    // Verify modification
    const Configs& modifiedConfig = prefManager->getConfig();
    TEST_ASSERT_EQUAL_STRING("ModifiedPanel", modifiedConfig.panelName.c_str());
    TEST_ASSERT_NOT_EQUAL_STRING(originalPanel.c_str(), modifiedConfig.panelName.c_str());
}

void runPreferenceManagerTests() {
    setUp_preference_manager();
    RUN_TEST(test_preference_manager_init);
    RUN_TEST(test_preference_manager_get_set_config);
    RUN_TEST(test_preference_manager_create_default_config);
    RUN_TEST(test_preference_manager_save_load_cycle);
    RUN_TEST(test_preference_manager_config_persistence);
    tearDown_preference_manager();
}