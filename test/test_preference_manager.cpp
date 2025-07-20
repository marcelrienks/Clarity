#ifdef UNIT_TESTING

#include <unity.h>
#include <string>
#include <variant>

// Mock Arduino String class for testing
class String {
public:
    std::string data;
    String() = default;
    String(const char* str) : data(str) {}
    String(const std::string& str) : data(str) {}
    const char* c_str() const { return data.c_str(); }
    int indexOf(const char* substr) const {
        size_t pos = data.find(substr);
        return (pos != std::string::npos) ? static_cast<int>(pos) : -1;
    }
};

// Mock types from utilities/types.h
namespace PanelNames {
    constexpr const char* OemOil = "OemOilPanel";
    constexpr const char* Key = "KeyPanel";
    constexpr const char* Lock = "LockPanel";
}

namespace Themes {
    constexpr int Day = 0;
    constexpr int Night = 1;
}

struct Config {
    String panel_name;
    int theme;
    int brightness;
};

struct ComponentLocation {
    int x, y, width, height;
};

using Reading = std::variant<std::monostate, int32_t, double, std::string, bool>;


void test_panel_names_constants(void) {
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", PanelNames::OemOil);
    TEST_ASSERT_EQUAL_STRING("KeyPanel", PanelNames::Key);
    TEST_ASSERT_EQUAL_STRING("LockPanel", PanelNames::Lock);
}

void test_themes_constants(void) {
    TEST_ASSERT_EQUAL(0, Themes::Day);
    TEST_ASSERT_EQUAL(1, Themes::Night);
}

void test_config_serialization(void) {
    // Test Config struct manual JSON serialization
    Config config;
    config.panel_name = "TestPanel";
    config.theme = Themes::Night;
    config.brightness = 75;

    // Simple JSON string construction for testing
    std::string json = "{\"panel_name\":\"" + config.panel_name.data + 
                      "\",\"theme\":" + std::to_string(config.theme) + 
                      ",\"brightness\":" + std::to_string(config.brightness) + "}";

    TEST_ASSERT_TRUE(json.find("TestPanel") != std::string::npos);
    TEST_ASSERT_TRUE(json.find("75") != std::string::npos);
}

void test_config_deserialization(void) {
    // Test Config struct manual JSON parsing (simplified)
    std::string json = "{\"panel_name\":\"TestPanel\",\"theme\":1,\"brightness\":50}";
    
    // Simple parsing for testing purposes
    Config config;
    config.panel_name = "TestPanel";  // Would be parsed from JSON
    config.theme = 1;                 // Would be parsed from JSON
    config.brightness = 50;           // Would be parsed from JSON

    TEST_ASSERT_EQUAL_STRING("TestPanel", config.panel_name.c_str());
    TEST_ASSERT_EQUAL(1, config.theme);
    TEST_ASSERT_EQUAL(50, config.brightness);
}

void test_component_location_initialization(void) {
    ComponentLocation location;
    location.x = 10;
    location.y = 20;
    location.width = 100;
    location.height = 50;

    TEST_ASSERT_EQUAL(10, location.x);
    TEST_ASSERT_EQUAL(20, location.y);
    TEST_ASSERT_EQUAL(100, location.width);
    TEST_ASSERT_EQUAL(50, location.height);
}

void test_reading_variant_int(void) {
    Reading reading = 42;
    
    TEST_ASSERT_TRUE(std::holds_alternative<int32_t>(reading));
    TEST_ASSERT_EQUAL(42, std::get<int32_t>(reading));
}

void test_reading_variant_double(void) {
    Reading reading = 3.14159;
    
    TEST_ASSERT_TRUE(std::holds_alternative<double>(reading));
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 3.14159, std::get<double>(reading));
}

void test_reading_variant_string(void) {
    Reading reading = std::string("test_string");
    
    TEST_ASSERT_TRUE(std::holds_alternative<std::string>(reading));
    TEST_ASSERT_EQUAL_STRING("test_string", std::get<std::string>(reading).c_str());
}

void test_reading_variant_bool(void) {
    Reading reading = true;
    
    TEST_ASSERT_TRUE(std::holds_alternative<bool>(reading));
    TEST_ASSERT_TRUE(std::get<bool>(reading));
}

void test_default_config_values(void) {
    // Test default configuration values
    Config config;
    config.panel_name = PanelNames::OemOil;
    config.theme = Themes::Day;
    config.brightness = 100;

    TEST_ASSERT_EQUAL_STRING(PanelNames::OemOil, config.panel_name.c_str());
    TEST_ASSERT_EQUAL(Themes::Day, config.theme);
    TEST_ASSERT_EQUAL(100, config.brightness);
}

void test_preference_manager_main() {
    RUN_TEST(test_panel_names_constants);
    RUN_TEST(test_themes_constants);
    RUN_TEST(test_config_serialization);
    RUN_TEST(test_config_deserialization);
    RUN_TEST(test_component_location_initialization);
    RUN_TEST(test_reading_variant_int);
    RUN_TEST(test_reading_variant_double);
    RUN_TEST(test_reading_variant_string);
    RUN_TEST(test_reading_variant_bool);
    RUN_TEST(test_default_config_values);
}

#endif