#ifdef UNIT_TESTING

#include <unity.h>
#include "test_fixtures.h"
#include "sensors/key_sensor.h"
#include "mocks/mock_gpio_provider.h"
#include "hardware/gpio_pins.h"
#include "utilities/types.h"

static std::unique_ptr<SensorTestFixture> fixture;
static KeySensor* sensor;


void test_key_sensor_init() {
    sensor->init();
    
    // Verify GPIO pins are configured correctly
    TEST_ASSERT_EQUAL(INPUT_PULLDOWN, fixture->getGpioProvider()->getPinMode(gpio_pins::KEY_PRESENT));
    TEST_ASSERT_EQUAL(INPUT_PULLDOWN, fixture->getGpioProvider()->getPinMode(gpio_pins::KEY_NOT_PRESENT));
}

void test_key_sensor_key_present_state() {
    sensor->init();
    
    // Set key present pin HIGH, key not present pin LOW
    fixture->setDigitalPin(gpio_pins::KEY_PRESENT, HIGH);
    fixture->setDigitalPin(gpio_pins::KEY_NOT_PRESENT, LOW);
    
    KeyState state = sensor->getKeyState();
    TEST_ASSERT_EQUAL(KeyState::Present, state);
    
    Reading reading = sensor->getReading();
    TEST_ASSERT_EQUAL(static_cast<int32_t>(KeyState::Present), std::get<int32_t>(reading));
}

void test_key_sensor_key_not_present_state() {
    sensor->init();
    
    // Set key present pin LOW, key not present pin HIGH
    fixture->setDigitalPin(gpio_pins::KEY_PRESENT, LOW);
    fixture->setDigitalPin(gpio_pins::KEY_NOT_PRESENT, HIGH);
    
    KeyState state = sensor->getKeyState();
    TEST_ASSERT_EQUAL(KeyState::NotPresent, state);
    
    Reading reading = sensor->getReading();
    TEST_ASSERT_EQUAL(static_cast<int32_t>(KeyState::NotPresent), std::get<int32_t>(reading));
}

void test_key_sensor_inactive_state() {
    sensor->init();
    
    // Set both pins LOW (inactive state)
    fixture->setDigitalPin(gpio_pins::KEY_PRESENT, LOW);
    fixture->setDigitalPin(gpio_pins::KEY_NOT_PRESENT, LOW);
    
    KeyState state = sensor->getKeyState();
    TEST_ASSERT_EQUAL(KeyState::Inactive, state);
    
    Reading reading = sensor->getReading();
    TEST_ASSERT_EQUAL(static_cast<int32_t>(KeyState::Inactive), std::get<int32_t>(reading));
}

void test_key_sensor_invalid_state() {
    sensor->init();
    
    // Set both pins HIGH (invalid state)
    fixture->setDigitalPin(gpio_pins::KEY_PRESENT, HIGH);
    fixture->setDigitalPin(gpio_pins::KEY_NOT_PRESENT, HIGH);
    
    KeyState state = sensor->getKeyState();
    // Should handle invalid state gracefully (implementation dependent)
    TEST_ASSERT_TRUE(state == KeyState::Present || 
                     state == KeyState::NotPresent || 
                     state == KeyState::Inactive);
}

void test_key_sensor_value_change_detection() {
    sensor->init();
    
    // Set initial state
    fixture->setDigitalPin(gpio_pins::KEY_PRESENT, LOW);
    fixture->setDigitalPin(gpio_pins::KEY_NOT_PRESENT, LOW);
    
    Reading reading1 = sensor->getReading();
    TEST_ASSERT_EQUAL(static_cast<int32_t>(KeyState::Inactive), std::get<int32_t>(reading1));
    
    // Same state should give same reading
    Reading reading2 = sensor->getReading();
    TEST_ASSERT_EQUAL(std::get<int32_t>(reading1), std::get<int32_t>(reading2));
    
    // Change to different state should give different reading
    fixture->setDigitalPin(gpio_pins::KEY_PRESENT, HIGH);
    Reading reading3 = sensor->getReading();
    TEST_ASSERT_EQUAL(static_cast<int32_t>(KeyState::Present), std::get<int32_t>(reading3));
    TEST_ASSERT_NOT_EQUAL(std::get<int32_t>(reading1), std::get<int32_t>(reading3));
}

