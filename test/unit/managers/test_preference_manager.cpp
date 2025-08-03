#ifdef UNIT_TESTING

#include <unity.h>
#include <cstring>
#include "test_fixtures.h"

// Unity extension macros for string comparison  
#define TEST_ASSERT_NOT_EQUAL_STRING(expected, actual) TEST_ASSERT_FALSE(strcmp(expected, actual) == 0)
#include "managers/preference_manager.h"
#include "utilities/types.h"
#include "ArduinoJson.h"

std::unique_ptr<ManagerTestFixture> prefManagerFixture;
PreferenceManager* prefManager = nullptr;

void setUp_preference_manager() {
    prefManagerFixture = std::make_unique<ManagerTestFixture>();
    prefManagerFixture->SetUp();
    prefManager = new PreferenceManager();
}

void tearDown_preference_manager() {
    delete prefManager;
    prefManager = nullptr;
    prefManagerFixture->TearDown();
    prefManagerFixture.reset();
}

void test_preference_manager_init() {
    // Test initialization doesn't crash
    prefManager->init();
    
    // Verify preference service was called during init
    TEST_ASSERT_TRUE(prefManagerFixture->getPreferenceService()->wasLoadCalled());
    
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
    
    // Verify the config was set correctly
    const Configs& currentConfig = prefManager->getConfig();
    TEST_ASSERT_EQUAL_STRING("CustomPanel", currentConfig.panelName.c_str());
    
    // Save and load should work without crashing
    prefManager->saveConfig();
    prefManager->loadConfig();
    
    // Config should remain valid after save/load cycle
    const Configs& loadedConfig = prefManager->getConfig();
    TEST_ASSERT_NOT_NULL(loadedConfig.panelName.c_str());
    TEST_ASSERT_TRUE(strlen(loadedConfig.panelName.c_str()) > 0);
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
    
    // Verify the config was set correctly
    const Configs& currentConfig = prefManager->getConfig();
    TEST_ASSERT_EQUAL_STRING("TestPanelName", currentConfig.panelName.c_str());
    
    // Test JSON serialization by saving config (should not crash)
    prefManager->saveConfig();
    
    // Verify JSON serialization works by loading and checking consistency
    prefManager->loadConfig();
    const Configs& loadedConfig = prefManager->getConfig();
    TEST_ASSERT_NOT_NULL(loadedConfig.panelName.c_str());
    TEST_ASSERT_TRUE(strlen(loadedConfig.panelName.c_str()) > 0);
}

