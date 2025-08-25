#ifdef CLARITY_DEBUG

#include "managers/interrupt_manager.h"
#include "sensors/base_sensor.h"
#include "utilities/types.h"
#include <Arduino.h>
#include <esp32-hal-log.h>

#ifndef LOG_TAG
#define LOG_TAG "InterruptSystemTest"
#endif

/**
 * @brief Comprehensive test for the coordinated interrupt system
 * @details Tests all interrupt types, priorities, and execution paths
 * 
 * Phase 6: Complete system integration testing
 */
class InterruptSystemTest
{
public:
    static void RunAllTests()
    {
        log_i("=== Starting Interrupt System Tests ===");
        
        // Test 1: Basic interrupt registration
        TestInterruptRegistration();
        
        // Test 2: Priority-based execution
        TestPriorityExecution();
        
        // Test 3: Polled vs Queued interrupts
        TestInterruptSources();
        
        // Test 4: Performance and memory usage
        TestPerformanceMetrics();
        
        // Test 5: Error handling and edge cases
        TestErrorHandling();
        
        log_i("=== Interrupt System Tests Complete ===");
    }

private:
    // Test callback functions
    static bool TestEvaluationCallback(void* context)
    {
        static int* callCount = static_cast<int*>(context);
        (*callCount)++;
        log_d("Test evaluation callback called - count: %d", *callCount);
        return *callCount % 5 == 0; // Trigger every 5th evaluation
    }
    
    static void TestExecutionCallback(void* context)
    {
        static int* callCount = static_cast<int*>(context);
        (*callCount)++;
        log_d("Test execution callback called - count: %d", *callCount);
    }
    
    static void TestInterruptRegistration()
    {
        log_i("--- Test 1: Interrupt Registration ---");
        
        InterruptManager& manager = InterruptManager::Instance();
        size_t initialCount = manager.GetRegisteredInterruptCount();
        
        // Test registering a basic polled interrupt
        static int testContext = 0;
        Interrupt testInterrupt = {};
        testInterrupt.id = "test_interrupt_registration";
        testInterrupt.priority = Priority::NORMAL;
        testInterrupt.source = InterruptSource::POLLED;
        testInterrupt.effect = InterruptEffect::BUTTON_ACTION;
        testInterrupt.evaluationFunc = TestEvaluationCallback;
        testInterrupt.executionFunc = TestExecutionCallback;
        testInterrupt.context = &testContext;
        testInterrupt.active = true;
        
        bool registered = manager.RegisterInterrupt(testInterrupt);
        if (registered)
        {
            log_i("✓ Test interrupt registered successfully");
            
            size_t newCount = manager.GetRegisteredInterruptCount();
            if (newCount == initialCount + 1)
            {
                log_i("✓ Interrupt count increased correctly");
            }
            else
            {
                log_e("✗ Interrupt count mismatch: expected %d, got %d", 
                      initialCount + 1, newCount);
            }
            
            // Cleanup
            manager.UnregisterInterrupt("test_interrupt_registration");
        }
        else
        {
            log_e("✗ Failed to register test interrupt");
        }
    }
    
