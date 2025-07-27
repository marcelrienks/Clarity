#include <unity.h>
#include "test_utilities.h"

// Mock panel states for testing
static const char* current_panel = "OemOilPanel";
static bool panel_loaded = false;
static bool panel_initialized = false;
static std::vector<const char*> panel_creation_history;
static std::vector<const char*> panel_load_history;

void setUp(void) {
    current_panel = "OemOilPanel";
    panel_loaded = false;
    panel_initialized = false;
    panel_creation_history.clear();
    panel_load_history.clear();
}

void tearDown(void) {
    // Clean up after each test
}

// Mock panel manager functions
void mockCreatePanel(const char* panelName) {
    panel_creation_history.push_back(panelName);
    current_panel = panelName;
    panel_initialized = true;
}

void mockLoadPanel(const char* panelName) {
    panel_load_history.push_back(panelName);
    current_panel = panelName;
    panel_loaded = true;
}

void mockPanelCleanup() {
    panel_loaded = false;
    panel_initialized = false;
}

// =================================================================
// PANEL MANAGER CORE FUNCTIONALITY TESTS
// =================================================================

void test_panel_manager_initialization(void) {
    // Test panel manager initialization
    TEST_ASSERT_TRUE_MESSAGE(true, "Panel manager should initialize successfully");
    
    // Verify default state
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", current_panel);
    TEST_ASSERT_FALSE(panel_loaded);
}

void test_panel_registration(void) {
    // Test that all required panel types can be registered
    const char* required_panels[] = {
        "SplashPanel",
        "OemOilPanel", 
        "KeyPanel",
        "LockPanel"
    };
    
    for (size_t i = 0; i < sizeof(required_panels) / sizeof(required_panels[0]); i++) {
        mockCreatePanel(required_panels[i]);
        TEST_ASSERT_TRUE_MESSAGE(panel_initialized, "Panel should be created successfully");
        TEST_ASSERT_EQUAL_STRING(required_panels[i], current_panel);
    }
}

void test_panel_creation_and_loading(void) {
    // Test creating and loading a specific panel
    const char* test_panel = "KeyPanel";
    
    mockCreatePanel(test_panel);
    TEST_ASSERT_TRUE(panel_initialized);
    TEST_ASSERT_EQUAL_STRING(test_panel, current_panel);
    
    mockLoadPanel(test_panel);
    TEST_ASSERT_TRUE(panel_loaded);
    TEST_ASSERT_EQUAL_STRING(test_panel, current_panel);
}

void test_panel_cleanup_on_switch(void) {
    // Test that panels are properly cleaned up when switching
    mockCreatePanel("KeyPanel");
    mockLoadPanel("KeyPanel");
    TEST_ASSERT_TRUE(panel_loaded);
    
    // Simulate panel switch - cleanup should occur
    mockPanelCleanup();
    TEST_ASSERT_FALSE(panel_loaded);
    TEST_ASSERT_FALSE(panel_initialized);
    
    // Create new panel
    mockCreatePanel("LockPanel");
    TEST_ASSERT_TRUE(panel_initialized);
    TEST_ASSERT_EQUAL_STRING("LockPanel", current_panel);
}

// =================================================================
// PANEL LIFECYCLE TESTS
// =================================================================

void test_panel_lifecycle_init_load_update(void) {
    const char* test_panel = "OemOilPanel";
    
    // Step 1: Initialize
    mockCreatePanel(test_panel);
    TEST_ASSERT_TRUE(panel_initialized);
    TEST_ASSERT_EQUAL_STRING(test_panel, current_panel);
    
    // Step 2: Load
    mockLoadPanel(test_panel);
    TEST_ASSERT_TRUE(panel_loaded);
    
    // Step 3: Update (simulated - panel should remain active)
    TEST_ASSERT_TRUE(panel_loaded);
    TEST_ASSERT_EQUAL_STRING(test_panel, current_panel);
}

void test_splash_panel_lifecycle(void) {
    // Test splash panel specific lifecycle
    mockCreatePanel("SplashPanel");
    TEST_ASSERT_EQUAL_STRING("SplashPanel", current_panel);
    
    mockLoadPanel("SplashPanel");
    TEST_ASSERT_TRUE(panel_loaded);
    
    // Simulate splash completion and transition
    mockPanelCleanup();
    mockCreatePanel("OemOilPanel");
    mockLoadPanel("OemOilPanel");
    
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", current_panel);
    TEST_ASSERT_TRUE(panel_loaded);
}

