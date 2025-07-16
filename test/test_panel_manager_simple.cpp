#ifdef UNIT_TESTING

#include <unity.h>
#include <string>
#include <memory>
#include <functional>
#include <map>
#include <vector>

// Simple mock panel that doesn't use complex inheritance
struct SimpleMockPanel {
    std::string name;
    bool init_called = false;
    bool load_called = false;
    bool update_called = false;
    bool load_completed = false;
    bool update_completed = false;
    
    SimpleMockPanel(const std::string& panel_name) : name(panel_name) {}
    
    void init() { init_called = true; }
    void load() { load_called = true; load_completed = true; }
    void update() { update_called = true; update_completed = true; }
    const char* get_name() const { return name.c_str(); }
};

// Simplified PanelManager that avoids singleton patterns and complex callbacks
class SimplePanelManager {
private:
    std::map<std::string, std::function<std::shared_ptr<SimpleMockPanel>()>> _registered_panels;
    std::shared_ptr<SimpleMockPanel> _current_panel = nullptr;
    std::string _restoration_panel = "";
    bool _is_loading = false;
    std::vector<std::string> _trigger_log; // Log of trigger events for testing

public:
    void register_panel(const std::string& name, std::function<std::shared_ptr<SimpleMockPanel>()> factory) {
        _registered_panels[name] = factory;
    }
    
    void init() {
        // Register standard panels
        register_panel("SplashPanel", []() { return std::make_shared<SimpleMockPanel>("SplashPanel"); });
        register_panel("OemOilPanel", []() { return std::make_shared<SimpleMockPanel>("OemOilPanel"); });
        register_panel("KeyPanel", []() { return std::make_shared<SimpleMockPanel>("KeyPanel"); });
        register_panel("LockPanel", []() { return std::make_shared<SimpleMockPanel>("LockPanel"); });
    }
    
    bool create_and_load_panel(const std::string& panel_name, bool is_trigger_driven = false) {
        // Track restoration panel for user-driven changes
        if (!is_trigger_driven) {
            _restoration_panel = panel_name;
        }
        
        auto it = _registered_panels.find(panel_name);
        if (it == _registered_panels.end()) {
            return false; // Panel not found
        }
        
        // Clean up existing panel
        if (_current_panel) {
            _current_panel.reset();
        }
        
        // Create and initialize new panel
        _current_panel = it->second();
        if (_current_panel) {
            _is_loading = true;
            _current_panel->init();
            _current_panel->load();
            _is_loading = false;
            return true;
        }
        
        return false;
    }
    
    void update_panel() {
        if (_current_panel) {
            _is_loading = true;
            _current_panel->update();
            _is_loading = false;
        }
    }
    
    void handle_trigger(const std::string& trigger_name, const std::string& target_panel) {
        _trigger_log.push_back(trigger_name);
        create_and_load_panel(target_panel, true);
    }
    
    void create_and_load_panel_with_splash(const std::string& target_panel) {
        // First load splash
        create_and_load_panel("SplashPanel");
        // Then load target panel
        create_and_load_panel(target_panel);
    }
    
    // Getters for testing
    std::shared_ptr<SimpleMockPanel> get_current_panel() const { return _current_panel; }
    const std::string& get_restoration_panel() const { return _restoration_panel; }
    bool is_loading() const { return _is_loading; }
    size_t get_registered_panel_count() const { return _registered_panels.size(); }
    const std::vector<std::string>& get_trigger_log() const { return _trigger_log; }
    
    void reset() {
        _current_panel.reset();
        _restoration_panel = "";
        _is_loading = false;
        _trigger_log.clear();
    }
};

// Test fixture
SimplePanelManager* simple_manager = nullptr;

void simple_panel_manager_setUp(void) {
    simple_manager = new SimplePanelManager();
    simple_manager->init();
}

void simple_panel_manager_tearDown(void) {
    if (simple_manager) {
        simple_manager->reset();
        delete simple_manager;
        simple_manager = nullptr;
    }
}

// Test cases
void test_simple_panel_manager_initialization(void) {
    TEST_ASSERT_NOT_NULL(simple_manager);
    TEST_ASSERT_EQUAL(4, simple_manager->get_registered_panel_count());
    TEST_ASSERT_NULL(simple_manager->get_current_panel().get());
    TEST_ASSERT_FALSE(simple_manager->is_loading());
}

void test_simple_panel_creation_and_loading(void) {
    bool result = simple_manager->create_and_load_panel("OemOilPanel");
    
    TEST_ASSERT_TRUE(result);
    
    auto panel = simple_manager->get_current_panel();
    TEST_ASSERT_NOT_NULL(panel.get());
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", panel->get_name());
    TEST_ASSERT_TRUE(panel->init_called);
    TEST_ASSERT_TRUE(panel->load_called);
    TEST_ASSERT_TRUE(panel->load_completed);
    TEST_ASSERT_FALSE(simple_manager->is_loading()); // Should be false after completion
}

void test_simple_panel_creation_invalid_name(void) {
    bool result = simple_manager->create_and_load_panel("InvalidPanel");
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_NULL(simple_manager->get_current_panel().get());
}

void test_simple_panel_switching(void) {
    // Load first panel
    simple_manager->create_and_load_panel("OemOilPanel");
    auto first_panel = simple_manager->get_current_panel();
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", first_panel->get_name());
    
    // Switch to different panel
    simple_manager->create_and_load_panel("KeyPanel");
    auto second_panel = simple_manager->get_current_panel();
    
    TEST_ASSERT_EQUAL_STRING("KeyPanel", second_panel->get_name());
    TEST_ASSERT_TRUE(second_panel->init_called);
    TEST_ASSERT_TRUE(second_panel->load_called);
    
    // Verify panels are different instances
    TEST_ASSERT_NOT_EQUAL(first_panel.get(), second_panel.get());
}