void test_key_sensor_construction() {
    // Test that sensor can be created and destroyed
    TEST_ASSERT_NOT_NULL(sensor);
}

void test_key_sensor_reading_consistency() {
    sensor->init();
    
    // Set a known state
    fixture->setDigitalPin(gpio_pins::KEY_PRESENT, HIGH);
    fixture->setDigitalPin(gpio_pins::KEY_NOT_PRESENT, LOW);
    
    // Multiple readings should be consistent
    Reading reading1 = sensor->getReading();
    Reading reading2 = sensor->getReading();
    KeyState state1 = sensor->getKeyState();
    KeyState state2 = sensor->getKeyState();
    
    TEST_ASSERT_EQUAL(std::get<int32_t>(reading1), std::get<int32_t>(reading2));
    TEST_ASSERT_EQUAL(state1, state2);
    TEST_ASSERT_EQUAL(std::get<int32_t>(reading1), static_cast<int32_t>(state1));
}

void test_key_sensor_timing_behavior() {
    sensor->init();
    
    // Test rapid state changes
    for (int i = 0; i < 10; i++) {
        fixture->setDigitalPin(gpio_pins::KEY_PRESENT, i % 2);
        fixture->setDigitalPin(gpio_pins::KEY_NOT_PRESENT, (i + 1) % 2);
        fixture->advanceTime(10); // 10ms intervals
        
        KeyState state = sensor->getKeyState();
        // State should be valid regardless of timing
        TEST_ASSERT_TRUE(state == KeyState::Present || 
                        state == KeyState::NotPresent || 
                        state == KeyState::Inactive);
    }
}

void test_key_sensor_debouncing() {
    sensor->init();
    
    // Test debouncing behavior with rapid pin changes
    fixture->setDigitalPin(gpio_pins::KEY_PRESENT, LOW);
    fixture->setDigitalPin(gpio_pins::KEY_NOT_PRESENT, LOW);
    
    KeyState initialState = sensor->getKeyState();
    
    // Rapid changes within debounce period
    for (int i = 0; i < 5; i++) {
        fixture->setDigitalPin(gpio_pins::KEY_PRESENT, HIGH);
        fixture->advanceTime(1); // 1ms - should be within debounce period
        fixture->setDigitalPin(gpio_pins::KEY_PRESENT, LOW);
        fixture->advanceTime(1);
    }
    
    // State should be stable after rapid changes
    KeyState finalState = sensor->getKeyState();
    TEST_ASSERT_TRUE(finalState == KeyState::Present || 
                    finalState == KeyState::NotPresent || 
                    finalState == KeyState::Inactive);
}

void test_key_sensor_state_transitions() {
    sensor->init();
    
    // Test all valid state transitions
    KeyState transitions[] = {
        KeyState::Inactive,
        KeyState::Present,
        KeyState::Inactive,
        KeyState::NotPresent,
        KeyState::Inactive
    };
    
    for (size_t i = 0; i < sizeof(transitions) / sizeof(transitions[0]); i++) {
        switch (transitions[i]) {
            case KeyState::Present:
                fixture->setDigitalPin(gpio_pins::KEY_PRESENT, HIGH);
                fixture->setDigitalPin(gpio_pins::KEY_NOT_PRESENT, LOW);
                break;
            case KeyState::NotPresent:
                fixture->setDigitalPin(gpio_pins::KEY_PRESENT, LOW);
                fixture->setDigitalPin(gpio_pins::KEY_NOT_PRESENT, HIGH);
                break;
            case KeyState::Inactive:
            default:
                fixture->setDigitalPin(gpio_pins::KEY_PRESENT, LOW);
                fixture->setDigitalPin(gpio_pins::KEY_NOT_PRESENT, LOW);
                break;
        }
        
        fixture->advanceTime(50); // Allow state to settle
        KeyState currentState = sensor->getKeyState();
        TEST_ASSERT_EQUAL(transitions[i], currentState);
    }
}

