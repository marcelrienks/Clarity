#include <unity.h>

// Test function declarations
extern void runPreferenceManagerTests();
extern void runTriggerManagerTests();
extern void runPanelManagerTests();
extern void runStyleManagerTests();
extern void runKeySensorTests();
extern void runLockSensorTests();
extern void runLightSensorTests();
extern void runOilPressureSensorTests();
extern void runOilTemperatureSensorTests();
extern void runGpioProviderTests();
extern void runServiceContainerTests();
extern void runTickerTests();

void setUp(void) {
    // Set up before each test
}

void tearDown(void) {
    // Clean up after each test
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    // Manager Tests
    runPreferenceManagerTests();
    runTriggerManagerTests();
    runPanelManagerTests();
    runStyleManagerTests();
    
    // Sensor Tests
    runKeySensorTests();
    runLockSensorTests();
    runLightSensorTests();
    runOilPressureSensorTests();
    runOilTemperatureSensorTests();
    
    // Provider Tests
    runGpioProviderTests();
    
    // System Tests
    runServiceContainerTests();
    
    // Utility Tests
    runTickerTests();
    
    return UNITY_END();
}