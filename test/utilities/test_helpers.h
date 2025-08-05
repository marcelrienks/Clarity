#pragma once

#include "unity.h"
#include "utilities/types.h"
#include "mocks/mock_gpio_provider.h"
#ifdef UNIT_TESTING
#include "arduino_mock.h"
#endif
#include <memory>
#include <chrono>
#include <thread>
#include <functional>

/**
 * @file test_helpers.h
 * @brief Common testing utilities and helper functions
 * 
 * @details Provides reusable testing utilities for sensor testing,
 * time simulation, and common test patterns used across all test files.
 */

namespace TestHelpers {

/**
 * @brief Create a configured mock GPIO provider for testing
 * @return Unique pointer to configured MockGpioProvider
 */
std::unique_ptr<MockGpioProvider> createMockGpioProvider();

/**
 * @brief Simulate time passage for time-based sensor testing
 * @param milliseconds Time to advance in milliseconds
 * @note Uses actual delay for realistic timing behavior
 */
void advanceTime(unsigned long milliseconds);

/**
 * @brief Wait for sensor update interval to ensure reading refresh
 * @param intervalMs Sensor update interval in milliseconds
 * @note Adds small buffer to ensure update occurs
 */
void waitForSensorUpdate(unsigned long intervalMs);

/**
 * @brief Assert that a Reading contains an int32_t value
 * @param reading Reading variant to check
 * @param expectedValue Expected int32_t value
 */
void assertReadingInt32(const Reading& reading, int32_t expectedValue);

/**
 * @brief Assert that a Reading contains a bool value
 * @param reading Reading variant to check  
 * @param expectedValue Expected bool value
 */
void assertReadingBool(const Reading& reading, bool expectedValue);

/**
 * @brief Assert that a Reading contains monostate (uninitialized)
 * @param reading Reading variant to check
 */
void assertReadingMonostate(const Reading& reading);

/**
 * @brief Verify that analog reading is within ADC range (0-4095)
 * @param value Analog reading value to validate
 */
void assertValidAdcRange(uint16_t value);

/**
 * @brief Verify that pressure reading is within valid range (0-10 Bar)
 * @param pressure Pressure value to validate
 */
void assertValidPressureRange(int32_t pressure);

/**
 * @brief Verify that temperature reading is within valid range (0-120°C)
 * @param temperature Temperature value to validate
 */
void assertValidTemperatureRange(int32_t temperature);

/**
 * @brief Configure mock GPIO provider with oil pressure sensor values
 * @param mock Mock GPIO provider to configure
 * @param adcValue ADC value to return (0-4095)
 */
void configureMockForOilPressure(MockGpioProvider* mock, uint16_t adcValue);

/**
 * @brief Configure mock GPIO provider with oil temperature sensor values
 * @param mock Mock GPIO provider to configure
 * @param adcValue ADC value to return (0-4095)
 */
void configureMockForOilTemperature(MockGpioProvider* mock, uint16_t adcValue);

/**
 * @brief Configure mock GPIO provider for key sensor testing
 * @param mock Mock GPIO provider to configure
 * @param keyPresent Key present pin state
 * @param keyNotPresent Key not present pin state
 */
void configureMockForKeySensor(MockGpioProvider* mock, bool keyPresent, bool keyNotPresent);

/**
 * @brief Configure mock GPIO provider for lock sensor testing
 * @param mock Mock GPIO provider to configure
 * @param lockState Lock pin state
 */
void configureMockForLockSensor(MockGpioProvider* mock, bool lockState);

/**
 * @brief Configure mock GPIO provider for light sensor testing
 * @param mock Mock GPIO provider to configure
 * @param lightState Light pin state
 */
void configureMockForLightSensor(MockGpioProvider* mock, bool lightState);

/**
 * @brief Calculate expected pressure from ADC value using sensor formula
 * @param adcValue ADC reading (0-4095)
 * @return Expected pressure in Bar (0-10)
 */
int32_t calculateExpectedPressure(uint16_t adcValue);

/**
 * @brief Calculate expected temperature from ADC value using sensor formula
 * @param adcValue ADC reading (0-4095)
 * @return Expected temperature in Celsius (0-120)
 */
int32_t calculateExpectedTemperature(uint16_t adcValue);

/**
 * @brief Test pattern: Initialize sensor and verify no errors
 * @param sensor Sensor to initialize
 * @param mock Mock GPIO provider used by sensor
 */
template<typename TSensor>
void testSensorInitialization(TSensor& sensor, MockGpioProvider* mock);

/**
 * @brief Test pattern: Verify sensor reading changes only when value changes
 * @param sensor Sensor to test
 * @param mock Mock GPIO provider
 * @param getValue Function to get current sensor value
 * @param setValue Function to set mock value
 */
template<typename TSensor, typename TValue>
void testDeltaBasedUpdates(TSensor& sensor, MockGpioProvider* mock,
                          std::function<TValue()> getValue,
                          std::function<void(TValue)> setValue);

} // namespace TestHelpers