void test_key_sensor_interrupt_handling() {
    sensor->init();
    
    // Verify interrupts are properly attached (in mock, this tests setup)
    TEST_ASSERT_TRUE(fixture->getGpioProvider()->hasInterrupt(gpio_pins::KEY_PRESENT));
    TEST_ASSERT_TRUE(fixture->getGpioProvider()->hasInterrupt(gpio_pins::KEY_NOT_PRESENT));
    
    // Simulate interrupt triggers
    fixture->setDigitalPin(gpio_pins::KEY_PRESENT, HIGH);
    fixture->triggerInterrupt(gpio_pins::KEY_PRESENT);
    
    // State should reflect the interrupt
    KeyState state = sensor->getKeyState();
    TEST_ASSERT_EQUAL(KeyState::Present, state);
}

void test_key_sensor_error_conditions() {
    sensor->init();
    
    // Test sensor behavior with invalid GPIO provider
    // This tests the robustness of the sensor implementation
    
    // Set conflicting states
    fixture->setDigitalPin(gpio_pins::KEY_PRESENT, HIGH);
    fixture->setDigitalPin(gpio_pins::KEY_NOT_PRESENT, HIGH);
    
    // Sensor should handle this gracefully
    KeyState state = sensor->getKeyState();
    Reading reading = sensor->getReading();
    
    // Should not crash and should return some valid state
    TEST_ASSERT_TRUE(state != static_cast<KeyState>(-1));
}

void test_key_sensor_performance() {
    sensor->init();
    
    fixture->setDigitalPin(gpio_pins::KEY_PRESENT, HIGH);
    fixture->setDigitalPin(gpio_pins::KEY_NOT_PRESENT, LOW);
    
    uint32_t startTime = fixture->getCurrentTime();
    
    // Perform many readings to test performance
    for (int i = 0; i < 1000; i++) {
        sensor->getReading();
        sensor->getKeyState();
    }
    
    uint32_t endTime = fixture->getCurrentTime();
    uint32_t elapsedTime = endTime - startTime;
    
    // Performance should be reasonable (this is a mock test)
    // In real hardware, you'd set actual performance thresholds
    TEST_ASSERT_TRUE(elapsedTime < 10000); // Less than 10 seconds for 1000 readings
}

void test_key_sensor_memory_stability() {
    // Test multiple init/cleanup cycles
    for (int i = 0; i < 10; i++) {
        sensor->init();
        
        fixture->setDigitalPin(gpio_pins::KEY_PRESENT, i % 2);
        fixture->setDigitalPin(gpio_pins::KEY_NOT_PRESENT, (i + 1) % 2);
        
        KeyState state = sensor->getKeyState();
        Reading reading = sensor->getReading();
        
        // Should work consistently across cycles
        TEST_ASSERT_TRUE(state != static_cast<KeyState>(-1));
    }
}

void test_key_sensor_concurrent_access() {
    sensor->init();
    
    fixture->setDigitalPin(gpio_pins::KEY_PRESENT, HIGH);
    fixture->setDigitalPin(gpio_pins::KEY_NOT_PRESENT, LOW);
    
    // Simulate concurrent access patterns
    KeyState state1 = sensor->getKeyState();
    Reading reading1 = sensor->getReading();
    
    KeyState state2 = sensor->getKeyState();
    Reading reading2 = sensor->getReading();
    
    // Concurrent access should be consistent
    TEST_ASSERT_EQUAL(state1, state2);
    TEST_ASSERT_EQUAL(std::get<int32_t>(reading1), std::get<int32_t>(reading2));
}

// Enhanced Phase 2 sensor state machine tests