void test_simple_panel_update_functionality(void) {
    simple_manager->create_and_load_panel("OemOilPanel");
    auto panel = simple_manager->get_current_panel();
    
    TEST_ASSERT_FALSE(panel->update_called);
    
    simple_manager->update_panel();
    
    TEST_ASSERT_TRUE(panel->update_called);
    TEST_ASSERT_TRUE(panel->update_completed);
    TEST_ASSERT_FALSE(simple_manager->is_loading()); // Should be false after completion
}

void test_simple_restoration_panel_tracking(void) {
    // Initially no restoration panel
    TEST_ASSERT_TRUE(simple_manager->get_restoration_panel().empty());
    
    // Load user-driven panel
    simple_manager->create_and_load_panel("OemOilPanel");
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", simple_manager->get_restoration_panel().c_str());
    
    // Switch to different user-driven panel
    simple_manager->create_and_load_panel("KeyPanel");
    TEST_ASSERT_EQUAL_STRING("KeyPanel", simple_manager->get_restoration_panel().c_str());
}

void test_simple_trigger_driven_panel_switching(void) {
    // Set up normal panel
    simple_manager->create_and_load_panel("OemOilPanel");
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", simple_manager->get_restoration_panel().c_str());
    
    // Trigger-driven panel change shouldn't affect restoration panel
    simple_manager->handle_trigger("KeyTrigger", "KeyPanel");
    
    // Current panel should have changed
    TEST_ASSERT_EQUAL_STRING("KeyPanel", simple_manager->get_current_panel()->get_name());
    
    // Restoration panel should remain unchanged
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", simple_manager->get_restoration_panel().c_str());
    
    // Trigger should be logged
    auto& trigger_log = simple_manager->get_trigger_log();
    TEST_ASSERT_EQUAL(1, trigger_log.size());
    TEST_ASSERT_EQUAL_STRING("KeyTrigger", trigger_log[0].c_str());
}

void test_simple_splash_panel_workflow(void) {
    simple_manager->create_and_load_panel_with_splash("OemOilPanel");
    
    // Should end up with the target panel (splash workflow completed)
    auto current_panel = simple_manager->get_current_panel();
    TEST_ASSERT_NOT_NULL(current_panel.get());
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", current_panel->get_name());
    TEST_ASSERT_TRUE(current_panel->init_called);
    TEST_ASSERT_TRUE(current_panel->load_called);
}

void test_simple_panel_lifecycle_states(void) {
    // Test loading state during panel creation
    simple_manager->create_and_load_panel("KeyPanel");
    
    auto panel = simple_manager->get_current_panel();
    TEST_ASSERT_NOT_NULL(panel.get());
    TEST_ASSERT_TRUE(panel->init_called);
    TEST_ASSERT_TRUE(panel->load_called);
    TEST_ASSERT_TRUE(panel->load_completed);
    TEST_ASSERT_FALSE(simple_manager->is_loading()); // Should be false after completion
}

void test_simple_multiple_trigger_handling(void) {
    simple_manager->create_and_load_panel("OemOilPanel");
    
    // Handle multiple triggers
    simple_manager->handle_trigger("KeyTrigger", "KeyPanel");
    simple_manager->handle_trigger("LockTrigger", "LockPanel");
    
    // Should end up with the last triggered panel
    TEST_ASSERT_EQUAL_STRING("LockPanel", simple_manager->get_current_panel()->get_name());
    
    // Both triggers should be logged
    auto& trigger_log = simple_manager->get_trigger_log();
    TEST_ASSERT_EQUAL(2, trigger_log.size());
    TEST_ASSERT_EQUAL_STRING("KeyTrigger", trigger_log[0].c_str());
    TEST_ASSERT_EQUAL_STRING("LockTrigger", trigger_log[1].c_str());
}

void test_simple_panel_manager_main() {
    // Each test needs its own setup/teardown
    simple_panel_manager_setUp(); RUN_TEST(test_simple_panel_manager_initialization); simple_panel_manager_tearDown();
    simple_panel_manager_setUp(); RUN_TEST(test_simple_panel_creation_and_loading); simple_panel_manager_tearDown();
    simple_panel_manager_setUp(); RUN_TEST(test_simple_panel_creation_invalid_name); simple_panel_manager_tearDown();
    simple_panel_manager_setUp(); RUN_TEST(test_simple_panel_switching); simple_panel_manager_tearDown();
    simple_panel_manager_setUp(); RUN_TEST(test_simple_panel_update_functionality); simple_panel_manager_tearDown();
    simple_panel_manager_setUp(); RUN_TEST(test_simple_restoration_panel_tracking); simple_panel_manager_tearDown();
    simple_panel_manager_setUp(); RUN_TEST(test_simple_trigger_driven_panel_switching); simple_panel_manager_tearDown();
    simple_panel_manager_setUp(); RUN_TEST(test_simple_splash_panel_workflow); simple_panel_manager_tearDown();
    simple_panel_manager_setUp(); RUN_TEST(test_simple_panel_lifecycle_states); simple_panel_manager_tearDown();
    simple_panel_manager_setUp(); RUN_TEST(test_simple_multiple_trigger_handling); simple_panel_manager_tearDown();
}

#endif