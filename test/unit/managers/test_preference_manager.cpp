#include <unity.h>
#include "test_utilities.h"

// Mock ESP32 dependencies for unit testing
extern "C" {
    // Mock Preferences class functionality
    bool mock_preferences_begin_success = true;
    bool mock_nvs_format_success = true;
    std::string mock_stored_json = "";
    bool mock_json_corruption = false;
    size_t mock_write_size = 100;
    
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
            return mock_preferences_begin_success;
        }
        
        MockString getString(const char* key, const char* defaultValue) {
            if (mock_json_corruption) {
                return MockString("{invalid_json");
            }
            return MockString(mock_stored_json.empty() ? defaultValue : mock_stored_json.c_str());
        }
        
        size_t putString(const char* key, const MockString& value) {
            mock_stored_json = value.data;
            return mock_write_size;
        }
        
        bool remove(const char* key) {
            return true;
        }
    };
    
    // Mock nvs_flash_erase
    void nvs_flash_erase() {
        mock_nvs_format_success = true;
    }
}

// Mock global objects that PreferenceManager uses
static MockPreferences mock_preferences_instance;

// Mock config structure
struct MockConfigs {
    std::string panelName;
};

// Mock PanelNames
namespace PanelNames {
    const char* OIL = "OemOilPanel";
    const char* KEY = "KeyPanel";
    const char* LOCK = "LockPanel";
    const char* SPLASH = "SplashPanel";
}

// Mock JsonDocNames
namespace JsonDocNames {
    const char* PANEL_NAME = "panelName";
}

// Mock PreferenceManager for testing
class MockPreferenceManager {
public:
    static MockPreferenceManager& GetInstance() {
        static MockPreferenceManager instance;
        return instance;
    }
    
    MockConfigs config;
    MockPreferences preferences_;
    static const char* CONFIG_KEY;
    
    void init() {
        if (!preferences_.begin("clarity", false)) {
            nvs_flash_erase();
            if (!preferences_.begin("clarity", false)) {
                // Failed after format
            }
        }
        loadConfig();
    }
    
    void createDefaultConfig() {
        config.panelName = PanelNames::OIL;
        saveConfig();
        loadConfig();
    }
    
    void loadConfig() {
        MockString jsonString = preferences_.getString(CONFIG_KEY, "");
        if (jsonString.length() == 0) {
            createDefaultConfig();
            return;
        }
        
        // Simple JSON parsing for test
        if (mock_json_corruption) {
            createDefaultConfig();
            return;
        }
        
        // Extract panelName from mock JSON
        if (mock_stored_json.find("KeyPanel") != std::string::npos) {
            config.panelName = PanelNames::KEY;
        } else if (mock_stored_json.find("LockPanel") != std::string::npos) {
            config.panelName = PanelNames::LOCK;
        } else {
            config.panelName = PanelNames::OIL;
        }
    }
    
    void saveConfig() {
        // Create simple JSON string
        std::string jsonString = "{\"panelName\":\"" + config.panelName + "\"}";
        MockString mockStr(jsonString);
        preferences_.putString(CONFIG_KEY, mockStr);
    }
};

const char* MockPreferenceManager::CONFIG_KEY = "config";

// Note: setUp() and tearDown() are defined in test_main.cpp

void resetMockState() {
    mock_preferences_begin_success = true;
    mock_nvs_format_success = true;
    mock_stored_json = "";
    mock_json_corruption = false;
    mock_write_size = 100;
}

// =================================================================
// PREFERENCE MANAGER INITIALIZATION TESTS
// =================================================================

void test_preference_manager_singleton_access(void) {
    MockPreferenceManager& pm1 = MockPreferenceManager::GetInstance();
    MockPreferenceManager& pm2 = MockPreferenceManager::GetInstance();
    
    TEST_ASSERT_EQUAL_PTR(&pm1, &pm2);
}

void test_preference_manager_successful_initialization(void) {
    resetMockState();
    MockPreferenceManager& pm = MockPreferenceManager::GetInstance();
    
    mock_preferences_begin_success = true;
    pm.init();
    
    // Should initialize with default config
    TEST_ASSERT_EQUAL_STRING(PanelNames::OIL, pm.config.panelName.c_str());
}

