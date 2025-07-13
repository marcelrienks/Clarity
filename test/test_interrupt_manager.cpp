#ifdef UNIT_TESTING

#include <unity.h>
#include <vector>
#include <algorithm>
#include "utilities/types.h"

// Mock implementation of trigger evaluation logic
struct MockTriggerInfo {
    int id;
    bool is_active;
    int priority;
    std::string target_panel;
    bool should_restore;
    
    MockTriggerInfo(int _id, bool _active, int _priority, std::string _panel, bool _restore = false)
        : id(_id), is_active(_active), priority(_priority), target_panel(_panel), should_restore(_restore) {}
};

// Mock InterruptManager logic for testing
class MockInterruptManager {
private:
    std::vector<MockTriggerInfo> triggers;
    std::vector<int> active_triggers;
    std::string current_panel;
    std::string previous_panel;

public:
    MockInterruptManager() : current_panel("OemOilPanel") {}

    void register_trigger(MockTriggerInfo trigger) {
        triggers.push_back(trigger);
    }

    void set_trigger_state(int id, bool active) {
        for (auto& trigger : triggers) {
            if (trigger.id == id) {
                trigger.is_active = active;
                break;
            }
        }
    }

    std::string evaluate_triggers() {
        // Clear previous active triggers
        active_triggers.clear();
        
        // Find all active triggers
        for (const auto& trigger : triggers) {
            if (trigger.is_active) {
                active_triggers.push_back(trigger.id);
            }
        }

        if (active_triggers.empty()) {
            return current_panel;
        }

        // Find highest priority trigger
        int highest_priority = -1;
        MockTriggerInfo* selected_trigger = nullptr;
        
        for (const auto& trigger : triggers) {
            if (trigger.is_active && trigger.priority > highest_priority) {
                highest_priority = trigger.priority;
                selected_trigger = const_cast<MockTriggerInfo*>(&trigger);
            }
        }

        if (selected_trigger) {
            if (selected_trigger->target_panel != current_panel) {
                previous_panel = current_panel;
                current_panel = selected_trigger->target_panel;
            }
            return selected_trigger->target_panel;
        }

        return current_panel;
    }

    void restore_previous_panel() {
        if (!previous_panel.empty()) {
            std::string temp = current_panel;
            current_panel = previous_panel;
            previous_panel = temp;
        }
    }

    std::string get_current_panel() const { return current_panel; }
    std::string get_previous_panel() const { return previous_panel; }
    size_t get_active_trigger_count() const { return active_triggers.size(); }
    
    bool has_active_restoration_trigger() const {
        for (const auto& trigger : triggers) {
            if (trigger.is_active && trigger.should_restore) {
                return true;
            }
        }
        return false;
    }
};

void setUp(void) {
    // Setup before each test
}

void tearDown(void) {
    // Cleanup after each test
}

void test_no_active_triggers(void) {
    MockInterruptManager manager;
    manager.register_trigger(MockTriggerInfo(1, false, 1, "KeyPanel"));
    manager.register_trigger(MockTriggerInfo(2, false, 2, "LockPanel"));
    
    std::string result = manager.evaluate_triggers();
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", result.c_str());
    TEST_ASSERT_EQUAL(0, manager.get_active_trigger_count());
}

void test_single_active_trigger(void) {
    MockInterruptManager manager;
    manager.register_trigger(MockTriggerInfo(1, true, 1, "KeyPanel"));
    manager.register_trigger(MockTriggerInfo(2, false, 2, "LockPanel"));
    
    std::string result = manager.evaluate_triggers();
    TEST_ASSERT_EQUAL_STRING("KeyPanel", result.c_str());
    TEST_ASSERT_EQUAL(1, manager.get_active_trigger_count());
}

void test_multiple_active_triggers_priority(void) {
    MockInterruptManager manager;
    manager.register_trigger(MockTriggerInfo(1, true, 1, "KeyPanel"));
    manager.register_trigger(MockTriggerInfo(2, true, 5, "LockPanel"));  // Higher priority
    manager.register_trigger(MockTriggerInfo(3, true, 3, "SplashPanel"));
    
    std::string result = manager.evaluate_triggers();
    TEST_ASSERT_EQUAL_STRING("LockPanel", result.c_str());  // Highest priority wins
    TEST_ASSERT_EQUAL(3, manager.get_active_trigger_count());
}

void test_trigger_state_changes(void) {
    MockInterruptManager manager;
    manager.register_trigger(MockTriggerInfo(1, false, 1, "KeyPanel"));
    
    // Initially no active triggers
    std::string result = manager.evaluate_triggers();
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", result.c_str());
    
    // Activate trigger
    manager.set_trigger_state(1, true);
    result = manager.evaluate_triggers();
    TEST_ASSERT_EQUAL_STRING("KeyPanel", result.c_str());
    
    // Deactivate trigger
    manager.set_trigger_state(1, false);
    result = manager.evaluate_triggers();
    TEST_ASSERT_EQUAL_STRING("KeyPanel", result.c_str());  // Panel doesn't change automatically
}

