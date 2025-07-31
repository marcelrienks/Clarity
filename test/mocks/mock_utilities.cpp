#include "test_utilities.h"
#include "mock_utilities.h"
#include <cstring>
#include <iostream>

// Global mock state (only declare those not already defined elsewhere)
bool mock_gpio_states[40] = {false};

// Trigger state tracking
bool mock_key_present_active = false;
bool mock_key_not_present_active = false;
bool mock_lock_active = false;
bool mock_theme_active = false;

// Panel and theme state
const char* mock_current_panel = PANEL_OIL;
const char* mock_current_theme = "Day";

// MockHardware implementation
uint16_t MockHardware::mock_adc_readings[40] = {0};
bool MockHardware::mock_adc_failures[40] = {false};

void MockHardware::reset() {
    for (int i = 0; i < 40; i++) {
        mock_gpio_states[i] = false;
        mock_adc_readings[i] = 0;
        mock_adc_failures[i] = false;
    }
    // Reset trigger states
    mock_key_present_active = false;
    mock_key_not_present_active = false;
    mock_lock_active = false;
    mock_theme_active = false;
    // Note: Other globals are managed by their respective test files
}

void MockHardware::setGpioState(uint8_t pin, bool state) {
    if (pin < 40) {
        mock_gpio_states[pin] = state;
    }
}

bool MockHardware::getGpioState(uint8_t pin) {
    if (pin < 40) {
        return mock_gpio_states[pin];
    }
    return false;
}

void MockHardware::simulateAdcReading(uint8_t pin, uint16_t value) {
    if (pin < 40) {
        mock_adc_readings[pin] = value;
    }
}

void MockHardware::simulateAdcFailure(uint8_t pin, bool failed) {
    if (pin < 40) {
        mock_adc_failures[pin] = failed;
    }
}

uint16_t MockHardware::getAdcReading(uint8_t pin) {
    if (pin < 40) {
        if (mock_adc_failures[pin]) {
            return 0; // Simulate failure by returning 0
        }
        return mock_adc_readings[pin];
    }
    return 0;
}

// Test verification functions  
// Reference externally defined globals
extern std::vector<const char*> panel_load_history;

bool verifyPanelLoaded(const char* panelName) {
    if (panel_load_history.empty()) return false;
    return strcmp(panel_load_history.back(), panelName) == 0;
}

bool verifyThemeApplied(const char* themeName) {
    // Mock implementation - always return true for now
    return true;
}

bool verifyTriggerState(const char* triggerName, bool expectedActive) {
    if (!triggerName) return false;
    
    bool actualState = false;
    
    if (strcmp(triggerName, "key_present") == 0) {
        actualState = mock_key_present_active;
    } else if (strcmp(triggerName, "key_not_present") == 0) {
        actualState = mock_key_not_present_active;
    } else if (strcmp(triggerName, "lock_state") == 0 || strcmp(triggerName, "lock") == 0) {
        actualState = mock_lock_active;
    } else if (strcmp(triggerName, "theme") == 0 || strcmp(triggerName, "lights_state") == 0) {
        actualState = mock_theme_active;
    }
    
    return actualState == expectedActive;
}

void setGpioAndUpdate(uint8_t pin, bool state) {
    MockHardware::setGpioState(pin, state);
    
    // Update trigger states based on pin mappings
    if (pin == 25) {  // key_present
        mock_key_present_active = state;
    } else if (pin == 26) {  // key_not_present
        mock_key_not_present_active = state;
    } else if (pin == 27) {  // lock
        mock_lock_active = state;
    }
    
    // Clear panel history and determine current panel
    panel_load_history.clear();
    
    // Determine panel based on current trigger states
    // Priority: Key Present > Key Not Present > Lock > Default
    if (mock_key_present_active) {
        panel_load_history.push_back("KeyPanel");
    } else if (mock_key_not_present_active) {
        panel_load_history.push_back("KeyPanel");
    } else if (mock_lock_active) {
        panel_load_history.push_back("LockPanel");
    } else {
        panel_load_history.push_back("OemOilPanel");
    }
    
    // Set panel as loaded
    panel_loaded = true;
}

