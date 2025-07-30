#pragma once

#include <unity.h>
#include <vector>
#include <functional>
#include <string>
#include <memory>
#include "../../mocks/mock_colors.h"

// Forward declare lvgl types and icons
typedef struct _lv_font_t lv_font_t;
extern "C" {
    extern const lv_font_t lv_font_montserrat_20;
    extern const uint8_t key_solid[];
    extern const uint8_t lock_alt_solid[];
    extern const uint8_t oil_can_regular[];
}

// LVGL Constants needed for tests
#define LV_ALIGN_CENTER 1

// Mock component location type
typedef struct {
    int32_t align;
    int32_t x_offset;
    int32_t y_offset;
} mock_component_location_t;

// Mock reading type
typedef struct {
    int int_value;
    float float_value;
    bool bool_value;
} mock_reading_t;

// Additional mock functions not in mock_colors.h
mock_lv_obj_t* mock_lv_arc_create(mock_lv_obj_t* screen);
mock_lv_obj_t* mock_lv_line_create(mock_lv_obj_t* screen);

// Helper function to create mock_lv_obj_t
inline mock_lv_obj_t create_mock_lv_obj() {
    mock_lv_obj_t obj = {};  // Zero initialize all fields
    obj.styles_applied = false;
    obj.invalidated = false;
    obj.created = false;
    obj.aligned = false;
    obj.deleted = false;
    obj.text_set = false;
    obj.image_set = false;
    obj.align_type = 0;
    obj.x_offset = 0;
    obj.y_offset = 0;
    obj.color_value = 0;
    obj.recolor_opa = 0;
    obj.style = nullptr;
    obj.text_content = nullptr;
    obj.image_src = nullptr;
    return obj;
}

// LVGL Layout Functions
void lv_obj_set_align(mock_lv_obj_t* obj, int32_t align_type);
void lv_obj_set_pos(mock_lv_obj_t* obj, int32_t x, int32_t y);
void mock_lv_obj_align(mock_lv_obj_t* obj, int32_t align, int32_t x_offset, int32_t y_offset);
mock_lv_obj_t* mock_lv_label_create(mock_lv_obj_t* screen);
void mock_lv_label_set_text(mock_lv_obj_t* obj, const char* text);
mock_lv_obj_t* mock_lv_image_create(mock_lv_obj_t* screen);
void mock_lv_image_set_src(mock_lv_obj_t* obj, const void* src);
void mock_lv_obj_set_style_text_font(mock_lv_obj_t* obj, const lv_font_t* font, uint32_t selector);
void mock_lv_obj_set_style_image_recolor(mock_lv_obj_t* obj, mock_lv_color_t color, uint32_t selector);
void mock_lv_obj_set_style_image_recolor_opa(mock_lv_obj_t* obj, uint8_t opa, uint32_t selector);
void mock_lv_obj_del(mock_lv_obj_t* obj);

// Mock GPIO states for testing
extern bool mock_gpio_states[40]; // ESP32 has up to 40 GPIO pins

// Panel manager mock state (defined in test_panel_manager.cpp)
extern std::vector<const char*> panel_creation_history;
extern std::vector<const char*> panel_load_history;
extern bool panel_loaded;
extern bool panel_initialized;

// Sensor mock state (defined in test_sensors.cpp)
extern bool sensor_initialized;
extern uint32_t last_update_time;
extern int32_t current_oil_pressure;
extern int32_t current_oil_temperature;

// Test utilities for mocking hardware
class MockHardware {
public:
    static void reset();
    static void setGpioState(uint8_t pin, bool state);
    static bool getGpioState(uint8_t pin);
    static void simulateAdcReading(uint8_t pin, uint16_t value);
    static uint16_t getAdcReading(uint8_t pin);
    static void simulateAdcFailure(uint8_t pin, bool failed);
    
private:
    static uint16_t mock_adc_readings[40];
    static bool mock_adc_failures[40];
};

// Test scenario framework
struct TriggerEvent {
    const char* triggerId;
    bool pinState;
    uint32_t timestamp;
};

