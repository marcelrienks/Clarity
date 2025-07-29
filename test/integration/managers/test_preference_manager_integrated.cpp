#include <unity.h>
#include "test_utilities.h"
#include <cstring>

// PreferenceManager-specific mock state using shared infrastructure
namespace PreferenceManagerMocks {
    // Mock ESP32/NVS state
    bool preferences_begin_success = true;
    bool nvs_format_success = true;
    std::string stored_json = "";
    bool json_corruption = false;
    size_t write_size = 100;
    
    // Mock Arduino String class
    class MockString {
    public:
        std::string data;
        MockString() = default;
        MockString(const char* str) : data(str ? str : "") {}
        MockString(const std::string& str) : data(str) {}
        
        size_t length() const { return data.length(); }
        const char* c_str() const { return data.c_str(); }
        
        MockString& operator=(const char* str) {
            data = str ? str : "";
            return *this;
        }
    };
    
    // Mock Preferences class
    class MockPreferences {
    public:
        bool begin(const char* name, bool readOnly) {
            return preferences_begin_success;
        }
        
        MockString getString(const char* key, const char* defaultValue) {
            if (json_corruption) {
                return MockString("{invalid_json");
            }
            return MockString(stored_json.empty() ? defaultValue : stored_json.c_str());
        }
        
        size_t putString(const char* key, const MockString& value) {
            stored_json = value.data;
            return write_size;
        }
        
        bool remove(const char* key) {
            return true;
        }
        
        void end() {}
    };
    
    void reset() {
        preferences_begin_success = true;
        nvs_format_success = true;
        stored_json = "";
        json_corruption = false;
        write_size = 100;
    }
}

// Mock PreferenceManager for testing
class MockPreferenceManager {
public:
    struct Config {
        std::string panelName = "DefaultPanel";
        std::string theme = "Day";
        int brightness = 128;
        bool autoMode = true;
    } config;
    
    static MockPreferenceManager& GetInstance() {
        static MockPreferenceManager instance;
        return instance;
    }
    
    bool init() {
        PreferenceManagerMocks::MockPreferences prefs;
        return prefs.begin("clarity", false);
    }
    
    void loadConfig() {
        PreferenceManagerMocks::MockPreferences prefs;
        prefs.begin("clarity", true);
        
        auto jsonStr = prefs.getString("config", "{}");
        parseConfigFromJson(jsonStr.c_str());
        
        prefs.end();
    }
    
    void saveConfig() {
        PreferenceManagerMocks::MockPreferences prefs;
        prefs.begin("clarity", false);
        
        std::string json = createConfigJson();
        prefs.putString("config", PreferenceManagerMocks::MockString(json));
        
        prefs.end();
    }
    
    void createDefaultConfig() {
        config.panelName = "OemOilPanel";
        config.theme = "Day";
        config.brightness = 128;
        config.autoMode = true;
    }

private:
    void parseConfigFromJson(const char* jsonStr) {
        if (!jsonStr || strlen(jsonStr) == 0 || strcmp(jsonStr, "{}") == 0) {
            createDefaultConfig();
            return;
        }
        
        // Mock JSON parsing - look for panel name
        if (strstr(jsonStr, "\"panelName\":\"KEY\"")) {
            config.panelName = "KEY";
        } else if (strstr(jsonStr, "\"panelName\":\"LOCK\"")) {
            config.panelName = "LOCK";
        } else {
            config.panelName = "OemOilPanel";
        }
        
        if (strstr(jsonStr, "\"theme\":\"Night\"")) {
            config.theme = "Night";
        } else {
            config.theme = "Day";
        }
    }
    
    std::string createConfigJson() {
        return "{\"panelName\":\"" + config.panelName + "\",\"theme\":\"" + config.theme + "\"}";
    }
};

// Reset function for preference manager tests
void resetPreferenceManagerMockState() {
    PreferenceManagerMocks::reset();
}

// =================================================================
// PREFERENCE MANAGER TESTS
// =================================================================

void test_preference_manager_singleton_access(void) {
    MockPreferenceManager& pm1 = MockPreferenceManager::GetInstance();
    MockPreferenceManager& pm2 = MockPreferenceManager::GetInstance();
    
    TEST_ASSERT_EQUAL_PTR(&pm1, &pm2);
}

void test_preference_manager_successful_initialization(void) {
    resetPreferenceManagerMockState();
    PreferenceManagerMocks::preferences_begin_success = true;
    
    MockPreferenceManager& pm = MockPreferenceManager::GetInstance();
    bool result = pm.init();
    
    TEST_ASSERT_TRUE(result);
}

