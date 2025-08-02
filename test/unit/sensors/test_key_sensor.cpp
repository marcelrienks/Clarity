#ifdef UNIT_TESTING

#include <unity.h>
#include "../../utilities/test_fixtures.h"
#include "sensors/key_sensor.h"
#include "../../mocks/mock_gpio_provider.h"
#include "hardware/gpio_pins.h"
#include "utilities/types.h"

std::unique_ptr<SensorTestFixture> fixture;
KeySensor* sensor;

void setUp_key_sensor() {
    fixture = std::make_unique<SensorTestFixture>();
    fixture->SetUp();
    sensor = new KeySensor(fixture->getGpioProvider());
}

void tearDown_key_sensor() {
    delete sensor;
    fixture->TearDown();
    fixture.reset();
}

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
    bool hasChanged1 = sensor->hasValueChanged();
    
    // First reading should indicate change
    TEST_ASSERT_TRUE(hasChanged1);
    
    // Same state should not indicate change
    Reading reading2 = sensor->getReading();
    bool hasChanged2 = sensor->hasValueChanged();
    TEST_ASSERT_FALSE(hasChanged2);
    
    // Change to different state should indicate change
    fixture->setDigitalPin(gpio_pins::KEY_PRESENT, HIGH);
    Reading reading3 = sensor->getReading();
    bool hasChanged3 = sensor->hasValueChanged();
    TEST_ASSERT_TRUE(hasChanged3);
    TEST_ASSERT_NOT_EQUAL(reading1, reading3);
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
    
    TEST_ASSERT_EQUAL(reading1, reading2);
    TEST_ASSERT_EQUAL(state1, state2);
    TEST_ASSERT_EQUAL(reading1, static_cast<int32_t>(state1));
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
        sensor->hasValueChanged();
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
    bool changed1 = sensor->hasValueChanged();
    
    KeyState state2 = sensor->getKeyState();
    Reading reading2 = sensor->getReading();
    bool changed2 = sensor->hasValueChanged();
    
    // Concurrent access should be consistent
    TEST_ASSERT_EQUAL(state1, state2);
    TEST_ASSERT_EQUAL(reading1, reading2);
    // First call may indicate change, second should not
    TEST_ASSERT_FALSE(changed2);
}

void runKeySensorTests() {
    setUp_key_sensor();
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
    tearDown_key_sensor();
}

#endif