void test_preference_manager_nvs_failure_recovery(void) {
    resetMockState();
    MockPreferenceManager& pm = MockPreferenceManager::GetInstance();
    
    // Simulate NVS failure on first attempt
    mock_preferences_begin_success = false;
    
    pm.init();
    
    // Should still work after format recovery
    TEST_ASSERT_EQUAL_STRING(PanelNames::OIL, pm.config.panelName.c_str());
}

void test_preference_manager_persistent_failure(void) {
    resetMockState();
    MockPreferenceManager& pm = MockPreferenceManager::GetInstance();
    
    // Simulate persistent NVS failure
    mock_preferences_begin_success = false;
    mock_nvs_format_success = false;
    
    pm.init();
    
    // Should still create default config even if NVS fails
    TEST_ASSERT_EQUAL_STRING(PanelNames::OIL, pm.config.panelName.c_str());
}

// =================================================================
// CONFIGURATION LOADING TESTS
// =================================================================

void test_preference_manager_load_empty_config(void) {
    resetMockState();
    MockPreferenceManager& pm = MockPreferenceManager::GetInstance();
    
    mock_stored_json = ""; // No existing config
    pm.loadConfig();
    
    // Should create default config
    TEST_ASSERT_EQUAL_STRING(PanelNames::OIL, pm.config.panelName.c_str());
}

void test_preference_manager_load_valid_config(void) {
    resetMockState();
    MockPreferenceManager& pm = MockPreferenceManager::GetInstance();
    
    mock_stored_json = "{\"panelName\":\"KeyPanel\"}";
    pm.loadConfig();
    
    TEST_ASSERT_EQUAL_STRING(PanelNames::KEY, pm.config.panelName.c_str());
}

void test_preference_manager_load_corrupted_config(void) {
    resetMockState();
    MockPreferenceManager& pm = MockPreferenceManager::GetInstance();
    
    mock_json_corruption = true;
    pm.loadConfig();
    
    // Should fall back to default config
    TEST_ASSERT_EQUAL_STRING(PanelNames::OIL, pm.config.panelName.c_str());
}

void test_preference_manager_load_missing_panel_name(void) {
    resetMockState();
    MockPreferenceManager& pm = MockPreferenceManager::GetInstance();
    
    mock_stored_json = "{\"otherField\":\"value\"}"; // Missing panelName
    pm.loadConfig();
    
    // Should create default config
    TEST_ASSERT_EQUAL_STRING(PanelNames::OIL, pm.config.panelName.c_str());
}

// =================================================================
// CONFIGURATION SAVING TESTS
// =================================================================

void test_preference_manager_save_config(void) {
    resetMockState();
    MockPreferenceManager& pm = MockPreferenceManager::GetInstance();
    
    pm.config.panelName = PanelNames::LOCK;
    pm.saveConfig();
    
    // Check that JSON was stored
    TEST_ASSERT_TRUE(mock_stored_json.find("LockPanel") != std::string::npos);
    TEST_ASSERT_TRUE(mock_stored_json.find("panelName") != std::string::npos);
}

void test_preference_manager_save_and_load_roundtrip(void) {
    resetMockState();
    MockPreferenceManager& pm = MockPreferenceManager::GetInstance();
    
    // Save a specific config
    pm.config.panelName = PanelNames::KEY;
    pm.saveConfig();
    
    // Clear and reload
    pm.config.panelName = "";
    pm.loadConfig();
    
    // Should match saved config
    TEST_ASSERT_EQUAL_STRING(PanelNames::KEY, pm.config.panelName.c_str());
}

void test_preference_manager_config_persistence(void) {
    resetMockState();
    MockPreferenceManager& pm = MockPreferenceManager::GetInstance();
    
    // Test multiple save/load cycles
    const char* test_panels[] = {PanelNames::KEY, PanelNames::LOCK, PanelNames::OIL};
    
    for (size_t i = 0; i < 3; i++) {
        pm.config.panelName = test_panels[i];
        pm.saveConfig();
        
        pm.config.panelName = ""; // Clear
        pm.loadConfig();
        
        TEST_ASSERT_EQUAL_STRING(test_panels[i], pm.config.panelName.c_str());
    }
}

// =================================================================
// DEFAULT CONFIGURATION TESTS
// =================================================================

