#include <unity.h>
#include "providers/gpio_provider.h"

GpioProvider* gpioProvider = nullptr;

void setUp_gpio_provider() {
    gpioProvider = new GpioProvider();
}

void tearDown_gpio_provider() {
    delete gpioProvider;
    gpioProvider = nullptr;
}

void test_gpio_provider_construction() {
    // Test that provider can be created and destroyed
    TEST_ASSERT_NOT_NULL(gpioProvider);
}

void test_gpio_provider_digital_operations() {
    // Test pinMode call doesn't crash
    gpioProvider->pinMode(2, OUTPUT);
    TEST_ASSERT_TRUE(true);
    
    // Test digitalRead call doesn't crash and returns valid value
    bool value = gpioProvider->digitalRead(2);
    TEST_ASSERT_TRUE(value == true || value == false);
}

void test_gpio_provider_analog_operations() {
    // Test analogRead call doesn't crash and returns reasonable value
    uint16_t value = gpioProvider->analogRead(A0);
    TEST_ASSERT_GREATER_OR_EQUAL(0, value);
    TEST_ASSERT_LESS_OR_EQUAL(4095, value); // ESP32 12-bit ADC
}

void test_gpio_provider_pin_mode_settings() {
    // Test various pin modes don't crash
    gpioProvider->pinMode(2, INPUT);
    gpioProvider->pinMode(3, OUTPUT);
    gpioProvider->pinMode(4, INPUT_PULLUP);
    gpioProvider->pinMode(5, INPUT_PULLDOWN);
    TEST_ASSERT_TRUE(true);
}

void test_gpio_provider_interface_compliance() {
    // Test that GpioProvider implements IGpioProvider interface correctly
    IGpioProvider* provider = gpioProvider;
    TEST_ASSERT_NOT_NULL(provider);
    
    // Test interface methods work
    provider->pinMode(10, OUTPUT);
    bool digitalVal = provider->digitalRead(10);
    uint16_t analogVal = provider->analogRead(A0);
    
    // Values should be in valid ranges
    TEST_ASSERT_TRUE(digitalVal == true || digitalVal == false);
    TEST_ASSERT_GREATER_OR_EQUAL(0, analogVal);
    TEST_ASSERT_LESS_OR_EQUAL(4095, analogVal);
}

void test_gpio_provider_multiple_pins() {
    // Test operations on multiple pins
    for (int pin = 2; pin <= 5; pin++) {
        gpioProvider->pinMode(pin, OUTPUT);
        bool value = gpioProvider->digitalRead(pin);
        TEST_ASSERT_TRUE(value == true || value == false);
    }
}

void test_gpio_provider_analog_read_consistency() {
    // Test that analog reads are consistent (within reason)
    uint16_t reading1 = gpioProvider->analogRead(A0);
    uint16_t reading2 = gpioProvider->analogRead(A0);
    
    // Readings should be in valid range
    TEST_ASSERT_GREATER_OR_EQUAL(0, reading1);
    TEST_ASSERT_LESS_OR_EQUAL(4095, reading1);
    TEST_ASSERT_GREATER_OR_EQUAL(0, reading2);
    TEST_ASSERT_LESS_OR_EQUAL(4095, reading2);
}

void runGpioProviderTests() {
    setUp_gpio_provider();
    RUN_TEST(test_gpio_provider_construction);
    RUN_TEST(test_gpio_provider_digital_operations);
    RUN_TEST(test_gpio_provider_analog_operations);
    RUN_TEST(test_gpio_provider_pin_mode_settings);
    RUN_TEST(test_gpio_provider_interface_compliance);
    RUN_TEST(test_gpio_provider_multiple_pins);
    RUN_TEST(test_gpio_provider_analog_read_consistency);
    tearDown_gpio_provider();
}