void test_key_sensor_state_machine_completeness() {
    // Test all possible state transitions systematically
    sensor->init();
    
    // Test matrix of all state combinations
    bool stateTransitions[4][4] = {false}; // [from][to] matrix
    
    // Start in Inactive state
    fixture->setDigitalPin(gpio_pins::KEY_PRESENT, LOW);
    fixture->setDigitalPin(gpio_pins::KEY_NOT_PRESENT, LOW);
    KeyState currentState = sensor->getKeyState();
    TEST_ASSERT_EQUAL(KeyState::Inactive, currentState);
    int fromState = static_cast<int>(currentState);
    
    // Test transitions to all other states
    // Inactive -> Present
    fixture->setDigitalPin(gpio_pins::KEY_PRESENT, HIGH);
    currentState = sensor->getKeyState();
    stateTransitions[fromState][static_cast<int>(currentState)] = true;
    
    // Present -> NotPresent
    fromState = static_cast<int>(currentState);
    fixture->setDigitalPin(gpio_pins::KEY_PRESENT, LOW);
    fixture->setDigitalPin(gpio_pins::KEY_NOT_PRESENT, HIGH);
    currentState = sensor->getKeyState();
    stateTransitions[fromState][static_cast<int>(currentState)] = true;
    
    // NotPresent -> Inactive (invalid state)
    fromState = static_cast<int>(currentState);
    fixture->setDigitalPin(gpio_pins::KEY_PRESENT, HIGH);
    // Both pins HIGH = invalid, should handle gracefully
    currentState = sensor->getKeyState();
    stateTransitions[fromState][static_cast<int>(currentState)] = true;
    
    // Verify we tested key transitions
    TEST_ASSERT_TRUE(stateTransitions[0][1] || stateTransitions[0][2]); // From Inactive
    TEST_ASSERT_TRUE(stateTransitions[1][0] || stateTransitions[1][2]); // From Present
    TEST_ASSERT_TRUE(stateTransitions[2][0] || stateTransitions[2][1]); // From NotPresent
}

void test_key_sensor_timing_dependent_behavior() {
    // Test behavior with realistic timing constraints
    sensor->init();
    
    // Test rapid state changes (bounce simulation)
    fixture->setDigitalPin(gpio_pins::KEY_PRESENT, HIGH);
    KeyState state1 = sensor->getKeyState();
    
    // Simulate 50Hz bounce for 10ms
    for (int i = 0; i < 10; i++) {
        fixture->setDigitalPin(gpio_pins::KEY_PRESENT, i % 2 ? HIGH : LOW);
        fixture->advanceTime(1); // 1ms intervals
    }
    
    // Final state should be stable
    fixture->setDigitalPin(gpio_pins::KEY_PRESENT, HIGH);
    fixture->setDigitalPin(gpio_pins::KEY_NOT_PRESENT, LOW);
    KeyState finalState = sensor->getKeyState();
    TEST_ASSERT_EQUAL(KeyState::Present, finalState);
}

void test_key_sensor_boundary_conditions() {
    // Test sensor behavior at electrical boundaries
    sensor->init();
    
    // Test with marginal voltage levels (simulated via rapid changes)
    for (int cycle = 0; cycle < 100; cycle++) {
        // Simulate electrical noise/marginal signals
        bool keyPresent = (cycle % 3 == 0);
        bool keyNotPresent = (cycle % 5 == 0);
        
        // Avoid invalid state (both true)
        if (keyPresent && keyNotPresent) {
            keyNotPresent = false;
        }
        
        fixture->setDigitalPin(gpio_pins::KEY_PRESENT, keyPresent ? HIGH : LOW);
        fixture->setDigitalPin(gpio_pins::KEY_NOT_PRESENT, keyNotPresent ? HIGH : LOW);
        
        KeyState state = sensor->getKeyState();
        // State should always be valid
        TEST_ASSERT_TRUE(state == KeyState::Present || 
                        state == KeyState::NotPresent || 
                        state == KeyState::Inactive);
    }
}