struct ExpectedState {
    const char* expectedPanel;
    const char* expectedTheme;
    std::vector<const char*> activeTriggers;
};

class TriggerScenarioTest {
public:
    void SetupScenario(const char* name);
    void ApplyTriggerSequence(const std::vector<TriggerEvent>& events);
    void ValidateExpectedState(const ExpectedState& expected);
    void LogScenarioResult(bool passed, const char* details);
    
private:
    std::string currentScenario;
    std::vector<std::string> logMessages;
};

// Custom Unity assertions for clarity
#define TEST_ASSERT_PANEL_LOADED(expected_panel) \
    TEST_ASSERT_TRUE_MESSAGE(verifyPanelLoaded(expected_panel), "Panel not loaded correctly")

#define TEST_ASSERT_THEME_APPLIED(expected_theme) \
    TEST_ASSERT_TRUE_MESSAGE(verifyThemeApplied(expected_theme), "Theme not applied correctly")

#define TEST_ASSERT_TRIGGER_STATE(trigger_name, expected_state) \
    TEST_ASSERT_TRUE_MESSAGE(verifyTriggerState(trigger_name, expected_state), "Trigger state incorrect")

// System simulation
void simulateSystemResponse();

// Test verification functions
bool verifyPanelLoaded(const char* panelName);
bool verifyThemeApplied(const char* themeName);
bool verifyTriggerState(const char* triggerName, bool expectedActive);

// Helper function to set GPIO state and trigger system response
void setGpioAndUpdate(uint8_t pin, bool state);

// Helper function to reset sensor mock timing (declared in test_sensors.cpp)
void resetSensorMockTiming();
void forceNextSensorUpdate();
void enableSensorTimingCache();

// Test data generation
std::vector<TriggerEvent> generateRapidToggleSequence();
std::vector<TriggerEvent> generateMultipleTriggerSequence();
std::vector<TriggerEvent> generateEdgeCaseSequence();

// Memory and performance testing
void measureMemoryUsage();
void measureResponseTime(std::function<void()> operation);

// Scenario generators based on scenarios.md
namespace TestScenarios {
    // S1: System Startup Scenarios
    std::vector<TriggerEvent> cleanStartup();
    std::vector<TriggerEvent> startupWithKeyPresent();
    std::vector<TriggerEvent> startupWithKeyNotPresent();
    std::vector<TriggerEvent> startupWithLock();
    std::vector<TriggerEvent> startupWithTheme();
    
    // S2: Single Trigger Scenarios
    std::vector<TriggerEvent> lightsTrigger();
    std::vector<TriggerEvent> lockTrigger();
    std::vector<TriggerEvent> keyPresentTrigger();
    std::vector<TriggerEvent> keyNotPresentTrigger();
    
    // S3: Multiple Trigger Scenarios
    std::vector<TriggerEvent> priorityOverrideKeyOverLock();
    std::vector<TriggerEvent> keyPresentVsKeyNotPresent();
    std::vector<TriggerEvent> keyNotPresentVsKeyPresent();
    std::vector<TriggerEvent> themeAndPanelTriggers();
    std::vector<TriggerEvent> tripleTriggerActivation();
    
    // S4: Edge Case Scenarios
    std::vector<TriggerEvent> rapidToggleSingle();
    std::vector<TriggerEvent> rapidToggleMultiple();
    std::vector<TriggerEvent> allTriggersRapid();
    std::vector<TriggerEvent> simultaneousDeactivation();
    std::vector<TriggerEvent> invalidTriggerCombinations();
}

// Expected states for validation
namespace ExpectedStates {
    extern const ExpectedState OIL_PANEL_DAY;
    extern const ExpectedState OIL_PANEL_NIGHT;
    extern const ExpectedState KEY_PANEL_GREEN;
    extern const ExpectedState KEY_PANEL_RED;
    extern const ExpectedState LOCK_PANEL;
    extern const ExpectedState KEY_PANEL_GREEN_NIGHT;
    extern const ExpectedState KEY_PANEL_RED_NIGHT;
    extern const ExpectedState LOCK_PANEL_NIGHT;
}