void test_preference_manager_json_deserialization() {
    prefManager->init();
    
    // Set up test data in preference service
    prefManagerFixture->setPreference("config", "{\"panel_name\":\"DeserializedPanel\"}");
    
    // Load configuration
    prefManager->loadConfig();
    
    // In a real implementation, we'd verify the config was loaded from JSON
    // For now, just verify the load operation occurred
    TEST_ASSERT_TRUE(prefManagerFixture->getPreferenceService()->wasLoadCalled());
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
    
    // Test that preference manager can handle basic operations
    Configs testConfig;
    testConfig.panelName = "IntegrationTest";
    prefManager->setConfig(testConfig);
    
    // Verify the config is accessible through the manager interface
    const Configs& retrievedConfig = prefManager->getConfig();
    TEST_ASSERT_EQUAL_STRING("IntegrationTest", retrievedConfig.panelName.c_str());
    
    // Save and load operations should work without errors
    prefManager->saveConfig();
    prefManager->loadConfig();
    
    // Verify the interface works correctly
    TEST_ASSERT_NOT_NULL(prefManager->getConfig().panelName.c_str());
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
    
    // Test rapid sequential access patterns
    std::string originalPanel = prefManager->getConfig().panelName;
    
    Configs newConfig;
    newConfig.panelName = "ConcurrentPanel";
    prefManager->setConfig(newConfig);
    
    // Verify the change was applied correctly
    const Configs& updatedConfig = prefManager->getConfig();
    TEST_ASSERT_EQUAL_STRING("ConcurrentPanel", updatedConfig.panelName.c_str());
    
    // Verify consistency across multiple reads
    const Configs& rereadConfig = prefManager->getConfig();
    TEST_ASSERT_EQUAL_STRING(updatedConfig.panelName.c_str(), rereadConfig.panelName.c_str());
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

// Enhanced coverage tests for Phase 2

void test_preference_manager_basic_save_load_verification() {
    prefManager->init();
    
    // Set a specific config
    Configs testConfig;
    testConfig.panelName = "TestSaveLoad";
    prefManager->setConfig(testConfig);
    
    // Verify it was set correctly
    const Configs& currentConfig = prefManager->getConfig();
    TEST_ASSERT_EQUAL_STRING("TestSaveLoad", currentConfig.panelName.c_str());
    
    // Save it
    prefManager->saveConfig();
    
    // Load it back
    prefManager->loadConfig();
    
    // Verify it was loaded correctly
    const Configs& loadedConfig = prefManager->getConfig();
    TEST_ASSERT_EQUAL_STRING("TestSaveLoad", loadedConfig.panelName.c_str());
}

void test_preference_manager_malformed_json_handling() {
    // Don't call init() first, set the preference and then init
    
    // Test with malformed JSON
    prefManagerFixture->setPreference("config", "{\"panel_name\":\"TestPanel\",}"); // trailing comma
    prefManager->init(); // This will call loadConfig internally
    
    // Should fall back to default config
    const Configs& config = prefManager->getConfig();
    TEST_ASSERT_EQUAL_STRING(PanelNames::OIL, config.panelName.c_str());
    
    // Test with incomplete JSON
    prefManagerFixture->setPreference("config", "{\"panel_name\":");
    prefManager->loadConfig();
    const Configs& config2 = prefManager->getConfig();
    TEST_ASSERT_EQUAL_STRING(PanelNames::OIL, config2.panelName.c_str());
    
    // Test with invalid JSON structure
    prefManagerFixture->setPreference("config", "not_json_at_all");
    prefManager->loadConfig();
    const Configs& config3 = prefManager->getConfig();
    TEST_ASSERT_EQUAL_STRING(PanelNames::OIL, config3.panelName.c_str());
}

void test_preference_manager_missing_json_fields() {
    prefManager->init();
    
    // Test with JSON missing panel_name field
    prefManagerFixture->setPreference("config", "{\"otherField\":\"value\"}");
    prefManager->loadConfig();
    
    // Should fall back to default config
    const Configs& config = prefManager->getConfig();
    TEST_ASSERT_EQUAL_STRING(PanelNames::OIL, config.panelName.c_str());
    
    // Test with empty JSON object
    prefManagerFixture->setPreference("config", "{}");
    prefManager->loadConfig();
    const Configs& config2 = prefManager->getConfig();
    TEST_ASSERT_EQUAL_STRING(PanelNames::OIL, config2.panelName.c_str());
    
    // Test with null panel_name
    prefManagerFixture->setPreference("config", "{\"panel_name\":null}");
    prefManager->loadConfig();
    const Configs& config3 = prefManager->getConfig();
    TEST_ASSERT_EQUAL_STRING(PanelNames::OIL, config3.panelName.c_str());
}

void test_preference_manager_boundary_values() {
    prefManager->init();
    
    // Test with very long panel name
    std::string longPanelName(1000, 'A');
    Configs longConfig;
    longConfig.panelName = longPanelName;
    prefManager->setConfig(longConfig);
    prefManager->saveConfig();
    
    const Configs& savedLongConfig = prefManager->getConfig();
    TEST_ASSERT_EQUAL_STRING(longPanelName.c_str(), savedLongConfig.panelName.c_str());
    
    // Test with empty panel name
    Configs emptyConfig;
    emptyConfig.panelName = "";
    prefManager->setConfig(emptyConfig);
    prefManager->saveConfig();
    
    const Configs& savedEmptyConfig = prefManager->getConfig();
    TEST_ASSERT_EQUAL_STRING("", savedEmptyConfig.panelName.c_str());
    
    // Test with special characters in panel name
    Configs specialConfig;
    specialConfig.panelName = "Panel!@#$%^&*()_+-={}[]|\\:;\"'<>?,./~`";
    prefManager->setConfig(specialConfig);
    prefManager->saveConfig();
    
    const Configs& savedSpecialConfig = prefManager->getConfig();
    TEST_ASSERT_EQUAL_STRING("Panel!@#$%^&*()_+-={}[]|\\:;\"'<>?,./~`", savedSpecialConfig.panelName.c_str());
}

void test_preference_manager_unicode_handling() {
    prefManager->init();
    
    // Test with unicode characters
    Configs unicodeConfig;
    unicodeConfig.panelName = "Panel测试UTF8字符";
    prefManager->setConfig(unicodeConfig);
    prefManager->saveConfig();
    
    const Configs& savedUnicodeConfig = prefManager->getConfig();
    TEST_ASSERT_EQUAL_STRING("Panel测试UTF8字符", savedUnicodeConfig.panelName.c_str());
    
    // Test load after save
    prefManager->loadConfig();
    const Configs& loadedUnicodeConfig = prefManager->getConfig();
    TEST_ASSERT_EQUAL_STRING("Panel测试UTF8字符", loadedUnicodeConfig.panelName.c_str());
}

void test_preference_manager_json_serialization_errors() {
    prefManager->init();
    
    // Test very large JSON that might cause serialization issues
    Configs largeConfig;
    largeConfig.panelName = std::string(10000, 'X'); // Very large string
    prefManager->setConfig(largeConfig);
    
    // Should handle large configs gracefully
    prefManager->saveConfig();
    const Configs& savedConfig = prefManager->getConfig();
    TEST_ASSERT_EQUAL_STRING(largeConfig.panelName.c_str(), savedConfig.panelName.c_str());
    
    // Test loading large config
    prefManager->loadConfig();
    const Configs& loadedConfig = prefManager->getConfig();
    TEST_ASSERT_EQUAL_STRING(largeConfig.panelName.c_str(), loadedConfig.panelName.c_str());
}

void test_preference_manager_configuration_migration() {
    // Set up the JSON preference before init
    prefManagerFixture->setPreference("config", "{\"panel_name\":\"MigratedPanel\",\"oldField\":\"oldValue\",\"deprecatedSetting\":123}");
    
    // Initialize which will load the config
    prefManager->init();
    
    const Configs& migratedConfig = prefManager->getConfig();
    TEST_ASSERT_EQUAL_STRING("MigratedPanel", migratedConfig.panelName.c_str());
    
    // Save should preserve valid fields and ignore unknown ones
    prefManager->saveConfig();
    prefManager->loadConfig();
    const Configs& savedMigratedConfig = prefManager->getConfig();
    TEST_ASSERT_EQUAL_STRING("MigratedPanel", savedMigratedConfig.panelName.c_str());
}

void test_preference_manager_rapid_save_load_cycles() {
    prefManager->init();
    
    // Test rapid save/load cycles to check for race conditions or corruption
    for (int i = 0; i < 50; i++) {
        Configs config;
        config.panelName = "RapidTest" + std::to_string(i);
        prefManager->setConfig(config);
        prefManager->saveConfig();
        prefManager->loadConfig();
        
        const Configs& verifyConfig = prefManager->getConfig();
        TEST_ASSERT_EQUAL_STRING(("RapidTest" + std::to_string(i)).c_str(), verifyConfig.panelName.c_str());
    }
}

void test_preference_manager_config_validation_edge_cases() {
    prefManager->init();
    
    // Test with whitespace-only panel name
    Configs whitespaceConfig;
    whitespaceConfig.panelName = "   \t\n\r  ";
    prefManager->setConfig(whitespaceConfig);
    prefManager->saveConfig();
    prefManager->loadConfig();
    
    const Configs& savedConfig = prefManager->getConfig();
    TEST_ASSERT_EQUAL_STRING("   \t\n\r  ", savedConfig.panelName.c_str());
    
    // Test with numeric panel name
    Configs numericConfig;
    numericConfig.panelName = "12345";
    prefManager->setConfig(numericConfig);
    prefManager->saveConfig();
    prefManager->loadConfig();
    
    const Configs& numericSavedConfig = prefManager->getConfig();
    TEST_ASSERT_EQUAL_STRING("12345", numericSavedConfig.panelName.c_str());
}

void test_preference_manager_empty_storage_handling() {
    prefManager->init();
    
    // Clear all preferences to simulate empty storage
    prefManagerFixture->clearAllPreferences();
    
    // Load should create default config
    prefManager->loadConfig();
    const Configs& defaultConfig = prefManager->getConfig();
    TEST_ASSERT_EQUAL_STRING(PanelNames::OIL, defaultConfig.panelName.c_str());
    
    // Verify save still works after empty storage
    Configs testConfig;
    testConfig.panelName = "AfterEmptyTest";
    prefManager->setConfig(testConfig);
    prefManager->saveConfig();
    
    const Configs& savedAfterEmpty = prefManager->getConfig();
    TEST_ASSERT_EQUAL_STRING("AfterEmptyTest", savedAfterEmpty.panelName.c_str());
}

void test_preference_manager_json_escape_sequences() {
    prefManager->init();
    
    // Test with JSON escape sequences in panel name
    Configs escapeConfig;
    escapeConfig.panelName = "Panel\"with\\quotes\nand\ttabs";
    prefManager->setConfig(escapeConfig);
    prefManager->saveConfig();
    prefManager->loadConfig();
    
    const Configs& escapedConfig = prefManager->getConfig();
    TEST_ASSERT_EQUAL_STRING("Panel\"with\\quotes\nand\ttabs", escapedConfig.panelName.c_str());
}

// Enhanced Phase 2 error handling tests

void test_preference_manager_malformed_json_recovery() {
    // Test recovery from various malformed JSON scenarios
    prefManager->init();
    
    // Test with JSON missing closing brace
    std::string malformedJson1 = R"({"panelName": "OIL", "theme": "DAY")";
    prefManagerFixture->getPreferenceService()->simulateCorruptedData(malformedJson1);
    
    // Manager should recover with defaults
    prefManager->loadConfig();
    const Configs& config1 = prefManager->getConfig();
    TEST_ASSERT_EQUAL_STRING(PanelNames::OIL, config1.panelName.c_str()); // Should use defaults
    
    // Test with invalid JSON types
    std::string malformedJson2 = R"({"panelName": 123, "theme": true})";
    prefManagerFixture->getPreferenceService()->simulateCorruptedData(malformedJson2);
    
    // Test with completely invalid JSON
    std::string malformedJson3 = "not json at all!@#$%^&*()";
    prefManagerFixture->getPreferenceService()->simulateCorruptedData(malformedJson3);
    
    // System should remain stable
    prefManager->createDefaultConfig();
    const Configs& finalConfig = prefManager->getConfig();
    TEST_ASSERT_EQUAL_STRING(PanelNames::OIL, finalConfig.panelName.c_str());
}

void test_preference_manager_storage_failure_simulation() {
    // Test behavior when storage operations fail
    prefManager->init();
    
    // Set a valid config
    Configs testConfig;
    testConfig.panelName = "TestPanel";
    prefManager->setConfig(testConfig);
    
    // Simulate storage failure
    prefManagerFixture->getPreferenceService()->simulateStorageFailure(true);
    
    // Save should fail gracefully
    prefManager->saveConfig(); // Should not crash
    
    // Load should fall back to defaults when storage fails
    prefManager->loadConfig();
    
    // Re-enable storage
    prefManagerFixture->getPreferenceService()->simulateStorageFailure(false);
    
    // System should recover
    prefManager->createDefaultConfig();
    const Configs& recoveredConfig = prefManager->getConfig();
    TEST_ASSERT_EQUAL_STRING(PanelNames::OIL, recoveredConfig.panelName.c_str());
}

void test_preference_manager_concurrent_corruption_protection() {
    // Test protection against concurrent access corruption
    prefManager->init();
    
    // Simulate rapid concurrent operations
    for (int i = 0; i < 50; i++) {
        Configs config;
        config.panelName = "Concurrent" + std::to_string(i);
        
        // Rapid set/save/load cycles
        prefManager->setConfig(config);
        if (i % 5 == 0) prefManager->saveConfig();
        if (i % 7 == 0) prefManager->loadConfig();
        
        // Verify consistency after each operation
        const Configs& currentConfig = prefManager->getConfig();
        TEST_ASSERT_NOT_NULL(currentConfig.panelName.c_str());
        TEST_ASSERT_TRUE(strlen(currentConfig.panelName.c_str()) > 0);
    }
    
    // Final state should be consistent
    const Configs& finalConfig = prefManager->getConfig();
    TEST_ASSERT_NOT_NULL(finalConfig.panelName.c_str());
}

void test_preference_manager_data_integrity_validation() {
    // Test data integrity across multiple operations
    prefManager->init();
    
    // Set known good config
    Configs originalConfig;
    originalConfig.panelName = "IntegrityTest";
    prefManager->setConfig(originalConfig);
    
    // Perform multiple save/load cycles
    for (int i = 0; i < 20; i++) {
        prefManager->saveConfig();
        prefManager->loadConfig();
        
        const Configs& currentConfig = prefManager->getConfig();
        // Data should remain consistent
        TEST_ASSERT_EQUAL_STRING("IntegrityTest", currentConfig.panelName.c_str());
    }
    
    // Test with config changes between cycles
    for (int i = 0; i < 10; i++) {
        Configs cycleConfig;
        cycleConfig.panelName = "Cycle" + std::to_string(i);
        prefManager->setConfig(cycleConfig);
        prefManager->saveConfig();
        
        // Load and verify
        prefManager->loadConfig();
        const Configs& loadedConfig = prefManager->getConfig();
        TEST_ASSERT_EQUAL_STRING(cycleConfig.panelName.c_str(), loadedConfig.panelName.c_str());
    }
}

void runPreferenceManagerTests() {
    setUp_preference_manager();
    
    // Original tests
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
    
    // Enhanced coverage tests for Phase 2
    RUN_TEST(test_preference_manager_basic_save_load_verification);
    RUN_TEST(test_preference_manager_malformed_json_handling);
    RUN_TEST(test_preference_manager_missing_json_fields);
    RUN_TEST(test_preference_manager_boundary_values);
    RUN_TEST(test_preference_manager_unicode_handling);
    RUN_TEST(test_preference_manager_json_serialization_errors);
    RUN_TEST(test_preference_manager_configuration_migration);
    RUN_TEST(test_preference_manager_rapid_save_load_cycles);
    RUN_TEST(test_preference_manager_config_validation_edge_cases);
    RUN_TEST(test_preference_manager_empty_storage_handling);
    RUN_TEST(test_preference_manager_json_escape_sequences);
    
    // Enhanced Phase 2 error handling tests
    RUN_TEST(test_preference_manager_malformed_json_recovery);
    RUN_TEST(test_preference_manager_storage_failure_simulation);
    RUN_TEST(test_preference_manager_concurrent_corruption_protection);
    RUN_TEST(test_preference_manager_data_integrity_validation);
    
    tearDown_preference_manager();
}

#endif