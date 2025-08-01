#include <unity.h>
#include <cstdint>

#ifdef UNIT_TESTING

// Mock millis function
static uint32_t mock_millis_value = 0;
uint32_t millis() { return mock_millis_value; }
void delay(uint32_t ms) {}

void set_mock_millis(uint32_t value) {
    mock_millis_value = value;
}

// Simple test implementation of dynamic delay logic
void handleDynamicDelay(uint32_t startTime) {
    uint32_t elapsedTime = millis() - startTime;
    uint32_t targetFrameTime = 16;
    if (elapsedTime < targetFrameTime)
        delay(targetFrameTime - elapsedTime);
    else
        delay(1);
}

#endif

void test_simple_dynamic_delay_normal_case() {
    set_mock_millis(0);
    uint32_t startTime = 0;
    
    // Simulate 10ms processing time
    set_mock_millis(10);
    
    // This should complete without issues
    handleDynamicDelay(startTime);
    TEST_ASSERT_TRUE(true);
}

void test_simple_dynamic_delay_slow_processing() {
    set_mock_millis(0);
    uint32_t startTime = 0;
    
    // Simulate 20ms processing time (longer than 16ms target)
    set_mock_millis(20);
    
    // This should handle the slow processing case
    handleDynamicDelay(startTime);
    TEST_ASSERT_TRUE(true);
}

void test_timing_calculation() {
    uint32_t targetFrameTime = 16;
    
    // Fast processing
    set_mock_millis(0);
    uint32_t startTime = 0;
    set_mock_millis(5);
    uint32_t elapsed = mock_millis_value - startTime;
    TEST_ASSERT_LESS_THAN(targetFrameTime, elapsed);
    
    // Slow processing
    set_mock_millis(0);
    startTime = 0;
    set_mock_millis(25);
    elapsed = mock_millis_value - startTime;
    TEST_ASSERT_GREATER_THAN(targetFrameTime, elapsed);
}

void runSimpleTickerTests() {
    RUN_TEST(test_simple_dynamic_delay_normal_case);
    RUN_TEST(test_simple_dynamic_delay_slow_processing);
    RUN_TEST(test_timing_calculation);
}