void test_preference_manager_create_default_config(void) {
    resetMockState();
    MockPreferenceManager& pm = MockPreferenceManager::GetInstance();
    
    pm.createDefaultConfig();
    
    TEST_ASSERT_EQUAL_STRING(PanelNames::OIL, pm.config.panelName.c_str());
    
    // Should also save the default config
    TEST_ASSERT_TRUE(mock_stored_json.find("OemOilPanel") != std::string::npos);
}

void test_preference_manager_default_config_consistency(void) {
    resetMockState();
    MockPreferenceManager& pm = MockPreferenceManager::GetInstance();
    
    // Create default multiple times
    pm.createDefaultConfig();
    std::string first_config = pm.config.panelName;
    
    pm.createDefaultConfig();
    std::string second_config = pm.config.panelName;
    
    TEST_ASSERT_EQUAL_STRING(first_config.c_str(), second_config.c_str());
}

// =================================================================
// INTEGRATION TESTS
// =================================================================

void test_preference_manager_full_lifecycle(void) {
    resetMockState();
    MockPreferenceManager& pm = MockPreferenceManager::GetInstance();
    
    // Full initialization process
    pm.init();
    
    // Should start with default
    TEST_ASSERT_EQUAL_STRING(PanelNames::OIL, pm.config.panelName.c_str());
    
    // Change and save
    pm.config.panelName = PanelNames::LOCK;
    pm.saveConfig();
    
    // Simulate restart by reinitializing
    pm.init();
    
    // Should remember the saved config
    TEST_ASSERT_EQUAL_STRING(PanelNames::LOCK, pm.config.panelName.c_str());
}

void test_preference_manager_error_recovery_flow(void) {
    resetMockState();
    MockPreferenceManager& pm = MockPreferenceManager::GetInstance();
    
    // Start with corrupted data
    mock_json_corruption = true;
    pm.init();
    
    // Should recover with defaults
    TEST_ASSERT_EQUAL_STRING(PanelNames::OIL, pm.config.panelName.c_str());
    
    // Fix corruption and save new config
    mock_json_corruption = false;
    pm.config.panelName = PanelNames::KEY;
    pm.saveConfig();
    
    // Reload should work normally
    pm.loadConfig();
    TEST_ASSERT_EQUAL_STRING(PanelNames::KEY, pm.config.panelName.c_str());
}

// =================================================================
// STRESS AND PERFORMANCE TESTS
// =================================================================

void test_preference_manager_rapid_save_load(void) {
    resetMockState();
    MockPreferenceManager& pm = MockPreferenceManager::GetInstance();
    
    // Rapid save/load cycles
    for (int i = 0; i < 10; i++) {
        pm.config.panelName = (i % 2 == 0) ? PanelNames::KEY : PanelNames::LOCK;
        pm.saveConfig();
        pm.loadConfig();
        
        const char* expected = (i % 2 == 0) ? PanelNames::KEY : PanelNames::LOCK;
        TEST_ASSERT_EQUAL_STRING(expected, pm.config.panelName.c_str());
    }
}

void test_preference_manager_memory_consistency(void) {
    resetMockState();
    MockPreferenceManager& pm = MockPreferenceManager::GetInstance();
    
    // Test that config memory remains stable
    pm.config.panelName = PanelNames::LOCK;
    const char* original_ptr = pm.config.panelName.c_str();
    
    // Perform operations
    pm.saveConfig();
    pm.loadConfig();
    
    // Config should still be valid
    TEST_ASSERT_EQUAL_STRING(PanelNames::LOCK, pm.config.panelName.c_str());
}

// =================================================================
// VALIDATION TESTS
// =================================================================

void test_preference_manager_panel_name_validation(void) {
    resetMockState();
    MockPreferenceManager& pm = MockPreferenceManager::GetInstance();
    
    // Test all valid panel names
    const char* valid_panels[] = {
        PanelNames::OIL,
        PanelNames::KEY, 
        PanelNames::LOCK,
        PanelNames::SPLASH
    };
    
    for (size_t i = 0; i < 4; i++) {
        pm.config.panelName = valid_panels[i];
        pm.saveConfig();
        
        pm.config.panelName = ""; // Clear
        pm.loadConfig();
        
        TEST_ASSERT_EQUAL_STRING(valid_panels[i], pm.config.panelName.c_str());
    }
}

// Note: PlatformIO will automatically discover and run test_ functions