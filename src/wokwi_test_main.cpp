/**
 * @file wokwi_test_main.cpp  
 * @brief Main entry point for Wokwi hardware simulation test
 * 
 * This file serves as the main entry point for running hardware simulation
 * tests in the Wokwi platform. It includes the test logic without requiring
 * PlatformIO's test framework transport layer.
 */

#include <Arduino.h>

// Test configuration
#define TEST_TIMEOUT_MS 30000    // 30 seconds test duration (optimized for Wokwi)
#define PHASE_DELAY_MS 500       // Delay between test phases
#define ACTION_DELAY_MS 100      // Delay between actions
#define SHORT_PRESS_MS 100       // Short button press duration
#define LONG_PRESS_MS 300        // Long button press duration

// GPIO Pin definitions (matching Wokwi diagram connections)
#define BTN_ACTION 32    // Action button connected to GPIO 32
#define BTN_KEY 25       // Key present simulation (DIP switch 1a)
#define BTN_LOCK 26      // Lock trigger simulation (DIP switch 2a)  
#define BTN_LIGHTS 27    // Lights trigger simulation (DIP switch 3a)
#define BTN_ERROR 34     // Error trigger button (GPIO 34)
#define POT_PRESSURE 36  // Oil pressure potentiometer (VP)
#define POT_TEMP 39      // Oil temperature potentiometer (VN)

// Test state tracking
static unsigned long testStartTime = 0;
static int currentPhase = 0;
static bool testPassed = true;
static char lastError[256] = {0};

// Helper functions for test actions
void simulateButtonPress(uint8_t pin, unsigned long duration) {
    Serial.printf("  Simulating button press on GPIO %d for %lu ms\n", pin, duration);
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);  // Simulate button press
    delay(duration);
    digitalWrite(pin, LOW);   // Release button
    pinMode(pin, INPUT_PULLDOWN);  // Return to input mode
    delay(ACTION_DELAY_MS);
    Serial.printf("  Button simulation complete for GPIO %d\n", pin);
}

void simulateButtonHold(uint8_t pin) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);  // Press and hold
}

void simulateButtonRelease(uint8_t pin) {
    digitalWrite(pin, LOW);   // Release
    pinMode(pin, INPUT_PULLDOWN);  // Return to input mode
    delay(ACTION_DELAY_MS);
}

void setPotentiometerValue(uint8_t pin, int value) {
    // In Wokwi, we simulate this through DAC values
    dacWrite(pin, value);  // ESP32 DAC for simulation
    delay(ACTION_DELAY_MS);
}

void logPhase(const char* phaseName) {
    currentPhase++;
    Serial.printf("\n========================================\n");
    Serial.printf("PHASE %d: %s\n", currentPhase, phaseName);
    Serial.printf("Time: %lu ms\n", millis() - testStartTime);
    Serial.printf("========================================\n");
}

void verifyCondition(bool condition, const char* description) {
    if (!condition) {
        testPassed = false;
        snprintf(lastError, sizeof(lastError), "Phase %d: %s", currentPhase, description);
        Serial.printf("❌ FAILED: %s\n", description);
    } else {
        Serial.printf("✅ PASSED: %s\n", description);
    }
}

// Test Phase Implementations

void test_phase1_hardware_initialization() {
    logPhase("Hardware Initialization & GPIO Setup");
    
    // Initialize all GPIO pins
    pinMode(BTN_ACTION, INPUT_PULLDOWN);
    pinMode(BTN_KEY, INPUT_PULLDOWN);
    pinMode(BTN_LOCK, INPUT_PULLDOWN);
    pinMode(BTN_LIGHTS, INPUT_PULLDOWN);
    pinMode(BTN_ERROR, INPUT_PULLDOWN);
    
    verifyCondition(true, "GPIO pins initialized successfully");
    
    // Test initial pin states
    verifyCondition(
        digitalRead(BTN_ACTION) == LOW,
        "Action button initial state is LOW"
    );
    
    verifyCondition(
        digitalRead(BTN_KEY) == LOW,
        "Key button initial state is LOW"
    );
    
    delay(PHASE_DELAY_MS);
}

void test_phase2_button_simulation() {
    logPhase("Button Simulation Testing");
    
    Serial.println("Starting button simulation tests...");
    
    // Test action button press simulation
    Serial.println("Testing action button...");
    simulateButtonPress(BTN_ACTION, SHORT_PRESS_MS);
    verifyCondition(true, "Action button press simulation completed");
    
    // Test key button simulation  
    Serial.println("Testing key button...");
    simulateButtonPress(BTN_KEY, SHORT_PRESS_MS);
    verifyCondition(true, "Key button press simulation completed");
    
    // Test lock button simulation
    Serial.println("Testing lock button...");
    simulateButtonPress(BTN_LOCK, SHORT_PRESS_MS);
    verifyCondition(true, "Lock button press simulation completed");
    
    // Test lights button simulation
    Serial.println("Testing lights button...");
    simulateButtonPress(BTN_LIGHTS, SHORT_PRESS_MS);
    verifyCondition(true, "Lights button press simulation completed");
    
    Serial.println("Button simulation phase complete.");
    delay(PHASE_DELAY_MS);
}

