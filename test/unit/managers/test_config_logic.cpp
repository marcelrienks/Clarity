#include <unity.h>
#include <string>
#include <map>

#ifdef UNIT_TESTING

// Test configuration management logic
class ConfigManager {
private:
    std::map<std::string, std::string> config;
    
public:
    void setConfig(const std::string& key, const std::string& value) {
        config[key] = value;
    }
    
    std::string getConfig(const std::string& key, const std::string& defaultValue = "") {
        auto it = config.find(key);
        return (it != config.end()) ? it->second : defaultValue;
    }
    
    bool hasConfig(const std::string& key) {
        return config.find(key) != config.end();
    }
    
    void createDefaultConfig() {
        config["panelName"] = "OIL";
        config["theme"] = "DAY";
    }
    
    size_t getConfigCount() const {
        return config.size();
    }
    
    void clearConfig() {
        config.clear();
    }
};

// Test panel name validation
bool isValidPanelName(const std::string& panelName) {
    return panelName == "OIL" || panelName == "KEY" || panelName == "LOCK";
}

// Test theme validation
bool isValidTheme(const std::string& theme) {
    return theme == "DAY" || theme == "NIGHT";
}

#endif

void test_config_set_get() {
    ConfigManager manager;
    
    // Test setting and getting config
    manager.setConfig("testKey", "testValue");
    std::string value = manager.getConfig("testKey");
    TEST_ASSERT_EQUAL_STRING("testValue", value.c_str());
    
    // Test default value for missing key
    std::string defaultValue = manager.getConfig("missingKey", "default");
    TEST_ASSERT_EQUAL_STRING("default", defaultValue.c_str());
}

void test_config_has_config() {
    ConfigManager manager;
    
    // Test key doesn't exist initially
    TEST_ASSERT_FALSE(manager.hasConfig("testKey"));
    
    // Test key exists after setting
    manager.setConfig("testKey", "value");
    TEST_ASSERT_TRUE(manager.hasConfig("testKey"));
}

void test_config_default_creation() {
    ConfigManager manager;
    
    // Create default config
    manager.createDefaultConfig();
    
    // Verify default values
    TEST_ASSERT_EQUAL_STRING("OIL", manager.getConfig("panelName").c_str());
    TEST_ASSERT_EQUAL_STRING("DAY", manager.getConfig("theme").c_str());
    TEST_ASSERT_EQUAL(2, manager.getConfigCount());
}

void test_panel_name_validation() {
    TEST_ASSERT_TRUE(isValidPanelName("OIL"));
    TEST_ASSERT_TRUE(isValidPanelName("KEY"));
    TEST_ASSERT_TRUE(isValidPanelName("LOCK"));
    TEST_ASSERT_FALSE(isValidPanelName("INVALID"));
    TEST_ASSERT_FALSE(isValidPanelName(""));
}

void test_theme_validation() {
    TEST_ASSERT_TRUE(isValidTheme("DAY"));
    TEST_ASSERT_TRUE(isValidTheme("NIGHT"));
    TEST_ASSERT_FALSE(isValidTheme("INVALID"));
    TEST_ASSERT_FALSE(isValidTheme(""));
}

void test_config_clear() {
    ConfigManager manager;
    
    // Add some config
    manager.setConfig("key1", "value1");
    manager.setConfig("key2", "value2");
    TEST_ASSERT_EQUAL(2, manager.getConfigCount());
    
    // Clear config
    manager.clearConfig();
    TEST_ASSERT_EQUAL(0, manager.getConfigCount());
    TEST_ASSERT_FALSE(manager.hasConfig("key1"));
}

void runConfigLogicTests() {
    RUN_TEST(test_config_set_get);
    RUN_TEST(test_config_has_config);
    RUN_TEST(test_config_default_creation);
    RUN_TEST(test_panel_name_validation);
    RUN_TEST(test_theme_validation);
    RUN_TEST(test_config_clear);
}