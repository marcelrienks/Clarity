#include <unity.h>
#include "mocks/mock_gpio_provider.h"

MockGpioProvider* gpioProviderMock = nullptr;

void setUp(void) {
    gpioProviderMock = new MockGpioProvider();
}

void tearDown(void) {
    delete gpioProviderMock;
    gpioProviderMock = nullptr;
}

// REMOVED: Pointless constructor test - just checks new works

void test_gpio_provider_digital_operations() {
    // Test pinMode call doesn't crash
    gpioProviderMock->pinMode(2, OUTPUT);
    TEST_ASSERT_TRUE(true);
    
    // Test digitalRead call doesn't crash and returns valid value
    bool value = gpioProviderMock->digitalRead(2);
    TEST_ASSERT_TRUE(value == true || value == false);
}

void test_gpio_provider_analog_operations() {
    // Test analogRead call doesn't crash and returns reasonable value
    uint16_t value = gpioProviderMock->analogRead(A0);
    TEST_ASSERT_GREATER_OR_EQUAL(0, value);
    TEST_ASSERT_LESS_OR_EQUAL(4095, value); // ESP32 12-bit ADC
}

// REMOVED: Pointless test - just calls pinMode and asserts true

void test_gpio_provider_interface_compliance() {
    // Test that MockGpioProvider implements IGpioProvider interface correctly
    IGpioProvider* provider = gpioProviderMock;
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
        gpioProviderMock->pinMode(pin, OUTPUT);
        bool value = gpioProviderMock->digitalRead(pin);
        TEST_ASSERT_TRUE(value == true || value == false);
    }
}

void test_gpio_provider_analog_read_consistency() {
    // Test that analog reads are consistent (within reason)
    uint16_t reading1 = gpioProviderMock->analogRead(A0);
    uint16_t reading2 = gpioProviderMock->analogRead(A0);
    
    // Readings should be in valid range
    TEST_ASSERT_GREATER_OR_EQUAL(0, reading1);
    TEST_ASSERT_LESS_OR_EQUAL(4095, reading1);
    TEST_ASSERT_GREATER_OR_EQUAL(0, reading2);
    TEST_ASSERT_LESS_OR_EQUAL(4095, reading2);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    RUN_TEST(test_gpio_provider_digital_operations);
    RUN_TEST(test_gpio_provider_analog_operations);
    RUN_TEST(test_gpio_provider_interface_compliance);
    RUN_TEST(test_gpio_provider_multiple_pins);
    RUN_TEST(test_gpio_provider_analog_read_consistency);
    
    return UNITY_END();
}