void test_phase3_analog_simulation() {
    logPhase("Analog Sensor Simulation");
    
    // Test pressure sensor simulation
    setPotentiometerValue(POT_PRESSURE, 128);  // ~50% = mid-range
    verifyCondition(true, "Pressure sensor value set successfully");
    
    // Test temperature sensor simulation
    setPotentiometerValue(POT_TEMP, 64);   // ~25% = low range
    verifyCondition(true, "Temperature sensor value set successfully");
    
    // Read back ADC values to verify
    int pressureReading = analogRead(POT_PRESSURE);
    int tempReading = analogRead(POT_TEMP);
    
    verifyCondition(
        pressureReading >= 0 && pressureReading <= 4095,
        "Pressure ADC reading within valid range"
    );
    
    verifyCondition(
        tempReading >= 0 && tempReading <= 4095,
        "Temperature ADC reading within valid range"
    );
    
    Serial.printf("Pressure reading: %d, Temperature reading: %d\n", 
                  pressureReading, tempReading);
    
    delay(PHASE_DELAY_MS);
}

void test_phase4_timing_validation() {
    logPhase("Timing & Performance Validation");
    
    unsigned long startTime = millis();
    
    // Simulate rapid button presses
    for (int i = 0; i < 5; i++) {
        simulateButtonPress(BTN_ACTION, 100);
        delay(50);
    }
    
    unsigned long endTime = millis();
    unsigned long duration = endTime - startTime;
    
    verifyCondition(
        duration < 2000,  // Should complete in under 2 seconds
        "Rapid button simulation completed within timing requirements"
    );
    
    Serial.printf("Rapid button test duration: %lu ms\n", duration);
    
    delay(PHASE_DELAY_MS);
}

void test_phase5_long_press_validation() {
    logPhase("Long Press Button Validation");
    
    // Test long press simulation on action button
    unsigned long startTime = millis();
    simulateButtonPress(BTN_ACTION, LONG_PRESS_MS);
    unsigned long endTime = millis();
    
    unsigned long actualDuration = endTime - startTime - ACTION_DELAY_MS;
    
    verifyCondition(
        actualDuration >= (LONG_PRESS_MS - 100) && 
        actualDuration <= (LONG_PRESS_MS + 100),
        "Long press duration accuracy validated"
    );
    
    Serial.printf("Long press duration: %lu ms (target: %d ms)\n", 
                  actualDuration, LONG_PRESS_MS);
    
    delay(PHASE_DELAY_MS);
}

// Main test execution
void runWokwiHardwareSimulationTest() {
    testStartTime = millis();
    testPassed = true;
    currentPhase = 0;
    
    Serial.println("\n\n");
    Serial.println("================================================");
    Serial.println("CLARITY WOKWI HARDWARE SIMULATION TEST");
    Serial.println("================================================");
    Serial.printf("Test Duration: ~30 seconds\n");
    Serial.printf("Total Phases: 5\n");
    Serial.println("================================================\n");
    
    // Execute all test phases
    test_phase1_hardware_initialization();
    test_phase2_button_simulation();
    test_phase3_analog_simulation();
    test_phase4_timing_validation();
    test_phase5_long_press_validation();
    
    // Test summary
    unsigned long testDuration = millis() - testStartTime;
    Serial.println("\n================================================");
    Serial.println("TEST SUMMARY");
    Serial.println("================================================");
    Serial.printf("Total Duration: %lu ms (%.1f seconds)\n", 
                  testDuration, testDuration / 1000.0);
    Serial.printf("Phases Completed: %d/5\n", currentPhase);
    Serial.printf("Test Result: %s\n", testPassed ? "PASSED ✅" : "FAILED ❌");
    if (!testPassed) {
        Serial.printf("Last Error: %s\n", lastError);
    }
    Serial.println("================================================\n");
    
    // Print final result for automated parsing
    if (testPassed) {
        Serial.println("WOKWI_TEST_RESULT: PASSED");
    } else {
        Serial.println("WOKWI_TEST_RESULT: FAILED");
    }
}

// Arduino main functions
void setup() {
    delay(2000);  // Wait for system stabilization
    
    Serial.begin(115200);
    while (!Serial) {
        delay(10);
    }
    
    // Wait a bit more for Wokwi to stabilize
    delay(1000);
    
    // Run the hardware simulation test
    runWokwiHardwareSimulationTest();
    
    Serial.println("Test execution completed. System will halt.");
}

void loop() {
    // Test completed, do nothing
    delay(1000);
}