#include "mock_types.h"
#include <vector>
#include <string>

// Declare global variables used across tests
std::vector<const char*> panel_creation_history;
std::vector<const char*> panel_load_history;
bool panel_loaded = false;
bool panel_initialized = false;
bool sensor_initialized = false;
uint32_t last_update_time = 0;
int32_t current_oil_pressure = 0;
int32_t current_oil_temperature = 0;

// Reset functions
void resetPreferenceManagerMockState() {
    // Implementation
}

void resetMockUtilitiesState() {
    // Implementation
}

void resetStyleManagerMockState() {
    // Implementation
}

void resetDeviceMockState() {
    // Implementation
}

void resetSensorMockTiming() {
    // Implementation
}

// Mock implementations of system functions
bool IsTriggerActive(int trigger) {
    // Implementation
    return false;
}

void SetTrigger(int trigger, bool state) {
    // Implementation
}

int GetCurrentPanel() {
    // Implementation
    return 0;
}

bool IsKeyPresent() {
    // Implementation
    return false;
}

bool IsLockActive() {
    // Implementation
    return false;
}

bool IsNightThemeActive() {
    // Implementation
    return false;
}

void InitializeTriggersFromGpio() {
    // Implementation
}