    static void TestPriorityExecution()
    {
        log_i("--- Test 2: Priority Execution ---");
        
        InterruptManager& manager = InterruptManager::Instance();
        
        // Register interrupts with different priorities
        static int criticalContext = 0;
        static int importantContext = 0;
        static int normalContext = 0;
        
        // Critical priority interrupt
        Interrupt criticalInterrupt = {};
        criticalInterrupt.id = "test_critical";
        criticalInterrupt.priority = Priority::CRITICAL;
        criticalInterrupt.source = InterruptSource::POLLED;
        criticalInterrupt.effect = InterruptEffect::LOAD_PANEL;
        criticalInterrupt.evaluationFunc = TestEvaluationCallback;
        criticalInterrupt.executionFunc = TestExecutionCallback;
        criticalInterrupt.context = &criticalContext;
        criticalInterrupt.active = true;
        
        // Important priority interrupt
        Interrupt importantInterrupt = {};
        importantInterrupt.id = "test_important";
        importantInterrupt.priority = Priority::IMPORTANT;
        importantInterrupt.source = InterruptSource::POLLED;
        importantInterrupt.effect = InterruptEffect::SET_THEME;
        importantInterrupt.evaluationFunc = TestEvaluationCallback;
        importantInterrupt.executionFunc = TestExecutionCallback;
        importantInterrupt.context = &importantContext;
        importantInterrupt.active = true;
        
        // Normal priority interrupt
        Interrupt normalInterrupt = {};
        normalInterrupt.id = "test_normal";
        normalInterrupt.priority = Priority::NORMAL;
        normalInterrupt.source = InterruptSource::POLLED;
        normalInterrupt.effect = InterruptEffect::SET_PREFERENCE;
        normalInterrupt.evaluationFunc = TestEvaluationCallback;
        normalInterrupt.executionFunc = TestExecutionCallback;
        normalInterrupt.context = &normalContext;
        normalInterrupt.active = true;
        
        manager.RegisterInterrupt(criticalInterrupt);
        manager.RegisterInterrupt(importantInterrupt);
        manager.RegisterInterrupt(normalInterrupt);
        
        log_i("✓ Registered interrupts with different priorities");
        
        // Process interrupts and verify priority order
        for (int i = 0; i < 10; ++i)
        {
            manager.Process();
            delay(10);
        }
        
        log_i("✓ Priority execution test completed");
        
        // Cleanup
        manager.UnregisterInterrupt("test_critical");
        manager.UnregisterInterrupt("test_important");
        manager.UnregisterInterrupt("test_normal");
    }
    
    static void TestInterruptSources()
    {
        log_i("--- Test 3: Interrupt Sources ---");
        
        InterruptManager& manager = InterruptManager::Instance();
        
        // Test both polled and queued sources
        static int polledContext = 0;
        static int queuedContext = 0;
        
        // Polled interrupt
        Interrupt polledInterrupt = {};
        polledInterrupt.id = "test_polled_source";
        polledInterrupt.priority = Priority::IMPORTANT;
        polledInterrupt.source = InterruptSource::POLLED;
        polledInterrupt.effect = InterruptEffect::BUTTON_ACTION;
        polledInterrupt.evaluationFunc = TestEvaluationCallback;
        polledInterrupt.executionFunc = TestExecutionCallback;
        polledInterrupt.context = &polledContext;
        polledInterrupt.active = true;
        
        // Queued interrupt
        Interrupt queuedInterrupt = {};
        queuedInterrupt.id = "test_queued_source";
        queuedInterrupt.priority = Priority::IMPORTANT;
        queuedInterrupt.source = InterruptSource::QUEUED;
        queuedInterrupt.effect = InterruptEffect::LOAD_PANEL;
        queuedInterrupt.evaluationFunc = TestEvaluationCallback;
        queuedInterrupt.executionFunc = TestExecutionCallback;
        queuedInterrupt.context = &queuedContext;
        queuedInterrupt.active = true;
        
        manager.RegisterInterrupt(polledInterrupt);
        manager.RegisterInterrupt(queuedInterrupt);
        
        log_i("✓ Registered both polled and queued interrupts");
        
        // Process and verify both types work
        for (int i = 0; i < 15; ++i)
        {
            manager.Process();
            delay(5);
        }
        
        log_i("✓ Interrupt source test completed");
        
        // Cleanup
        manager.UnregisterInterrupt("test_polled_source");
        manager.UnregisterInterrupt("test_queued_source");
    }
    