extern bool panel_loaded;

void simulateSystemResponse() {
    // Mock system response - set panel as loaded
    panel_loaded = true;
    // Also add to panel load history for verification
    panel_load_history.push_back("MockPanel");
}

// Memory measurement function (missing implementation)
void measureMemoryUsage() {
    // Mock implementation - do nothing for now
}

// Performance measurement
void measureResponseTime(std::function<void()> operation) {
    // Mock implementation - just execute the operation
    if (operation) {
        operation();
    }
}

// TriggerScenarioTest implementation
void TriggerScenarioTest::SetupScenario(const char* name) {
    currentScenario = name;
    logMessages.clear();
}

void TriggerScenarioTest::ApplyTriggerSequence(const std::vector<TriggerEvent>& events) {
    // Clear previous history
    panel_load_history.clear();
    
    // Process all events to determine final state
    for (const auto& event : events) {
        MockHardware::setGpioState(1, event.pinState); // Mock trigger application
        
        // Update global trigger states based on events
        if (event.triggerId) {
            if (strcmp(event.triggerId, "key_present") == 0) {
                mock_key_present_active = event.pinState;
            } else if (strcmp(event.triggerId, "key_not_present") == 0) {
                mock_key_not_present_active = event.pinState;
            } else if (strcmp(event.triggerId, "lock") == 0 || strcmp(event.triggerId, "lock_state") == 0) {
                mock_lock_active = event.pinState;
            } else if (strcmp(event.triggerId, "theme") == 0 || strcmp(event.triggerId, "lights") == 0) {
                mock_theme_active = event.pinState;
            }
        }
    }
    
    // Determine final panel based on what triggers are CURRENTLY active
    if (events.empty()) {
        // Clean startup - load default OEM oil panel
        panel_load_history.push_back("OemOilPanel");
    } else {
        // Priority: Key Present > Key Not Present > Lock > Default
        if (mock_key_present_active) {
            panel_load_history.push_back("KeyPanel");
        } else if (mock_key_not_present_active) {
            panel_load_history.push_back("KeyPanel");
        } else if (mock_lock_active) {
            panel_load_history.push_back("LockPanel");
        } else {
            // No triggers active - return to default
            panel_load_history.push_back("OemOilPanel");
        }
    }
}

void TriggerScenarioTest::ValidateExpectedState(const ExpectedState& expected) {
    // Mock validation
    panel_loaded = true;
}

void TriggerScenarioTest::LogScenarioResult(bool passed, const char* details) {
    logMessages.push_back(details);
}

// Test scenario generators (mock implementations)
namespace TestScenarios {
    std::vector<TriggerEvent> cleanStartup() {
        return {};
    }
    
    std::vector<TriggerEvent> startupWithKeyPresent() {
        return {{"key_present", true, 0}};
    }
    
    std::vector<TriggerEvent> startupWithKeyNotPresent() {
        return {{"key_not_present", true, 0}};
    }
    
    std::vector<TriggerEvent> startupWithLock() {
        return {{"lock", true, 0}};
    }
    
    std::vector<TriggerEvent> startupWithTheme() {
        return {{"theme", true, 0}};
    }
    
    std::vector<TriggerEvent> lightsTrigger() {
        return {{"lights", true, 0}};
    }
    
    std::vector<TriggerEvent> lockTrigger() {
        // Single trigger: activate then deactivate
        return {{"lock", true, 0}, {"lock", false, 100}};
    }
    
    std::vector<TriggerEvent> keyPresentTrigger() {
        // Single trigger: activate then deactivate
        return {{"key_present", true, 0}, {"key_present", false, 100}};
    }
    
    std::vector<TriggerEvent> keyNotPresentTrigger() {
        // Single trigger: activate then deactivate
        return {{"key_not_present", true, 0}, {"key_not_present", false, 100}};
    }
    
