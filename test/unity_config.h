#pragma once

#include <stdio.h>
#include <stdlib.h>

// Unity Framework Configuration for Clarity Testing
// This file configures Unity testing framework for ESP32 and native environments

// Enable floating point support for sensor value testing
#ifndef UNITY_INCLUDE_DOUBLE
#define UNITY_INCLUDE_DOUBLE
#endif
#define UNITY_DOUBLE_PRECISION 1e-12

// Enable 64-bit integer support for timestamp testing
#define UNITY_INCLUDE_64

// Configure test output format
#define UNITY_OUTPUT_CHAR(a) putchar(a)
#define UNITY_OUTPUT_FLUSH() fflush(stdout)

// Test timeout configuration (prevent infinite loops)
#define UNITY_OUTPUT_START() printf("Starting Unity Tests...\n")
#define UNITY_OUTPUT_COMPLETE() printf("Unity Tests Complete.\n")

// Custom assertion macros for sensor-specific testing
#define TEST_ASSERT_READING_INT32(expected, actual) \
    TEST_ASSERT_EQUAL_INT32(std::get<int32_t>(expected), std::get<int32_t>(actual))

#define TEST_ASSERT_READING_BOOL(expected, actual) \
    TEST_ASSERT_EQUAL(std::get<bool>(expected), std::get<bool>(actual))

#define TEST_ASSERT_READING_MONOSTATE(reading) \
    TEST_ASSERT_TRUE(std::holds_alternative<std::monostate>(reading))

// Memory allocation tracking macros (for embedded testing)
#ifdef ESP32
    #define TEST_ASSERT_NO_MEMORY_LEAK() \
        TEST_ASSERT_GREATER_THAN(0, ESP.getFreeHeap())
#else
    #define TEST_ASSERT_NO_MEMORY_LEAK() // No-op for native testing
#endif

// GPIO pin validation macros
#define TEST_ASSERT_VALID_PIN(pin) \
    TEST_ASSERT_TRUE((pin >= 0) && (pin <= 39))

#define TEST_ASSERT_ADC_RANGE(value) \
    TEST_ASSERT_TRUE((value >= 0) && (value <= 4095))

#define TEST_ASSERT_PRESSURE_RANGE(value) \
    TEST_ASSERT_TRUE((value >= 0) && (value <= 10))

#define TEST_ASSERT_TEMPERATURE_RANGE(value) \
    TEST_ASSERT_TRUE((value >= 0) && (value <= 120))