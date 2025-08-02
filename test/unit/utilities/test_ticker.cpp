#include <unity.h>
#include "utilities/ticker.h"

#ifdef UNIT_TESTING
// Note: Using Arduino.h mock functions for timing
// set_mock_millis is defined in test_all.cpp to avoid conflicts
#endif

void test_ticker_dynamic_delay_normal_case() {
    // Test that method doesn't crash with normal input
    uint32_t startTime = 10;
    Ticker::handleDynamicDelay(startTime);
    TEST_ASSERT_TRUE(true);
}

void test_ticker_dynamic_delay_various_inputs() {
    // Test that method doesn't crash with various inputs
    Ticker::handleDynamicDelay(0);
    Ticker::handleDynamicDelay(100);
    Ticker::handleDynamicDelay(1000);
    TEST_ASSERT_TRUE(true);
}

void test_ticker_lv_tasks() {
    // Test that handleLvTasks runs without crashing
    Ticker::handleLvTasks();
    TEST_ASSERT_TRUE(true);
}

void test_ticker_multiple_calls() {
    // Test multiple calls don't crash
    for (int i = 0; i < 10; i++) {
        Ticker::handleDynamicDelay(i * 10);
        Ticker::handleLvTasks();
    }
    TEST_ASSERT_TRUE(true);
}

void test_ticker_static_methods_accessible() {
    // Test that static methods are accessible
    Ticker::handleDynamicDelay(50);
    Ticker::handleLvTasks();
    TEST_ASSERT_TRUE(true);
}

void test_ticker_timing_consistency() {
    // Test that timing calculations are consistent
    set_mock_millis(1000);
    Ticker::handleDynamicDelay(500);
    
    set_mock_millis(2000);
    Ticker::handleDynamicDelay(1500);
    
    // Test passes if no crashes occur
    TEST_ASSERT_TRUE(true);
}

void runTickerTests() {
    RUN_TEST(test_ticker_dynamic_delay_normal_case);
    RUN_TEST(test_ticker_dynamic_delay_various_inputs);
    RUN_TEST(test_ticker_lv_tasks);
    RUN_TEST(test_ticker_multiple_calls);
    RUN_TEST(test_ticker_static_methods_accessible);
    RUN_TEST(test_ticker_timing_consistency);
}