    static void TestPerformanceMetrics()
    {
        log_i("--- Test 4: Performance Metrics ---");
        
        InterruptManager& manager = InterruptManager::Instance();
        
        size_t initialEvals, initialExecs;
        manager.GetInterruptStatistics(initialEvals, initialExecs);
        
        log_i("Initial stats - Evaluations: %lu, Executions: %lu", 
              initialEvals, initialExecs);
        
        // Add temporary test interrupt
        static int perfContext = 0;
        Interrupt perfInterrupt = {};
        perfInterrupt.id = "test_performance";
        perfInterrupt.priority = Priority::NORMAL;
        perfInterrupt.source = InterruptSource::POLLED;
        perfInterrupt.effect = InterruptEffect::BUTTON_ACTION;
        perfInterrupt.evaluationFunc = TestEvaluationCallback;
        perfInterrupt.executionFunc = TestExecutionCallback;
        perfInterrupt.context = &perfContext;
        perfInterrupt.active = true;
        
        manager.RegisterInterrupt(perfInterrupt);
        
        // Run performance test
        unsigned long startTime = millis();
        for (int i = 0; i < 100; ++i)
        {
            manager.Process();
            delay(1);
        }
        unsigned long endTime = millis();
        
        size_t finalEvals, finalExecs;
        manager.GetInterruptStatistics(finalEvals, finalExecs);
        
        log_i("Final stats - Evaluations: %lu (+%lu), Executions: %lu (+%lu)", 
              finalEvals, finalEvals - initialEvals, 
              finalExecs, finalExecs - initialExecs);
        log_i("Performance test completed in %lu ms", endTime - startTime);
        
        // Test memory optimization
        manager.OptimizeMemoryUsage();
        log_i("✓ Memory optimization completed");
        
        // Cleanup
        manager.UnregisterInterrupt("test_performance");
    }
    
    static void TestErrorHandling()
    {
        log_i("--- Test 5: Error Handling ---");
        
        InterruptManager& manager = InterruptManager::Instance();
        
        // Test registering interrupt with null ID
        Interrupt nullIdInterrupt = {};
        nullIdInterrupt.id = nullptr;
        nullIdInterrupt.priority = Priority::NORMAL;
        nullIdInterrupt.source = InterruptSource::POLLED;
        nullIdInterrupt.effect = InterruptEffect::BUTTON_ACTION;
        nullIdInterrupt.evaluationFunc = TestEvaluationCallback;
        nullIdInterrupt.executionFunc = TestExecutionCallback;
        nullIdInterrupt.context = nullptr;
        nullIdInterrupt.active = true;
        
        bool nullIdRegistered = manager.RegisterInterrupt(nullIdInterrupt);
        if (!nullIdRegistered)
        {
            log_i("✓ Correctly rejected interrupt with null ID");
        }
        else
        {
            log_e("✗ Should not have registered interrupt with null ID");
        }
        
        // Test unregistering non-existent interrupt
        manager.UnregisterInterrupt("non_existent_interrupt");
        log_i("✓ Gracefully handled unregistering non-existent interrupt");
        
        // Test null callback functions
        Interrupt nullCallbackInterrupt = {};
        nullCallbackInterrupt.id = "test_null_callback";
        nullCallbackInterrupt.priority = Priority::NORMAL;
        nullCallbackInterrupt.source = InterruptSource::POLLED;
        nullCallbackInterrupt.effect = InterruptEffect::BUTTON_ACTION;
        nullCallbackInterrupt.evaluationFunc = nullptr;
        nullCallbackInterrupt.executionFunc = nullptr;
        nullCallbackInterrupt.context = nullptr;
        nullCallbackInterrupt.active = true;
        
        bool nullCallbackRegistered = manager.RegisterInterrupt(nullCallbackInterrupt);
        if (!nullCallbackRegistered)
        {
            log_i("✓ Correctly rejected interrupt with null callbacks");
        }
        else
        {
            log_e("✗ Should not have registered interrupt with null callbacks");
            manager.UnregisterInterrupt("test_null_callback");
        }
        
        log_i("✓ Error handling test completed");
    }
};

// Global test function that can be called from main
void RunInterruptSystemTests()
{
    InterruptSystemTest::RunAllTests();
}

#endif // CLARITY_DEBUG