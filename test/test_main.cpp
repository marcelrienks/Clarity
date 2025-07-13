#ifdef UNIT_TESTING

#include <unity.h>

// Function declarations from individual test files
extern void test_preference_manager_main();
extern void test_sensor_logic_main();
extern void test_interrupt_manager_main();

void setUp(void) {
    // Setup before each test
}

void tearDown(void) {
    // Cleanup after each test
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    // Run all test suites
    test_preference_manager_main();
    test_sensor_logic_main();
    test_interrupt_manager_main();
    
    return UNITY_END();
}

#endif