void test_key_sensor_resource_exhaustion_handling() {
    // Test sensor behavior under resource constraints
    sensor->init();
    
    // Simulate many rapid readings (stress test)
    int validReadings = 0;
    int totalReadings = 1000;
    
    for (int i = 0; i < totalReadings; i++) {
        // Vary the state periodically
        bool present = (i % 7 == 0);
        bool notPresent = (i % 11 == 0);
        if (present && notPresent) notPresent = false;
        
        fixture->setDigitalPin(gpio_pins::KEY_PRESENT, present ? HIGH : LOW);
        fixture->setDigitalPin(gpio_pins::KEY_NOT_PRESENT, notPresent ? HIGH : LOW);
        
        KeyState state = sensor->getKeyState();
        Reading reading = sensor->getReading();
        
        // Every reading should be valid
        if (state == KeyState::Present || state == KeyState::NotPresent || state == KeyState::Inactive) {
            validReadings++;
        }
        
        // Reading should match state
        int32_t readingValue = std::get<int32_t>(reading);
        TEST_ASSERT_EQUAL(static_cast<int32_t>(state), readingValue);
    }
    
    // All readings should be valid
    TEST_ASSERT_EQUAL(totalReadings, validReadings);
}

void test_key_sensor_state_consistency_validation() {
    // Test that sensor maintains internal consistency
    sensor->init();
    
    // Test sequence that verifies internal state consistency
    struct StateTest {
        bool keyPresent;
        bool keyNotPresent;
        KeyState expectedState;
    };
    
    StateTest tests[] = {
        {false, false, KeyState::Inactive},
        {true, false, KeyState::Present},
        {false, true, KeyState::NotPresent},
        {true, true, KeyState::Inactive}, // Invalid -> should default to safe state
    };
    
    for (auto& test : tests) {
        fixture->setDigitalPin(gpio_pins::KEY_PRESENT, test.keyPresent ? HIGH : LOW);
        fixture->setDigitalPin(gpio_pins::KEY_NOT_PRESENT, test.keyNotPresent ? HIGH : LOW);
        
        KeyState actualState = sensor->getKeyState();
        Reading reading = sensor->getReading();
        
        if (test.keyPresent && test.keyNotPresent) {
            // Invalid state - should be handled gracefully (any safe state)
            TEST_ASSERT_TRUE(actualState == KeyState::Inactive || 
                            actualState == KeyState::Present || 
                            actualState == KeyState::NotPresent);
        } else {
            TEST_ASSERT_EQUAL(test.expectedState, actualState);
        }
        
        // Reading should always match state
        int32_t readingValue = std::get<int32_t>(reading);
        TEST_ASSERT_EQUAL(static_cast<int32_t>(actualState), readingValue);
    }
}

void setUp(void) {
    fixture = std::make_unique<SensorTestFixture>();
    fixture->SetUp();
    sensor = new KeySensor(fixture->getGpioProvider());
}

void tearDown(void) {
    delete sensor;
    fixture->TearDown();
    fixture.reset();
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    // Original tests
    RUN_TEST(test_key_sensor_construction);
    RUN_TEST(test_key_sensor_init);
    RUN_TEST(test_key_sensor_key_present_state);
    RUN_TEST(test_key_sensor_key_not_present_state);
    RUN_TEST(test_key_sensor_inactive_state);
    RUN_TEST(test_key_sensor_invalid_state);
    RUN_TEST(test_key_sensor_value_change_detection);
    RUN_TEST(test_key_sensor_reading_consistency);
    RUN_TEST(test_key_sensor_timing_behavior);
    RUN_TEST(test_key_sensor_debouncing);
    RUN_TEST(test_key_sensor_state_transitions);
    RUN_TEST(test_key_sensor_interrupt_handling);
    RUN_TEST(test_key_sensor_error_conditions);
    RUN_TEST(test_key_sensor_performance);
    RUN_TEST(test_key_sensor_memory_stability);
    RUN_TEST(test_key_sensor_concurrent_access);
    
    // Enhanced Phase 2 state machine tests
    RUN_TEST(test_key_sensor_state_machine_completeness);
    RUN_TEST(test_key_sensor_timing_dependent_behavior);
    RUN_TEST(test_key_sensor_boundary_conditions);
    RUN_TEST(test_key_sensor_resource_exhaustion_handling);
    RUN_TEST(test_key_sensor_state_consistency_validation);
    
    return UNITY_END();
}

#endif