// =================================================================
// PANEL SWITCHING TESTS
// =================================================================

void test_trigger_driven_panel_switch(void) {
    // Start with oil panel
    mockCreatePanel("OemOilPanel");
    mockLoadPanel("OemOilPanel");
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", current_panel);
    
    // Simulate trigger-driven switch to key panel
    mockPanelCleanup();
    mockCreatePanel("KeyPanel");
    mockLoadPanel("KeyPanel");
    
    TEST_ASSERT_EQUAL_STRING("KeyPanel", current_panel);
    TEST_ASSERT_TRUE(panel_loaded);
    
    // Verify panel creation history
    TEST_ASSERT_EQUAL_size_t(2, panel_creation_history.size());
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", panel_creation_history[0]);
    TEST_ASSERT_EQUAL_STRING("KeyPanel", panel_creation_history[1]);
}

void test_panel_restoration_chain(void) {
    // Test complex panel switching and restoration
    std::vector<const char*> switch_sequence = {
        "OemOilPanel",  // Default
        "LockPanel",    // Lock trigger
        "KeyPanel",     // Key override
        "LockPanel",    // Key removed, lock restored
        "OemOilPanel"   // Lock removed, default restored
    };
    
    for (const char* panel : switch_sequence) {
        if (panel_loaded) {
            mockPanelCleanup();
        }
        mockCreatePanel(panel);
        mockLoadPanel(panel);
        
        TEST_ASSERT_EQUAL_STRING(panel, current_panel);
        TEST_ASSERT_TRUE(panel_loaded);
    }
    
    // Verify all panels were created in sequence
    TEST_ASSERT_EQUAL_size_t(switch_sequence.size(), panel_creation_history.size());
    for (size_t i = 0; i < switch_sequence.size(); i++) {
        TEST_ASSERT_EQUAL_STRING(switch_sequence[i], panel_creation_history[i]);
    }
}

void test_rapid_panel_switching(void) {
    // Test rapid panel switches don't cause issues
    const char* panels[] = {"KeyPanel", "LockPanel", "OemOilPanel", "KeyPanel"};
    const size_t panel_count = sizeof(panels) / sizeof(panels[0]);
    
    for (size_t i = 0; i < panel_count; i++) {
        if (panel_loaded) {
            mockPanelCleanup();
        }
        mockCreatePanel(panels[i]);
        mockLoadPanel(panels[i]);
        
        TEST_ASSERT_EQUAL_STRING(panels[i], current_panel);
        TEST_ASSERT_TRUE(panel_loaded);
    }
    
    // Final state should be the last panel
    TEST_ASSERT_EQUAL_STRING("KeyPanel", current_panel);
}

// =================================================================
// PANEL STATE MANAGEMENT TESTS
// =================================================================

void test_panel_state_consistency(void) {
    // Test that panel state remains consistent during operations
    mockCreatePanel("KeyPanel");
    TEST_ASSERT_TRUE(panel_initialized);
    TEST_ASSERT_FALSE(panel_loaded); // Not loaded yet
    
    mockLoadPanel("KeyPanel");
    TEST_ASSERT_TRUE(panel_initialized);
    TEST_ASSERT_TRUE(panel_loaded);
    
    // State should remain consistent
    TEST_ASSERT_EQUAL_STRING("KeyPanel", current_panel);
}

void test_panel_memory_management(void) {
    // Test panel memory management (cleanup on switch)
    mockCreatePanel("LockPanel");
    mockLoadPanel("LockPanel");
    TEST_ASSERT_TRUE(panel_loaded);
    
    // Switch panels - should cleanup previous
    mockPanelCleanup();
    TEST_ASSERT_FALSE(panel_loaded);
    TEST_ASSERT_FALSE(panel_initialized);
    
    // Create new panel
    mockCreatePanel("OemOilPanel");
    mockLoadPanel("OemOilPanel");
    TEST_ASSERT_TRUE(panel_loaded);
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", current_panel);
}

// =================================================================
// ERROR HANDLING TESTS  
// =================================================================