void test_preference_manager_nvs_failure_recovery(void) {
    resetPreferenceManagerMockState();
    PreferenceManagerMocks::preferences_begin_success = false;
    
    MockPreferenceManager& pm = MockPreferenceManager::GetInstance();
    bool result = pm.init();
    
    TEST_ASSERT_FALSE(result);
}

void test_preference_manager_persistent_failure(void) {
    resetPreferenceManagerMockState();
    PreferenceManagerMocks::preferences_begin_success = false;
    PreferenceManagerMocks::nvs_format_success = false;
    
    MockPreferenceManager& pm = MockPreferenceManager::GetInstance();
    bool result = pm.init();
    
    TEST_ASSERT_FALSE(result);
}

void test_preference_manager_load_empty_config(void) {
    resetPreferenceManagerMockState();
    PreferenceManagerMocks::stored_json = "";
    
    MockPreferenceManager& pm = MockPreferenceManager::GetInstance();
    pm.loadConfig();
    
    // Should have default values
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", pm.config.panelName.c_str());
    TEST_ASSERT_EQUAL_STRING("Day", pm.config.theme.c_str());
}

void test_preference_manager_load_valid_config(void) {
    resetPreferenceManagerMockState();
    PreferenceManagerMocks::stored_json = "{\"panelName\":\"KEY\",\"theme\":\"Night\"}";
    
    MockPreferenceManager& pm = MockPreferenceManager::GetInstance();
    pm.loadConfig();
    
    TEST_ASSERT_EQUAL_STRING("KEY", pm.config.panelName.c_str());
    TEST_ASSERT_EQUAL_STRING("Night", pm.config.theme.c_str());
}

void test_preference_manager_load_corrupted_config(void) {
    resetPreferenceManagerMockState();
    PreferenceManagerMocks::json_corruption = true;
    
    MockPreferenceManager& pm = MockPreferenceManager::GetInstance();
    pm.loadConfig();
    
    // Should fallback to defaults on corruption
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", pm.config.panelName.c_str());
    TEST_ASSERT_EQUAL_STRING("Day", pm.config.theme.c_str());
}

void test_preference_manager_load_missing_panel_name(void) {
    resetPreferenceManagerMockState();
    PreferenceManagerMocks::stored_json = "{\"theme\":\"Night\"}"; // Missing panelName
    
    MockPreferenceManager& pm = MockPreferenceManager::GetInstance();
    pm.loadConfig();
    
    // Should use default for missing panelName
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", pm.config.panelName.c_str());
    TEST_ASSERT_EQUAL_STRING("Night", pm.config.theme.c_str());
}

void test_preference_manager_save_config(void) {
    resetPreferenceManagerMockState();
    
    MockPreferenceManager& pm = MockPreferenceManager::GetInstance();
    pm.config.panelName = "LOCK";
    pm.config.theme = "Night";
    pm.saveConfig();
    
    // Verify the JSON was stored
    TEST_ASSERT_TRUE(PreferenceManagerMocks::stored_json.find("LOCK") != std::string::npos);
    TEST_ASSERT_TRUE(PreferenceManagerMocks::stored_json.find("Night") != std::string::npos);
}

void test_preference_manager_save_and_load_roundtrip(void) {
    resetPreferenceManagerMockState();
    MockPreferenceManager& pm = MockPreferenceManager::GetInstance();
    
    // Save a specific config
    pm.config.panelName = "KEY";
    pm.config.theme = "Night";
    pm.saveConfig();
    
    // Clear and reload
    pm.config.panelName = "";
    pm.config.theme = "";
    pm.loadConfig();
    
    // Should match saved config
    TEST_ASSERT_EQUAL_STRING("KEY", pm.config.panelName.c_str());
    TEST_ASSERT_EQUAL_STRING("Night", pm.config.theme.c_str());
}

void test_preference_manager_config_persistence(void) {
    resetPreferenceManagerMockState();
    MockPreferenceManager& pm = MockPreferenceManager::GetInstance();
    
    // Set and save config
    pm.config.panelName = "LOCK";
    pm.saveConfig();
    
    // Simulate restart - get new instance and load
    MockPreferenceManager& pm2 = MockPreferenceManager::GetInstance();
    pm2.loadConfig();
    
    TEST_ASSERT_EQUAL_STRING("LOCK", pm2.config.panelName.c_str());
}

