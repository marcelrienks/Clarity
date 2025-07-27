#include "test_utilities.h"
#include <cstring>
#include <chrono>

// Mock hardware state
bool mock_gpio_states[40] = {false};
uint16_t MockHardware::mock_adc_readings[40] = {0};

// Mock current system state for testing
static const char* current_panel = "OemOilPanel";
static const char* current_theme = "Day";
static bool trigger_states[10] = {false}; // key_present, key_not_present, lock_state, etc.

void MockHardware::reset() {
    memset(mock_gpio_states, false, sizeof(mock_gpio_states));
    memset(mock_adc_readings, 0, sizeof(mock_adc_readings));
    current_panel = "OemOilPanel";
    current_theme = "Day";
    memset(trigger_states, false, sizeof(trigger_states));
}

void MockHardware::setGpioState(uint8_t pin, bool state) {
    if (pin < 40) {
        mock_gpio_states[pin] = state;
    }
}

bool MockHardware::getGpioState(uint8_t pin) {
    return pin < 40 ? mock_gpio_states[pin] : false;
}

void MockHardware::simulateAdcReading(uint8_t pin, uint16_t value) {
    if (pin < 40) {
        mock_adc_readings[pin] = value;
    }
}

uint16_t MockHardware::getAdcReading(uint8_t pin) {
    return pin < 40 ? mock_adc_readings[pin] : 0;
}

// TriggerScenarioTest implementation
void TriggerScenarioTest::SetupScenario(const char* name) {
    currentScenario = name;
    logMessages.clear();
    MockHardware::reset();
}

void TriggerScenarioTest::ApplyTriggerSequence(const std::vector<TriggerEvent>& events) {
    for (const auto& event : events) {
        // Simulate GPIO pin changes based on trigger events
        if (strcmp(event.triggerId, "key_present") == 0) {
            MockHardware::setGpioState(25, event.pinState); // GPIO 25
            trigger_states[0] = event.pinState;
        } else if (strcmp(event.triggerId, "key_not_present") == 0) {
            MockHardware::setGpioState(26, event.pinState); // GPIO 26
            trigger_states[1] = event.pinState;
        } else if (strcmp(event.triggerId, "lock_state") == 0) {
            MockHardware::setGpioState(27, event.pinState); // GPIO 27
            trigger_states[2] = event.pinState;
        } else if (strcmp(event.triggerId, "lights_state") == 0) {
            MockHardware::setGpioState(28, event.pinState); // GPIO 28 (mock)
            trigger_states[3] = event.pinState;
        }
        
        // Simulate system response to trigger changes
        simulateSystemResponse();
    }
}

void TriggerScenarioTest::ValidateExpectedState(const ExpectedState& expected) {
    // Validate panel state
    if (strcmp(current_panel, expected.expectedPanel) != 0) {
        LogScenarioResult(false, "Panel mismatch");
        return;
    }
    
    // Validate theme state
    if (strcmp(current_theme, expected.expectedTheme) != 0) {
        LogScenarioResult(false, "Theme mismatch");
        return;
    }
    
    // Validate trigger states
    for (const char* trigger : expected.activeTriggers) {
        if (!verifyTriggerState(trigger, true)) {
            LogScenarioResult(false, "Trigger state mismatch");
            return;
        }
    }
    
    LogScenarioResult(true, "All validations passed");
}

void TriggerScenarioTest::LogScenarioResult(bool passed, const char* details) {
    char message[256];
    snprintf(message, sizeof(message), "Scenario '%s': %s - %s", 
             currentScenario.c_str(), passed ? "PASS" : "FAIL", details);
    logMessages.push_back(message);
}

// Mock system response simulation
void simulateSystemResponse() {
    // Priority-based panel determination (simplified mock)
    if (trigger_states[0]) { // key_present (highest priority)
        current_panel = "KeyPanel";
    } else if (trigger_states[1]) { // key_not_present
        current_panel = "KeyPanel"; 
    } else if (trigger_states[2]) { // lock_state
        current_panel = "LockPanel";
    } else {
        current_panel = "OemOilPanel"; // default
    }
    
    // Theme determination
    if (trigger_states[3]) { // lights_state
        current_theme = "Night";
    } else {
        current_theme = "Day";
    }
}

// Verification functions
bool verifyPanelLoaded(const char* panelName) {
    return strcmp(current_panel, panelName) == 0;
}

bool verifyThemeApplied(const char* themeName) {
    return strcmp(current_theme, themeName) == 0;
}

