#include <unity.h>
#include "sensors/key_sensor.h"
#include "../mocks/mock_gpio_provider.h"
#include "hardware/gpio_pins.h"
#include "utilities/types.h"

MockGpioProvider* mockGpio;
KeySensor* sensor;

void setUp_key_sensor() {
    mockGpio = new MockGpioProvider();
    sensor = new KeySensor(mockGpio);
}

void tearDown_key_sensor() {
    delete sensor;
    delete mockGpio;
}

void test_key_sensor_init() {
    sensor->init();
    
    // Verify GPIO pins are configured correctly
    TEST_ASSERT_EQUAL(INPUT_PULLDOWN, mockGpio->getPinMode(gpio_pins::KEY_PRESENT));
    TEST_ASSERT_EQUAL(INPUT_PULLDOWN, mockGpio->getPinMode(gpio_pins::KEY_NOT_PRESENT));
}

void test_key_sensor_key_present_state() {
    sensor->init();
    
    // Set key present pin HIGH, key not present pin LOW
    mockGpio->setDigitalValue(gpio_pins::KEY_PRESENT, true);
    mockGpio->setDigitalValue(gpio_pins::KEY_NOT_PRESENT, false);
    
    KeyState state = sensor->getKeyState();
    TEST_ASSERT_EQUAL(KeyState::Present, state);
    
    Reading reading = sensor->getReading();
    TEST_ASSERT_EQUAL(static_cast<int32_t>(KeyState::Present), std::get<int32_t>(reading));
}

void test_key_sensor_key_not_present_state() {
    sensor->init();
    
    // Set key present pin LOW, key not present pin HIGH
    mockGpio->setDigitalValue(gpio_pins::KEY_PRESENT, false);
    mockGpio->setDigitalValue(gpio_pins::KEY_NOT_PRESENT, true);
    
    KeyState state = sensor->getKeyState();
    TEST_ASSERT_EQUAL(KeyState::NotPresent, state);
    
    Reading reading = sensor->getReading();
    TEST_ASSERT_EQUAL(static_cast<int32_t>(KeyState::NotPresent), std::get<int32_t>(reading));
}

void test_key_sensor_inactive_state() {
    sensor->init();
    
    // Set both pins LOW (inactive state)
    mockGpio->setDigitalValue(gpio_pins::KEY_PRESENT, false);
    mockGpio->setDigitalValue(gpio_pins::KEY_NOT_PRESENT, false);
    
    KeyState state = sensor->getKeyState();
    TEST_ASSERT_EQUAL(KeyState::Inactive, state);
    
    Reading reading = sensor->getReading();
    TEST_ASSERT_EQUAL(static_cast<int32_t>(KeyState::Inactive), std::get<int32_t>(reading));
}

void test_key_sensor_invalid_state() {
    sensor->init();
    
    // Set both pins HIGH (invalid state)
    mockGpio->setDigitalValue(gpio_pins::KEY_PRESENT, true);
    mockGpio->setDigitalValue(gpio_pins::KEY_NOT_PRESENT, true);
    
    KeyState state = sensor->getKeyState();
    // Should handle invalid state gracefully (implementation dependent)
    TEST_ASSERT_TRUE(state == KeyState::Present || 
                     state == KeyState::NotPresent || 
                     state == KeyState::Inactive);
}

void test_key_sensor_value_change_detection() {
    sensor->init();
    
    // Set initial state
    mockGpio->setDigitalValue(gpio_pins::KEY_PRESENT, LOW);
    mockGpio->setDigitalValue(gpio_pins::KEY_NOT_PRESENT, LOW);
    
    Reading reading1 = sensor->getReading();
    bool hasChanged1 = sensor->hasValueChanged();
    
    // First reading should indicate change
    TEST_ASSERT_TRUE(hasChanged1);
    
    // Same state should not indicate change
    Reading reading2 = sensor->getReading();
    bool hasChanged2 = sensor->hasValueChanged();
    TEST_ASSERT_FALSE(hasChanged2);
    
    // Change to different state should indicate change
    mockGpio->setDigitalValue(gpio_pins::KEY_PRESENT, HIGH);
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
    mockGpio->setDigitalValue(gpio_pins::KEY_PRESENT, HIGH);
    mockGpio->setDigitalValue(gpio_pins::KEY_NOT_PRESENT, LOW);
    
    // Multiple readings should be consistent
    Reading reading1 = sensor->getReading();
    Reading reading2 = sensor->getReading();
    KeyState state1 = sensor->getKeyState();
    KeyState state2 = sensor->getKeyState();
    
    TEST_ASSERT_EQUAL(reading1, reading2);
    TEST_ASSERT_EQUAL(state1, state2);
    TEST_ASSERT_EQUAL(reading1, static_cast<int32_t>(state1));
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
    tearDown_key_sensor();
}