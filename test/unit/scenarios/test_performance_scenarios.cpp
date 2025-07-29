#include <unity.h>
#include "mock_utilities.h"
#include "mock_managers.h"
#include "mock_types.h"
#include <chrono>

using namespace std::chrono;

// S5.1: High-Frequency Trigger Events
void test_S5_1_high_frequency_trigger_events(void) {
    InitializeTriggersFromGpio();
    
    auto start = high_resolution_clock::now();
    
    // Generate 100 rapid trigger events
    for(int i = 0; i < 100; i++) {
        SetTrigger(TRIGGER_LOCK, true);
        SetTrigger(TRIGGER_LOCK, false);
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    
    // Should process all events within reasonable time (adjust threshold as needed)
    TEST_ASSERT_LESS_THAN(1000, duration.count());
    TEST_ASSERT_EQUAL(PANEL_OIL, GetCurrentPanel());
}

// S5.3: Panel Load Performance
void test_S5_3_panel_load_performance(void) {
    InitializeTriggersFromGpio();
    
    auto start = high_resolution_clock::now();
    
    // Switch between all panel types rapidly
    for(int i = 0; i < 10; i++) {
        SetTrigger(TRIGGER_KEY_PRESENT, true);
        SetTrigger(TRIGGER_KEY_PRESENT, false);
        SetTrigger(TRIGGER_LOCK, true);
        SetTrigger(TRIGGER_LOCK, false);
        // Let it return to oil panel
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    
    // Should complete all panel switches within reasonable time
    TEST_ASSERT_LESS_THAN(2000, duration.count());
    TEST_ASSERT_EQUAL(PANEL_OIL, GetCurrentPanel());
}
