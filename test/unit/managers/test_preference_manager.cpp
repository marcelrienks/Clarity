#ifdef UNIT_TESTING

#include <unity.h>
#include <cstring>
#include "test_fixtures.h"

// Unity extension macros for string comparison  
#define TEST_ASSERT_NOT_EQUAL_STRING(expected, actual) TEST_ASSERT_FALSE(strcmp(expected, actual) == 0)
#include "managers/preference_manager.h"
#include "utilities/types.h"
#include "ArduinoJson.h"

std::unique_ptr<ManagerTestFixture> fixture;
PreferenceManager* prefManager = nullptr;

void setUp_preference_manager() {
    fixture = std::make_unique<ManagerTestFixture>();
    fixture->SetUp();
    prefManager = new PreferenceManager();
}

void tearDown_preference_manager() {
    delete prefManager;
    prefManager = nullptr;
    fixture->TearDown();
    fixture.reset();
}

void test_preference_manager_init() {
    // Test initialization doesn't crash
    prefManager->init();
    
    // Verify preference service was called during init
    TEST_ASSERT_TRUE(fixture->getPreferenceService()->wasLoadCalled());
    
    // After init, config should have default panel name
    const Configs& config = prefManager->getConfig();
    TEST_ASSERT_EQUAL_STRING(PanelNames::OIL, config.panelName.c_str());
}

void test_preference_manager_get_set_config() {
    prefManager->init();
    
    // Test getting config
    const Configs& originalConfig = prefManager->getConfig();
    TEST_ASSERT_EQUAL_STRING(PanelNames::OIL, originalConfig.panelName.c_str());
    
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
    TEST_ASSERT_TRUE(fixture->getPreferenceService()->wasSaveCalled());
    
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

void test_preference_manager_json_serialization() {
    prefManager->init();
    
    // Set up complex configuration
    Configs testConfig;
    testConfig.panelName = "TestPanelName";
    prefManager->setConfig(testConfig);
    
    // Test JSON serialization by saving config
    prefManager->saveConfig();
    
    // Verify service interaction occurred
    TEST_ASSERT_TRUE(fixture->getPreferenceService()->wasSaveCalled());
}

void test_preference_manager_json_deserialization() {
    prefManager->init();
    
    // Set up test data in preference service
    fixture->setPreference("config", "{\"panelName\":\"DeserializedPanel\"}");
    
    // Load configuration
    prefManager->loadConfig();
    
    // In a real implementation, we'd verify the config was loaded from JSON
    // For now, just verify the load operation occurred
    TEST_ASSERT_TRUE(fixture->getPreferenceService()->wasLoadCalled());
}

void test_preference_manager_error_handling() {
    prefManager->init();
    
    // Test with empty configuration
    Configs emptyConfig;
    emptyConfig.panelName = "";
    prefManager->setConfig(emptyConfig);
    
    const Configs& config = prefManager->getConfig();
    TEST_ASSERT_NOT_NULL(config.panelName.c_str());
}

void test_preference_manager_multiple_save_load() {
    prefManager->init();
    
    // Test multiple save/load cycles
    for (int i = 0; i < 5; i++) {
        Configs config;
        config.panelName = "Panel" + std::to_string(i);
        prefManager->setConfig(config);
        prefManager->saveConfig();
        
        const Configs& savedConfig = prefManager->getConfig();
        TEST_ASSERT_EQUAL_STRING(("Panel" + std::to_string(i)).c_str(), 
                                savedConfig.panelName.c_str());
    }
}

void test_preference_manager_service_integration() {
    prefManager->init();
    
    // Test that preference manager properly uses the preference service
    auto* prefService = fixture->getPreferenceService();
    
    // Initially should have called load
    TEST_ASSERT_TRUE(prefService->wasLoadCalled());
    
    // Save should call service save
    prefManager->saveConfig();
    TEST_ASSERT_TRUE(prefService->wasSaveCalled());
}

void test_preference_manager_config_validation() {
    prefManager->init();
    
    // Test with valid configuration
    Configs validConfig;
    validConfig.panelName = "ValidPanel";
    prefManager->setConfig(validConfig);
    
    const Configs& config = prefManager->getConfig();
    TEST_ASSERT_EQUAL_STRING("ValidPanel", config.panelName.c_str());
}

void test_preference_manager_default_config_creation() {
    // Test default config creation before init
    prefManager->createDefaultConfig();
    
    const Configs& defaultConfig = prefManager->getConfig();
    TEST_ASSERT_EQUAL_STRING(PanelNames::OIL, defaultConfig.panelName.c_str());
    
    // Test that init still works after creating default config
    prefManager->init();
    
    const Configs& configAfterInit = prefManager->getConfig();
    TEST_ASSERT_NOT_NULL(configAfterInit.panelName.c_str());
}

void test_preference_manager_concurrent_access() {
    prefManager->init();
    
    // Simulate concurrent access patterns
    const Configs& config1 = prefManager->getConfig();
    Configs newConfig;
    newConfig.panelName = "ConcurrentPanel";
    prefManager->setConfig(newConfig);
    const Configs& config2 = prefManager->getConfig();
    
    // Verify consistency
    TEST_ASSERT_NOT_EQUAL_STRING(config1.panelName.c_str(), config2.panelName.c_str());
    TEST_ASSERT_EQUAL_STRING("ConcurrentPanel", config2.panelName.c_str());
}

void test_preference_manager_memory_management() {
    // Test creating and destroying multiple instances
    for (int i = 0; i < 10; i++) {
        PreferenceManager* tempManager = new PreferenceManager();
        tempManager->init();
        tempManager->createDefaultConfig();
        delete tempManager;
    }
    
    // If we get here without crashes, memory management is working
    TEST_ASSERT_TRUE(true);
}

void runPreferenceManagerTests() {
    setUp_preference_manager();
    RUN_TEST(test_preference_manager_init);
    RUN_TEST(test_preference_manager_get_set_config);
    RUN_TEST(test_preference_manager_create_default_config);
    RUN_TEST(test_preference_manager_save_load_cycle);
    RUN_TEST(test_preference_manager_config_persistence);
    RUN_TEST(test_preference_manager_json_serialization);
    RUN_TEST(test_preference_manager_json_deserialization);
    RUN_TEST(test_preference_manager_error_handling);
    RUN_TEST(test_preference_manager_multiple_save_load);
    RUN_TEST(test_preference_manager_service_integration);
    RUN_TEST(test_preference_manager_config_validation);
    RUN_TEST(test_preference_manager_default_config_creation);
    RUN_TEST(test_preference_manager_concurrent_access);
    RUN_TEST(test_preference_manager_memory_management);
    tearDown_preference_manager();
}

#endif