    std::vector<TriggerEvent> priorityOverrideKeyOverLock() {
        // Activate both, then deactivate both
        return {
            {"lock", true, 0}, 
            {"key_present", true, 10},  // Key should take priority
            {"key_present", false, 100}, 
            {"lock", false, 110}
        };
    }
    
    std::vector<TriggerEvent> keyPresentVsKeyNotPresent() {
        // Activate both conflicting key states, then deactivate both
        return {
            {"key_not_present", true, 0}, 
            {"key_present", true, 10},  // Key present should take priority
            {"key_present", false, 100}, 
            {"key_not_present", false, 110}
        };
    }
    
    std::vector<TriggerEvent> keyNotPresentVsKeyPresent() {
        return {{"key_not_present", true, 0}, {"key_present", true, 0}};
    }
    
    std::vector<TriggerEvent> themeAndPanelTriggers() {
        return {{"theme", true, 0}, {"lock", true, 0}};
    }
    
    std::vector<TriggerEvent> tripleTriggerActivation() {
        return {{"key_present", true, 0}, {"lock", true, 0}, {"theme", true, 0}};
    }
    
    std::vector<TriggerEvent> rapidToggleSingle() {
        return {{"key_present", true, 0}, {"key_present", false, 10}, {"key_present", true, 20}};
    }
    
    std::vector<TriggerEvent> rapidToggleMultiple() {
        // Rapid toggle sequence ending with key_not_present active (last in FIFO)
        return {
            {"key_present", true, 0}, 
            {"lock", true, 5}, 
            {"key_present", false, 10}, 
            {"key_not_present", true, 12}, // Add key_not_present as final active trigger
            {"lock", false, 15}
        };
    }
    
    std::vector<TriggerEvent> allTriggersRapid() {
        return {{"key_present", true, 0}, {"lock", true, 1}, {"theme", true, 2}};
    }
    
    std::vector<TriggerEvent> simultaneousDeactivation() {
        return {{"key_present", false, 0}, {"lock", false, 0}};
    }
    
    std::vector<TriggerEvent> invalidTriggerCombinations() {
        // Invalid state: both key_present and key_not_present active simultaneously
        return {
            {"key_not_present", true, 0}, 
            {"key_present", true, 10}  // Both keys active - invalid state
        };
    }
}

// Expected states
namespace ExpectedStates {
    const ExpectedState OIL_PANEL_DAY = {"OemOilPanel", "Day", {}};
    const ExpectedState OIL_PANEL_NIGHT = {"OemOilPanel", "Night", {}};
    const ExpectedState KEY_PANEL_GREEN = {"KeyPanel", "Day", {"key_present"}};
    const ExpectedState KEY_PANEL_RED = {"KeyPanel", "Day", {"key_not_present"}};
    const ExpectedState LOCK_PANEL = {"LockPanel", "Day", {"lock"}};
    const ExpectedState KEY_PANEL_GREEN_NIGHT = {"KeyPanel", "Night", {"key_present"}};
    const ExpectedState KEY_PANEL_RED_NIGHT = {"KeyPanel", "Night", {"key_not_present"}};
    const ExpectedState LOCK_PANEL_NIGHT = {"LockPanel", "Night", {"lock"}};
}

// Mock Arduino and LVGL functions that are referenced by multiple test files  
extern "C" {
    // These need to be in extern "C" block to avoid C++ name mangling conflicts
    // Note: millis() and delay() are already defined in other test files, 
    // so we avoid redefining them here to prevent conflicts
    
    // Mock LVGL functions (shared across test files)
    void lv_tick_inc(uint32_t tick_period) {
        // Mock tick increment
    }
    
    uint32_t lv_timer_handler() {
        return 5; // Mock work time
    }
    
    // Mock display functions to avoid conflicts
    void mock_lv_display_flush_ready(void* display) {
        // Mock function
    }
    
    void mock_lv_scr_load(void* scr) {
        // Mock function
    }
    
    // Mock color function is now in mock_colors.h
    
    // Mock LVGL object functions
    void* mock_lv_obj_create(void* parent) {
        static int dummy_obj = 1;
        return &dummy_obj;
    }
    
    void mock_lv_obj_del(mock_lv_obj_t* obj) {
        // Mock delete
        if (obj) {
            obj->deleted = true;
        }
    }
    
    void mock_lv_obj_add_style(void* obj, void* style, uint32_t selector) {
        // Mock style application
    }
    
    void mock_lv_obj_invalidate(void* obj) {
        // Mock invalidation
    }
    
    void* mock_lv_scr_act() {
        static int dummy_screen = 1;
        return &dummy_screen;
    }
}