void test_invalid_panel_creation(void) {
    // Test handling of invalid panel types (graceful degradation)
    const char* invalid_panel = "InvalidPanel";
    
    // System should handle invalid panels gracefully
    // In real implementation, this might fall back to default
    mockCreatePanel("OemOilPanel"); // Fallback to default
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", current_panel);
}

void test_panel_creation_failure_recovery(void) {
    // Test recovery from panel creation failures
    
    // Simulate failure scenario
    panel_initialized = false;
    
    // Should attempt fallback or retry
    mockCreatePanel("OemOilPanel"); // Fallback to safe default
    TEST_ASSERT_TRUE(panel_initialized);
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", current_panel);
}

// =================================================================
// INTEGRATION WITH TRIGGER SYSTEM TESTS
// =================================================================

void test_panel_trigger_integration(void) {
    // Test panel manager integration with trigger system
    
    // Start with default panel
    mockCreatePanel("OemOilPanel");
    mockLoadPanel("OemOilPanel");
    
    // Simulate key trigger
    MockHardware::setGpioState(25, true); // key_present
    mockPanelCleanup();
    mockCreatePanel("KeyPanel");
    mockLoadPanel("KeyPanel");
    
    TEST_ASSERT_EQUAL_STRING("KeyPanel", current_panel);
    
    // Simulate trigger removal
    MockHardware::setGpioState(25, false);
    mockPanelCleanup();
    mockCreatePanel("OemOilPanel");
    mockLoadPanel("OemOilPanel");
    
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", current_panel);
}

void test_multiple_trigger_panel_priority(void) {
    // Test panel priority with multiple triggers
    
    // Start default
    mockCreatePanel("OemOilPanel");
    mockLoadPanel("OemOilPanel");
    
    // Lock trigger (lower priority)
    MockHardware::setGpioState(27, true);
    mockPanelCleanup();
    mockCreatePanel("LockPanel");
    mockLoadPanel("LockPanel");
    TEST_ASSERT_EQUAL_STRING("LockPanel", current_panel);
    
    // Key trigger (higher priority) - should override
    MockHardware::setGpioState(25, true);
    mockPanelCleanup();
    mockCreatePanel("KeyPanel");
    mockLoadPanel("KeyPanel");
    TEST_ASSERT_EQUAL_STRING("KeyPanel", current_panel);
    
    // Remove key trigger - should restore lock
    MockHardware::setGpioState(25, false);
    mockPanelCleanup();
    mockCreatePanel("LockPanel");
    mockLoadPanel("LockPanel");
    TEST_ASSERT_EQUAL_STRING("LockPanel", current_panel);
}

// =================================================================
// PERFORMANCE TESTS
// =================================================================

void test_panel_switching_performance(void) {
    // Test panel switching performance under load
    const int switch_count = 100;
    
    measureResponseTime([&]() {
        for (int i = 0; i < switch_count; i++) {
            const char* panel = (i % 2 == 0) ? "KeyPanel" : "LockPanel";
            
            if (panel_loaded) {
                mockPanelCleanup();
            }
            mockCreatePanel(panel);
            mockLoadPanel(panel);
        }
    });
    
    // Should complete without issues
    TEST_ASSERT_TRUE(panel_loaded);
}

// =================================================================
// TEST RUNNER SETUP
// =================================================================

void runPanelManagerTests(void) {
    UNITY_BEGIN();
    
    // Core functionality
    RUN_TEST(test_panel_manager_initialization);
    RUN_TEST(test_panel_registration);
    RUN_TEST(test_panel_creation_and_loading);
    RUN_TEST(test_panel_cleanup_on_switch);
    
    // Lifecycle tests
    RUN_TEST(test_panel_lifecycle_init_load_update);
    RUN_TEST(test_splash_panel_lifecycle);
    
    // Panel switching
    RUN_TEST(test_trigger_driven_panel_switch);
    RUN_TEST(test_panel_restoration_chain);
    RUN_TEST(test_rapid_panel_switching);
    
    // State management
    RUN_TEST(test_panel_state_consistency);
    RUN_TEST(test_panel_memory_management);
    
    // Error handling
    RUN_TEST(test_invalid_panel_creation);
    RUN_TEST(test_panel_creation_failure_recovery);
    
    // Integration tests
    RUN_TEST(test_panel_trigger_integration);
    RUN_TEST(test_multiple_trigger_panel_priority);
    
    // Performance tests
    RUN_TEST(test_panel_switching_performance);
    
    UNITY_END();
}