bool verifyTriggerState(const char* triggerName, bool expectedActive) {
    if (strcmp(triggerName, "key_present") == 0) {
        return trigger_states[0] == expectedActive;
    } else if (strcmp(triggerName, "key_not_present") == 0) {
        return trigger_states[1] == expectedActive;
    } else if (strcmp(triggerName, "lock_state") == 0) {
        return trigger_states[2] == expectedActive;
    } else if (strcmp(triggerName, "lights_state") == 0) {
        return trigger_states[3] == expectedActive;
    }
    return false;
}

// Performance testing
void measureMemoryUsage() {
    // Mock implementation - in real tests would measure actual memory
}

void measureResponseTime(std::function<void()> operation) {
    auto start = std::chrono::high_resolution_clock::now();
    operation();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    // Store or assert on duration as needed
}

// Test scenario generators based on scenarios.md
namespace TestScenarios {
    
    std::vector<TriggerEvent> cleanStartup() {
        return {}; // No triggers active
    }
    
    std::vector<TriggerEvent> startupWithKeyPresent() {
        return {{"key_present", true, 0}};
    }
    
    std::vector<TriggerEvent> startupWithKeyNotPresent() {
        return {{"key_not_present", true, 0}};
    }
    
    std::vector<TriggerEvent> startupWithLock() {
        return {{"lock_state", true, 0}};
    }
    
    std::vector<TriggerEvent> startupWithTheme() {
        return {{"lights_state", true, 0}};
    }
    
    std::vector<TriggerEvent> lockTrigger() {
        return {
            {"lock_state", true, 100},   // Activate lock
            {"lock_state", false, 200}   // Deactivate lock
        };
    }
    
    std::vector<TriggerEvent> keyPresentTrigger() {
        return {
            {"key_present", true, 100},   // Key inserted
            {"key_present", false, 200}   // Key removed
        };
    }
    
    std::vector<TriggerEvent> keyNotPresentTrigger() {
        return {
            {"key_not_present", true, 100},   // Key explicitly not present
            {"key_not_present", false, 200}   // State cleared
        };
    }
    
    std::vector<TriggerEvent> priorityOverrideKeyOverLock() {
        return {
            {"lock_state", true, 100},      // Lock first
            {"key_present", true, 200},     // Key overrides lock
            {"key_present", false, 300},    // Key removed, lock restored
            {"lock_state", false, 400}      // Lock removed, oil restored
        };
    }
    
    std::vector<TriggerEvent> keyPresentVsKeyNotPresent() {
        return {
            {"key_present", true, 100},     // Key present first
            {"key_not_present", true, 200}, // Key not present overrides (FIFO)
            {"key_present", false, 300},    // Key present removed
            {"key_not_present", false, 400} // All keys inactive
        };
    }
    
    std::vector<TriggerEvent> rapidToggleSingle() {
        return {
            {"key_present", true, 10},
            {"key_present", false, 20},
            {"key_present", true, 30}
        };
    }
    
    std::vector<TriggerEvent> rapidToggleMultiple() {
        return {
            {"key_present", true, 10},
            {"key_not_present", true, 20},
            {"key_present", false, 30}
        };
    }
    
    std::vector<TriggerEvent> invalidTriggerCombinations() {
        return {
            {"key_present", true, 100},
            {"key_not_present", true, 110} // Both keys active - invalid
        };
    }
    
    std::vector<TriggerEvent> simultaneousDeactivation() {
        return {
            {"key_present", true, 100},
            {"lock_state", true, 110},
            {"lights_state", true, 120},
            {"key_present", false, 200},    // All deactivated at once
            {"lock_state", false, 200},
            {"lights_state", false, 200}
        };
    }
}

// Expected states
namespace ExpectedStates {
    const ExpectedState OIL_PANEL_DAY = {"OemOilPanel", "Day", {}};
    const ExpectedState OIL_PANEL_NIGHT = {"OemOilPanel", "Night", {"lights_state"}};
    const ExpectedState KEY_PANEL_GREEN = {"KeyPanel", "Day", {"key_present"}};
    const ExpectedState KEY_PANEL_RED = {"KeyPanel", "Day", {"key_not_present"}};
    const ExpectedState LOCK_PANEL = {"LockPanel", "Day", {"lock_state"}};
    const ExpectedState KEY_PANEL_GREEN_NIGHT = {"KeyPanel", "Night", {"key_present", "lights_state"}};
    const ExpectedState KEY_PANEL_RED_NIGHT = {"KeyPanel", "Night", {"key_not_present", "lights_state"}};
    const ExpectedState LOCK_PANEL_NIGHT = {"LockPanel", "Night", {"lock_state", "lights_state"}};
}