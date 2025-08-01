#include <unity.h>
#include "sensors/lock_sensor.h"
#include "../mocks/mock_gpio_provider.h"
#include "hardware/gpio_pins.h"
#include "utilities/types.h"

MockGpioProvider* mockGpio;
LockSensor* sensor;

void setUp_lock_sensor() {
    mockGpio = new MockGpioProvider();
    sensor = new LockSensor(mockGpio);
}

void tearDown_lock_sensor() {
    delete sensor;
    delete mockGpio;
}

void test_lock_sensor_init() {
    sensor->init();
    
    // Verify GPIO pin is configured correctly
    TEST_ASSERT_EQUAL(INPUT_PULLDOWN, mockGpio->getPinMode(gpio_pins::LOCK));
}

void test_lock_sensor_locked_state() {
    sensor->init();
    
    // Set lock pin HIGH (locked)
    mockGpio->setDigitalValue(gpio_pins::LOCK, true);
    
    Reading reading = sensor->getReading();
    TEST_ASSERT_TRUE(std::get<bool>(reading));
}

void test_lock_sensor_unlocked_state() {
    sensor->init();
    
    // Set lock pin LOW (unlocked)
    mockGpio->setDigitalValue(gpio_pins::LOCK, false);
    
    Reading reading = sensor->getReading();
    TEST_ASSERT_FALSE(std::get<bool>(reading));
}

void test_lock_sensor_value_change_detection() {
    sensor->init();
    
    // Set initial state (unlocked)
    mockGpio->setDigitalValue(gpio_pins::LOCK, false);
    
    Reading reading1 = sensor->getReading();
    bool hasChanged1 = sensor->hasValueChanged();
    
    // First reading should indicate change
    TEST_ASSERT_TRUE(hasChanged1);
    
    // Same state should not indicate change
    Reading reading2 = sensor->getReading();
    bool hasChanged2 = sensor->hasValueChanged();
    TEST_ASSERT_FALSE(hasChanged2);
    
    // Change to locked state should indicate change
    mockGpio->setDigitalValue(gpio_pins::LOCK, true);
    Reading reading3 = sensor->getReading();
    bool hasChanged3 = sensor->hasValueChanged();
    TEST_ASSERT_TRUE(hasChanged3);
    TEST_ASSERT_NOT_EQUAL(std::get<bool>(reading1), std::get<bool>(reading3));
}

void test_lock_sensor_construction() {
    // Test that sensor can be created and destroyed
    TEST_ASSERT_NOT_NULL(sensor);
}

void test_lock_sensor_reading_consistency() {
    sensor->init();
    
    // Set a known state
    mockGpio->setDigitalValue(gpio_pins::LOCK, true);
    
    // Multiple readings should be consistent
    Reading reading1 = sensor->getReading();
    Reading reading2 = sensor->getReading();
    
    TEST_ASSERT_EQUAL(std::get<bool>(reading1), std::get<bool>(reading2));
}

void test_lock_sensor_state_transitions() {
    sensor->init();
    
    // Test unlocked to locked transition
    mockGpio->setDigitalValue(gpio_pins::LOCK, false);
    Reading unlockedReading = sensor->getReading();
    TEST_ASSERT_FALSE(std::get<bool>(unlockedReading));
    
    mockGpio->setDigitalValue(gpio_pins::LOCK, true);
    Reading lockedReading = sensor->getReading();
    TEST_ASSERT_TRUE(std::get<bool>(lockedReading));
    
    // States should be different
    TEST_ASSERT_NOT_EQUAL(std::get<bool>(unlockedReading), std::get<bool>(lockedReading));
}

void test_lock_sensor_boolean_logic() {
    sensor->init();
    
    // Test that readings properly correspond to GPIO states
    mockGpio->setDigitalValue(gpio_pins::LOCK, false);
    Reading falseReading = sensor->getReading();
    TEST_ASSERT_FALSE(std::get<bool>(falseReading));
    
    mockGpio->setDigitalValue(gpio_pins::LOCK, true);
    Reading trueReading = sensor->getReading();
    TEST_ASSERT_TRUE(std::get<bool>(trueReading));
}

void runLockSensorTests() {
    setUp_lock_sensor();
    RUN_TEST(test_lock_sensor_construction);
    RUN_TEST(test_lock_sensor_init);
    RUN_TEST(test_lock_sensor_locked_state);
    RUN_TEST(test_lock_sensor_unlocked_state);
    RUN_TEST(test_lock_sensor_value_change_detection);
    RUN_TEST(test_lock_sensor_reading_consistency);
    RUN_TEST(test_lock_sensor_state_transitions);
    RUN_TEST(test_lock_sensor_boolean_logic);
    tearDown_lock_sensor();
}