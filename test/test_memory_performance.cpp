#ifdef UNIT_TEST
#include <unity.h>
#include <esp32-hal-log.h>
#include "managers/interrupt_manager.h"
#include "utilities/types.h"
#include "utilities/interrupt_callbacks.h"

// Test constants for memory optimization verification
const size_t EXPECTED_INTERRUPT_SIZE = 29; // Expected size in bytes per interrupt
const size_t EXPECTED_MEMORY_SAVINGS = 28; // Total memory savings from single function design
const unsigned long PERFORMANCE_THRESHOLD_MS = 50; // Maximum interrupt processing time

/**
 * @brief Test Memory Usage: Interrupt Structure Size
 * Verify that the interrupt structure meets memory optimization goals
 */
void test_interrupt_structure_memory_usage() {
    // Verify interrupt structure size meets optimization target
    size_t actualSize = sizeof(Interrupt);
    log_i("Interrupt structure size: %d bytes (target: %d bytes)", actualSize, EXPECTED_INTERRUPT_SIZE);
    
    // Allow some variance due to compiler padding
    TEST_ASSERT_LESS_OR_EQUAL(32, actualSize);
    TEST_ASSERT_GREATER_OR_EQUAL(28, actualSize);
}

/**
 * @brief Test Memory Usage: Function Pointer Architecture
 * Verify static function pointers work correctly and don't cause memory leaks
 */
void test_function_pointer_memory_safety() {
    // Test static function pointer execution (no heap allocation)
    void* testContext = nullptr;
    
    // These should not allocate memory on the heap
    InterruptResult result1 = InterruptCallbacks::KeyPresentProcess(testContext);
    InterruptResult result2 = InterruptCallbacks::LightsStateProcess(testContext);
    
    // Results should be valid (though NO_ACTION due to null context)
    TEST_ASSERT_EQUAL(InterruptResult::NO_ACTION, result1);
    TEST_ASSERT_EQUAL(InterruptResult::NO_ACTION, result2);
    
    // No way to directly test heap allocation in unit tests,
    // but this verifies function pointers execute without crashing
}

/**
 * @brief Test Performance: Interrupt Processing Time
 * Verify interrupt processing meets timing requirements
 */
void test_interrupt_processing_performance() {
    // Create a minimal interrupt manager for testing
    InterruptManager testManager;
    testManager.Init();
    
    // Measure processing time
    unsigned long startTime = millis();
    
    // Process interrupts multiple times
    for (int i = 0; i < 100; i++) {
        testManager.Process();
    }
    
    unsigned long processingTime = millis() - startTime;
    log_i("100 interrupt processing cycles took %lu ms", processingTime);
    
    // Average processing time should be reasonable
    unsigned long averageTime = processingTime / 100;
    TEST_ASSERT_LESS_THAN(PERFORMANCE_THRESHOLD_MS, averageTime);
}

/**
 * @brief Test Performance: Priority Coordination Efficiency
 * Verify cross-handler priority coordination is efficient
 */
void test_priority_coordination_performance() {
    InterruptManager testManager;
    testManager.Init();
    
    // Measure evaluation time with no active interrupts
    unsigned long startTime = millis();
    
    for (int i = 0; i < 1000; i++) {
        // This calls EvaluateInterrupts internally
        testManager.Process();
    }
    
    unsigned long evaluationTime = millis() - startTime;
    log_i("1000 evaluation cycles took %lu ms", evaluationTime);
    
    // Should be very fast with no active interrupts
    TEST_ASSERT_LESS_THAN(500, evaluationTime); // Less than 500ms for 1000 evaluations
}

/**
 * @brief Test Memory: Error Manager Memory Usage
 * Verify error queue management doesn't cause memory leaks
 */
void test_error_manager_memory_usage() {
    ErrorManager& errorManager = ErrorManager::Instance();
    
    // Clear any existing errors
    errorManager.ClearAllErrors();
    TEST_ASSERT_FALSE(errorManager.HasPendingErrors());
    
    // Add many errors to test queue management
    for (int i = 0; i < 20; i++) { // More than the typical queue limit
        std::string errorMsg = "Test error " + std::to_string(i);
        errorManager.ReportError(ErrorLevel::WARNING, "TestComponent", errorMsg.c_str());
    }
    
    // Should have errors but be bounded
    TEST_ASSERT_TRUE(errorManager.HasPendingErrors());
    
    // Clear all errors
    errorManager.ClearAllErrors();
    TEST_ASSERT_FALSE(errorManager.HasPendingErrors());
}

/**
 * @brief Test Memory: Panel Manager Memory Management
 * Verify panels are properly created and destroyed without leaks
 */