void test_panel_restoration(void) {
    MockInterruptManager manager;
    manager.register_trigger(MockTriggerInfo(1, false, 1, "KeyPanel", true));  // Restoration trigger
    
    // Start with oil panel
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", manager.get_current_panel().c_str());
    
    // Activate restoration trigger - should switch and remember previous
    manager.set_trigger_state(1, true);
    std::string result = manager.evaluate_triggers();
    TEST_ASSERT_EQUAL_STRING("KeyPanel", result.c_str());
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", manager.get_previous_panel().c_str());
    
    // Test restoration
    manager.restore_previous_panel();
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", manager.get_current_panel().c_str());
    TEST_ASSERT_EQUAL_STRING("KeyPanel", manager.get_previous_panel().c_str());
}

void test_restoration_trigger_detection(void) {
    MockInterruptManager manager;
    manager.register_trigger(MockTriggerInfo(1, false, 1, "KeyPanel", false));     // Normal trigger
    manager.register_trigger(MockTriggerInfo(2, false, 2, "LockPanel", true));     // Restoration trigger
    
    // No restoration triggers active initially
    TEST_ASSERT_FALSE(manager.has_active_restoration_trigger());
    
    // Activate normal trigger
    manager.set_trigger_state(1, true);
    manager.evaluate_triggers();
    TEST_ASSERT_FALSE(manager.has_active_restoration_trigger());
    
    // Activate restoration trigger
    manager.set_trigger_state(2, true);
    manager.evaluate_triggers();
    TEST_ASSERT_TRUE(manager.has_active_restoration_trigger());
}

void test_priority_tie_handling(void) {
    MockInterruptManager manager;
    manager.register_trigger(MockTriggerInfo(1, true, 5, "KeyPanel"));
    manager.register_trigger(MockTriggerInfo(2, true, 5, "LockPanel"));    // Same priority
    
    std::string result = manager.evaluate_triggers();
    // Should select the first one found with highest priority
    TEST_ASSERT_EQUAL_STRING("LockPanel", result.c_str());  // Last one registered wins in this implementation
}

void test_complex_trigger_scenario(void) {
    MockInterruptManager manager;
    
    // Register triggers with different priorities and types
    manager.register_trigger(MockTriggerInfo(1, false, 1, "KeyPanel", false));      // Low priority, permanent
    manager.register_trigger(MockTriggerInfo(2, false, 10, "LockPanel", true));     // High priority, restoration
    manager.register_trigger(MockTriggerInfo(3, false, 5, "SplashPanel", false));   // Medium priority, permanent
    
    // Start with oil panel
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", manager.get_current_panel().c_str());
    
    // Activate low priority trigger
    manager.set_trigger_state(1, true);
    std::string result = manager.evaluate_triggers();
    TEST_ASSERT_EQUAL_STRING("KeyPanel", result.c_str());
    
    // Activate high priority restoration trigger (should override)
    manager.set_trigger_state(2, true);
    result = manager.evaluate_triggers();
    TEST_ASSERT_EQUAL_STRING("LockPanel", result.c_str());
    
    // Activate medium priority trigger (should not override high priority)
    manager.set_trigger_state(3, true);
    result = manager.evaluate_triggers();
    TEST_ASSERT_EQUAL_STRING("LockPanel", result.c_str());  // High priority still wins
    
    // Deactivate high priority trigger
    manager.set_trigger_state(2, false);
    result = manager.evaluate_triggers();
    TEST_ASSERT_EQUAL_STRING("LockPanel", result.c_str());  // Panel doesn't auto-switch
    
    // Manually restore if needed and re-evaluate
    manager.restore_previous_panel();
    result = manager.evaluate_triggers();
    TEST_ASSERT_EQUAL_STRING("SplashPanel", result.c_str());  // Medium priority now wins
}

void test_trigger_edge_cases(void) {
    MockInterruptManager manager;
    
    // Test with no triggers registered
    std::string result = manager.evaluate_triggers();
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", result.c_str());
    
    // Test with trigger pointing to same panel as current
    manager.register_trigger(MockTriggerInfo(1, true, 1, "OemOilPanel"));
    result = manager.evaluate_triggers();
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", result.c_str());
    TEST_ASSERT_EQUAL_STRING("", manager.get_previous_panel().c_str());  // No panel change, so no previous
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_no_active_triggers);
    RUN_TEST(test_single_active_trigger);
    RUN_TEST(test_multiple_active_triggers_priority);
    RUN_TEST(test_trigger_state_changes);
    RUN_TEST(test_panel_restoration);
    RUN_TEST(test_restoration_trigger_detection);
    RUN_TEST(test_priority_tie_handling);
    RUN_TEST(test_complex_trigger_scenario);
    RUN_TEST(test_trigger_edge_cases);
    
    return UNITY_END();
}

#endif