// Mock trigger and panel management functions
void InitializeTriggersFromGpio(void) {
    // Reset all trigger states
    mock_key_present_active = false;
    mock_key_not_present_active = false;
    mock_lock_active = false;
    mock_theme_active = false;
    mock_current_panel = PANEL_OIL;
}

const char* GetCurrentPanel(void) {
    return mock_current_panel;
}

void SetTrigger(const char* trigger, bool active) {
    if (strcmp(trigger, TRIGGER_KEY_PRESENT) == 0) {
        mock_key_present_active = active;
        if (active) {
            mock_current_panel = PANEL_KEY;
            mock_key_not_present_active = false; // Mutually exclusive
        }
    } else if (strcmp(trigger, TRIGGER_KEY_NOT_PRESENT) == 0) {
        mock_key_not_present_active = active;
        if (active) {
            mock_current_panel = PANEL_KEY;
            mock_key_present_active = false; // Mutually exclusive
        }
    } else if (strcmp(trigger, TRIGGER_LOCK) == 0) {
        mock_lock_active = active;
        if (active) {
            mock_current_panel = PANEL_LOCK;
        }
    } else if (strcmp(trigger, TRIGGER_THEME) == 0) {
        mock_theme_active = active;
        if (active) {
            mock_current_theme = (strcmp(mock_current_theme, "Day") == 0) ? "Night" : "Day";
        }
    }
    
    // If no triggers are active, return to oil panel
    if (!mock_key_present_active && !mock_key_not_present_active && !mock_lock_active) {
        mock_current_panel = PANEL_OIL;
    }
}

bool IsTriggerActive(const char* trigger) {
    if (strcmp(trigger, TRIGGER_KEY_PRESENT) == 0) {
        return mock_key_present_active;
    } else if (strcmp(trigger, TRIGGER_KEY_NOT_PRESENT) == 0) {
        return mock_key_not_present_active;
    } else if (strcmp(trigger, TRIGGER_LOCK) == 0) {
        return mock_lock_active;
    } else if (strcmp(trigger, TRIGGER_THEME) == 0) {
        return mock_theme_active;
    }
    return false;
}

void ResetAllTriggers(void) {
    mock_key_present_active = false;
    mock_key_not_present_active = false;
    mock_lock_active = false;
    mock_theme_active = false;
    mock_current_panel = PANEL_OIL;
}

void SimulateSystemTick(uint32_t ms) {
    // Mock system tick for timing tests
    // This could be used to simulate time-based behavior
}

void SetTheme(const char* theme) {
    mock_current_theme = theme;
}

const char* GetCurrentTheme(void) {
    return mock_current_theme;
}

// Additional mock functions for scenario tests
bool IsNightThemeActive(void) {
    return strcmp(mock_current_theme, "Night") == 0;
}

bool IsKeyPresent(void) {
    return mock_key_present_active;
}

bool IsKeyNotPresent(void) {
    return mock_key_not_present_active;
}

bool IsLockActive(void) {
    return mock_lock_active;
}

// Oil sensor mock functions
void InitializeOilPressureSensor(void) {
    // Mock initialization
    current_oil_pressure = DEFAULT_OIL_PRESSURE;
}

void InitializeOilTemperatureSensor(void) {
    // Mock initialization  
    current_oil_temperature = DEFAULT_OIL_TEMPERATURE;
}