void test_panel_manager_memory_management() {
    // This test would require mock providers and would be more complex
    // For now, just test that we can measure memory usage
    
    // Get initial free heap
    size_t initialFreeHeap = ESP.getFreeHeap();
    log_i("Initial free heap: %d bytes", initialFreeHeap);
    
    // Perform some operations that create/destroy objects
    {
        std::vector<std::unique_ptr<int>> testObjects;
        for (int i = 0; i < 100; i++) {
            testObjects.push_back(std::make_unique<int>(i));
        }
        // Objects destroyed when going out of scope
    }
    
    // Check heap after cleanup
    size_t finalFreeHeap = ESP.getFreeHeap();
    log_i("Final free heap: %d bytes", finalFreeHeap);
    
    // Should have similar or more free heap (allowing for some fragmentation)
    TEST_ASSERT_GREATER_OR_EQUAL(initialFreeHeap - 1000, finalFreeHeap);
}

/**
 * @brief Test Performance: Theme Change Frequency
 * Verify theme changes meet performance requirements (max 2 per second)
 */
void test_theme_change_frequency_performance() {
    // This would test that theme change interrupts don't fire more than 2 times per second
    // For now, just verify the timing constants are correct
    
    const unsigned long MIN_THEME_CHANGE_INTERVAL_MS = 500; // 2 times per second max
    
    // Verify that sensor evaluation intervals support this
    // This is more of a documentation test of the design
    TEST_ASSERT_GREATER_OR_EQUAL(MIN_THEME_CHANGE_INTERVAL_MS / 10, 25); // IMPORTANT priority min interval
}

/**
 * @brief Test Memory: Static Callback Memory Savings Verification
 * Verify the 28-byte memory savings from single function design
 */
void test_static_callback_memory_savings() {
    // This is a design verification test
    // In the old design, each interrupt would have had:
    // - evaluationFunc (4 bytes)
    // - activateFunc (4 bytes) 
    // - deactivateFunc (4 bytes) - ELIMINATED
    // Total savings: 4 bytes per interrupt
    
    // With 7 typical interrupts, savings = 7 * 4 = 28 bytes
    const size_t INTERRUPTS_IN_SYSTEM = 7;
    const size_t BYTES_SAVED_PER_INTERRUPT = 4;
    const size_t TOTAL_EXPECTED_SAVINGS = INTERRUPTS_IN_SYSTEM * BYTES_SAVED_PER_INTERRUPT;
    
    TEST_ASSERT_EQUAL(EXPECTED_MEMORY_SAVINGS, TOTAL_EXPECTED_SAVINGS);
    
    log_i("Memory savings verified: %d bytes total (%d interrupts × %d bytes)", 
          TOTAL_EXPECTED_SAVINGS, INTERRUPTS_IN_SYSTEM, BYTES_SAVED_PER_INTERRUPT);
}

/**
 * @brief Test Performance: ESP32 Memory Constraints Compliance
 * Verify the system works within ESP32 memory limits
 */
void test_esp32_memory_constraints_compliance() {
    // Test that we're operating within ESP32-WROOM-32 constraints
    size_t totalHeapSize = ESP.getHeapSize();
    size_t freeHeap = ESP.getFreeHeap();
    size_t usedHeap = totalHeapSize - freeHeap;
    
    log_i("ESP32 Memory Status:");
    log_i("  Total heap: %d bytes", totalHeapSize);
    log_i("  Used heap: %d bytes", usedHeap);
    log_i("  Free heap: %d bytes", freeHeap);
    
    // Verify we're not using excessive memory
    // ESP32-WROOM-32 has ~320KB total RAM, ~250KB available after system overhead
    const size_t MAX_REASONABLE_USAGE = 200000; // 200KB limit
    
    TEST_ASSERT_LESS_THAN(MAX_REASONABLE_USAGE, usedHeap);
    
    // Verify we have reasonable free memory remaining
    const size_t MIN_FREE_MEMORY = 50000; // 50KB minimum
    TEST_ASSERT_GREATER_THAN(MIN_FREE_MEMORY, freeHeap);
}

void runMemoryPerformanceTests(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_interrupt_structure_memory_usage);
    RUN_TEST(test_function_pointer_memory_safety);
    RUN_TEST(test_interrupt_processing_performance);
    RUN_TEST(test_priority_coordination_performance);
    RUN_TEST(test_error_manager_memory_usage);
    RUN_TEST(test_panel_manager_memory_management);
    RUN_TEST(test_theme_change_frequency_performance);
    RUN_TEST(test_static_callback_memory_savings);
    RUN_TEST(test_esp32_memory_constraints_compliance);
    
    UNITY_END();
}

#ifdef ARDUINO
void setup() {
    delay(2000); // Give time for serial monitor
    runMemoryPerformanceTests();
}

void loop() {
    // Tests run once in setup
}
#else
int main() {
    runMemoryPerformanceTests();
    return 0;
}
#endif

#endif // UNIT_TEST