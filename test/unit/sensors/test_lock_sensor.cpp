#include <unity.h>
#include "sensors/lock_sensor.h"
#include "mock_gpio_provider.h"
#include "hardware/gpio_pins.h"
#include "utilities/types.h"

MockGpioProvider* lockMockGpio;
LockSensor* lockSensor;

void setUp_lock_sensor() {
    lockMockGpio = new MockGpioProvider();
    lockSensor = new LockSensor(lockMockGpio);
}

void tearDown_lock_sensor() {
    delete lockSensor;
    delete lockMockGpio;
}

void test_lock_sensor_init() {
    lockSensor->init();
    
    // Verify GPIO pin is configured correctly
    TEST_ASSERT_EQUAL(INPUT_PULLDOWN, lockMockGpio->getPinMode(gpio_pins::LOCK));
}

void test_lock_sensor_locked_state() {
    lockSensor->init();
    
    // Set lock pin HIGH (locked)
    lockMockGpio->setDigitalValue(gpio_pins::LOCK, true);
    
    Reading reading = lockSensor->getReading();
    TEST_ASSERT_TRUE(std::get<bool>(reading));
}

void test_lock_sensor_unlocked_state() {
    lockSensor->init();
    
    // Set lock pin LOW (unlocked)
    lockMockGpio->setDigitalValue(gpio_pins::LOCK, false);
    
    Reading reading = lockSensor->getReading();
    TEST_ASSERT_FALSE(std::get<bool>(reading));
}

void test_lock_sensor_value_change_detection() {
    lockSensor->init();
    
    // Set initial state (unlocked)
    lockMockGpio->setDigitalValue(gpio_pins::LOCK, false);
    
    Reading reading1 = lockSensor->getReading();
    TEST_ASSERT_FALSE(std::get<bool>(reading1));
    
    // Same state should give same reading
    Reading reading2 = lockSensor->getReading();
    TEST_ASSERT_EQUAL(std::get<bool>(reading1), std::get<bool>(reading2));
    
    // Change to locked state should give different reading
    lockMockGpio->setDigitalValue(gpio_pins::LOCK, true);
    Reading reading3 = lockSensor->getReading();
    TEST_ASSERT_TRUE(std::get<bool>(reading3));
    TEST_ASSERT_NOT_EQUAL(std::get<bool>(reading1), std::get<bool>(reading3));
}

void test_lock_sensor_construction() {
    // Test that lockSensor can be created and destroyed
    TEST_ASSERT_NOT_NULL(lockSensor);
}

void test_lock_sensor_reading_consistency() {
    lockSensor->init();
    
    // Set a known state
    lockMockGpio->setDigitalValue(gpio_pins::LOCK, true);
    
    // Multiple readings should be consistent
    Reading reading1 = lockSensor->getReading();
    Reading reading2 = lockSensor->getReading();
    
    TEST_ASSERT_EQUAL(std::get<bool>(reading1), std::get<bool>(reading2));
}

void test_lock_sensor_state_transitions() {
    lockSensor->init();
    
    // Test unlocked to locked transition
    lockMockGpio->setDigitalValue(gpio_pins::LOCK, false);
    Reading unlockedReading = lockSensor->getReading();
    TEST_ASSERT_FALSE(std::get<bool>(unlockedReading));
    
    lockMockGpio->setDigitalValue(gpio_pins::LOCK, true);
    Reading lockedReading = lockSensor->getReading();
    TEST_ASSERT_TRUE(std::get<bool>(lockedReading));
    
    // States should be different
    TEST_ASSERT_NOT_EQUAL(std::get<bool>(unlockedReading), std::get<bool>(lockedReading));
}

void test_lock_sensor_boolean_logic() {
    lockSensor->init();
    
    // Test that readings properly correspond to GPIO states
    lockMockGpio->setDigitalValue(gpio_pins::LOCK, false);
    Reading falseReading = lockSensor->getReading();
    TEST_ASSERT_FALSE(std::get<bool>(falseReading));
    
    lockMockGpio->setDigitalValue(gpio_pins::LOCK, true);
    Reading trueReading = lockSensor->getReading();
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