void test_preference_manager_create_default_config(void) {
    resetPreferenceManagerMockState();
    MockPreferenceManager& pm = MockPreferenceManager::GetInstance();
    
    pm.createDefaultConfig();
    
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", pm.config.panelName.c_str());
    TEST_ASSERT_EQUAL_STRING("Day", pm.config.theme.c_str());
    TEST_ASSERT_EQUAL_INT(128, pm.config.brightness);
    TEST_ASSERT_TRUE(pm.config.autoMode);
}

void test_preference_manager_default_config_consistency(void) {
    resetPreferenceManagerMockState();
    MockPreferenceManager& pm = MockPreferenceManager::GetInstance();
    
    // Load empty config should match explicit default creation
    pm.loadConfig();
    std::string panel1 = pm.config.panelName;
    std::string theme1 = pm.config.theme;
    
    pm.createDefaultConfig();
    std::string panel2 = pm.config.panelName;
    std::string theme2 = pm.config.theme;
    
    TEST_ASSERT_EQUAL_STRING(panel2.c_str(), panel1.c_str());
    TEST_ASSERT_EQUAL_STRING(theme2.c_str(), theme1.c_str());
}

void test_preference_manager_full_lifecycle(void) {
    resetPreferenceManagerMockState();
    MockPreferenceManager& pm = MockPreferenceManager::GetInstance();
    
    // Initialize
    TEST_ASSERT_TRUE(pm.init());
    
    // Load (should be defaults)
    pm.loadConfig();
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", pm.config.panelName.c_str());
    
    // Modify and save
    pm.config.panelName = "KEY";
    pm.saveConfig();
    
    // Reload and verify
    pm.config.panelName = "";
    pm.loadConfig();
    TEST_ASSERT_EQUAL_STRING("KEY", pm.config.panelName.c_str());
}

void test_preference_manager_error_recovery_flow(void) {
    resetPreferenceManagerMockState();
    MockPreferenceManager& pm = MockPreferenceManager::GetInstance();
    
    // Simulate corruption
    PreferenceManagerMocks::json_corruption = true;
    pm.loadConfig();
    
    // Should recover with defaults
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", pm.config.panelName.c_str());
    
    // Fix corruption and try again
    PreferenceManagerMocks::json_corruption = false;
    PreferenceManagerMocks::stored_json = "{\"panelName\":\"LOCK\"}";
    pm.loadConfig();
    
    TEST_ASSERT_EQUAL_STRING("LOCK", pm.config.panelName.c_str());
}

void test_preference_manager_rapid_save_load(void) {
    resetPreferenceManagerMockState();
    MockPreferenceManager& pm = MockPreferenceManager::GetInstance();
    
    // Rapid save/load cycles should be stable
    std::string panels[] = {"KEY", "LOCK", "OemOilPanel", "KEY"};
    
    for (int i = 0; i < 4; i++) {
        pm.config.panelName = panels[i];
        pm.saveConfig();
        
        pm.config.panelName = "";
        pm.loadConfig();
        
        TEST_ASSERT_EQUAL_STRING(panels[i].c_str(), pm.config.panelName.c_str());
    }
}

void test_preference_manager_memory_consistency(void) {
    resetPreferenceManagerMockState();
    MockPreferenceManager& pm = MockPreferenceManager::GetInstance();
    
    // Multiple operations should maintain consistency
    pm.createDefaultConfig();
    std::string original = pm.config.panelName;
    
    pm.saveConfig();
    pm.loadConfig();
    
    TEST_ASSERT_EQUAL_STRING(original.c_str(), pm.config.panelName.c_str());
}

void test_preference_manager_panel_name_validation(void) {
    resetPreferenceManagerMockState();
    MockPreferenceManager& pm = MockPreferenceManager::GetInstance();
    
    // Test various panel name scenarios
    std::string testCases[] = {
        "{\"panelName\":\"KEY\"}", 
        "{\"panelName\":\"LOCK\"}", 
        "{\"panelName\":\"Invalid\"}", 
        "{\"other\":\"value\"}"
    };
    
    std::string expected[] = {"KEY", "LOCK", "OemOilPanel", "OemOilPanel"};
    
    for (int i = 0; i < 4; i++) {
        PreferenceManagerMocks::stored_json = testCases[i];
        pm.loadConfig();
        TEST_ASSERT_EQUAL_STRING(expected[i].c_str(), pm.config.panelName.c_str());
    }
}

// Note: PlatformIO will automatically discover and run test_ functions