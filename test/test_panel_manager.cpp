#ifdef UNIT_TESTING

#include <unity.h>
#include <string>
#include <memory>
#include <functional>
#include <map>

// Mock dependencies
class MockTicker {
public:
    static void handle_lv_tasks() {}
    static void execute_throttled(int interval, std::function<void()> callback) {
        callback();
    }
};

class MockInterruptManager {
private:
    std::function<void(const char*)> _panel_switch_callback;
    std::string _current_panel;
    bool _triggers_active = false;

public:
    static MockInterruptManager& get_instance() {
        static MockInterruptManager instance;
        return instance;
    }

    void init(std::function<void(const char*)> callback) {
        _panel_switch_callback = callback;
    }

    void set_current_panel(const char* panel_name) {
        _current_panel = panel_name;
    }

    bool check_triggers() {
        if (_triggers_active && _panel_switch_callback) {
            _panel_switch_callback("TestTriggerPanel");
            return true;
        }
        return false;
    }

    void clear_panel_triggers() {}

    void set_triggers_active(bool active) {
        _triggers_active = active;
    }

    template<typename T>
    void register_global_trigger(const char* trigger_id, std::shared_ptr<T> trigger) {}
};

// Mock IPanel interface
class MockPanel {
protected:
    std::function<void()> _callback_function;
    std::string _name;
    bool _init_called = false;
    bool _load_called = false;
    bool _update_called = false;

public:
    MockPanel(const std::string& name) : _name(name) {}

    const char* get_name() const { return _name.c_str(); }

    void init() {
        _init_called = true;
    }

    void load(std::function<void()> callback_function) {
        _load_called = true;
        _callback_function = callback_function;
        // Simulate async completion
        if (_callback_function) {
            _callback_function();
        }
    }

    void update(std::function<void()> callback_function) {
        _update_called = true;
        _callback_function = callback_function;
        // Simulate async completion
        if (_callback_function) {
            _callback_function();
        }
    }

    // Test helpers
    bool was_init_called() const { return _init_called; }
    bool was_load_called() const { return _load_called; }
    bool was_update_called() const { return _update_called; }
};

// Mock panel types
class MockSplashPanel : public MockPanel {
public:
    MockSplashPanel() : MockPanel("SplashPanel") {}
};

class MockOemOilPanel : public MockPanel {
public:
    MockOemOilPanel() : MockPanel("OemOilPanel") {}
};

class MockKeyPanel : public MockPanel {
public:
    MockKeyPanel() : MockPanel("KeyPanel") {}
};

class MockLockPanel : public MockPanel {
public:
    MockLockPanel() : MockPanel("LockPanel") {}
};

// Mock PanelManager implementation for testing
class MockPanelManager {
private:
    std::shared_ptr<MockPanel> _panel = nullptr;
    std::map<std::string, std::function<std::shared_ptr<MockPanel>()>> _registered_panels;
    bool _is_loading = false;
    std::string _last_non_trigger_panel;

public:
    void init() {
        register_panels();
        
        MockInterruptManager& interrupt_manager = MockInterruptManager::get_instance();
        interrupt_manager.init([this](const char* panel_name) {
            this->create_and_load_panel(panel_name, [this]() { this->interrupt_panel_switch_callback(); }, true);
        });
    }

    void create_and_load_panel(const char* panel_name, std::function<void()> completion_callback = nullptr, bool is_trigger_driven = false) {
        if (!is_trigger_driven) {
            _last_non_trigger_panel = std::string(panel_name);
        }

        // Check for trigger activations
        if (MockInterruptManager::get_instance().check_triggers()) {
            return;
        }

        MockInterruptManager::get_instance().clear_panel_triggers();
        MockInterruptManager::get_instance().set_current_panel(panel_name);
        
        // Clean up existing panel
        if (_panel) {
            _panel.reset();
        }
        
        _panel = create_panel(panel_name);
        
        if (!_panel) {
            return;
        }

        _is_loading = true;
        _panel->init();
        
        auto callback = completion_callback ? completion_callback : [this]() { this->panel_completion_callback(); };
        _panel->load(callback);
    }

    void create_and_load_panel_with_splash(const char* panel_name) {
        create_and_load_panel("SplashPanel", [this, panel_name]() {
            this->splash_completion_callback(panel_name);
        });
    }

    void update_panel() {
        MockInterruptManager::get_instance().check_triggers();
        
        if (!_panel) {
            return;
        }

        _is_loading = true;
        _panel->update([this]() { this->panel_completion_callback(); });
    }

    const char* get_restoration_panel() const {
        if (_last_non_trigger_panel.empty()) {
            return nullptr;
        }
        return _last_non_trigger_panel.c_str();
    }

    template<typename T>
    void register_panel(const char* panel_name) {
        _registered_panels[panel_name] = []() -> std::shared_ptr<MockPanel> {
            return std::make_shared<T>();
        };
    }

    // Test helpers
    std::shared_ptr<MockPanel> get_current_panel() const { return _panel; }
    bool is_loading() const { return _is_loading; }
    size_t get_registered_panel_count() const { return _registered_panels.size(); }

private:
    std::shared_ptr<MockPanel> create_panel(const char* panel_name) {
        auto iterator = _registered_panels.find(panel_name);
        if (iterator == _registered_panels.end()) {
            return nullptr;
        }
        return iterator->second();
    }

    void register_panels() {
        register_panel<MockSplashPanel>("SplashPanel");
        register_panel<MockOemOilPanel>("OemOilPanel");
        register_panel<MockKeyPanel>("KeyPanel");
        register_panel<MockLockPanel>("LockPanel");
    }

    void splash_completion_callback(const char* panel_name) {
        _panel.reset();
        create_and_load_panel(panel_name);
    }

    void panel_completion_callback() {
        _is_loading = false;
    }

    void interrupt_panel_switch_callback() {
        _is_loading = false;
    }
};

// Test fixtures
MockPanelManager* test_manager = nullptr;

void panel_manager_setUp(void) {
    test_manager = new MockPanelManager();
}

void panel_manager_tearDown(void) {
    delete test_manager;
    test_manager = nullptr;
}

// Test cases
void test_panel_manager_singleton_initialization(void) {
    test_manager->init();
    TEST_ASSERT_EQUAL(4, test_manager->get_registered_panel_count());
}

void test_panel_registration_and_creation(void) {
    test_manager->init();
    
    test_manager->create_and_load_panel("OemOilPanel");
    
    auto current_panel = test_manager->get_current_panel();
    TEST_ASSERT_NOT_NULL(current_panel.get());
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", current_panel->get_name());
    TEST_ASSERT_TRUE(current_panel->was_init_called());
    TEST_ASSERT_TRUE(current_panel->was_load_called());
}

void test_panel_creation_with_invalid_name(void) {
    test_manager->init();
    
    test_manager->create_and_load_panel("InvalidPanel");
    
    auto current_panel = test_manager->get_current_panel();
    TEST_ASSERT_NULL(current_panel.get());
}

void test_panel_lifecycle_completion(void) {
    test_manager->init();
    
    test_manager->create_and_load_panel("KeyPanel");
    
    // Loading should be completed after callback
    TEST_ASSERT_FALSE(test_manager->is_loading());
    
    auto current_panel = test_manager->get_current_panel();
    TEST_ASSERT_NOT_NULL(current_panel.get());
    TEST_ASSERT_EQUAL_STRING("KeyPanel", current_panel->get_name());
}

void test_panel_update_functionality(void) {
    test_manager->init();
    test_manager->create_and_load_panel("OemOilPanel");
    
    auto current_panel = test_manager->get_current_panel();
    TEST_ASSERT_NOT_NULL(current_panel.get());
    
    test_manager->update_panel();
    
    TEST_ASSERT_TRUE(current_panel->was_update_called());
    TEST_ASSERT_FALSE(test_manager->is_loading());
}

void test_panel_switching_cleanup(void) {
    test_manager->init();
    
    // Load first panel
    test_manager->create_and_load_panel("OemOilPanel");
    auto first_panel = test_manager->get_current_panel();
    TEST_ASSERT_NOT_NULL(first_panel.get());
    
    // Switch to different panel
    test_manager->create_and_load_panel("KeyPanel");
    auto second_panel = test_manager->get_current_panel();
    
    TEST_ASSERT_NOT_NULL(second_panel.get());
    TEST_ASSERT_EQUAL_STRING("KeyPanel", second_panel->get_name());
    TEST_ASSERT_NOT_EQUAL(first_panel.get(), second_panel.get());
}

void test_panel_manager_splash_workflow(void) {
    test_manager->init();
    
    test_manager->create_and_load_panel_with_splash("OemOilPanel");
    
    // Should end up with the target panel after splash completion
    auto current_panel = test_manager->get_current_panel();
    TEST_ASSERT_NOT_NULL(current_panel.get());
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", current_panel->get_name());
}

void test_trigger_driven_panel_switching(void) {
    test_manager->init();
    
    // Set up normal panel
    test_manager->create_and_load_panel("OemOilPanel");
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", test_manager->get_restoration_panel());
    
    // Trigger-driven panel change shouldn't affect restoration panel
    test_manager->create_and_load_panel("KeyPanel", nullptr, true);
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", test_manager->get_restoration_panel());
}

void test_restoration_panel_tracking(void) {
    test_manager->init();
    
    // Initially no restoration panel
    TEST_ASSERT_NULL(test_manager->get_restoration_panel());
    
    // Load user-driven panel
    test_manager->create_and_load_panel("OemOilPanel");
    TEST_ASSERT_EQUAL_STRING("OemOilPanel", test_manager->get_restoration_panel());
    
    // Switch to different user-driven panel
    test_manager->create_and_load_panel("KeyPanel");
    TEST_ASSERT_EQUAL_STRING("KeyPanel", test_manager->get_restoration_panel());
}

void test_interrupt_manager_integration(void) {
    test_manager->init();
    
    // Set up normal panel
    test_manager->create_and_load_panel("OemOilPanel");
    
    // Simulate trigger activation
    MockInterruptManager::get_instance().set_triggers_active(true);
    
    // Update should trigger panel switch
    test_manager->update_panel();
    
    auto current_panel = test_manager->get_current_panel();
    TEST_ASSERT_NOT_NULL(current_panel.get());
    // Panel should have been switched by trigger
}

void test_panel_manager_main() {
    // Each test needs its own setup/teardown
    panel_manager_setUp(); RUN_TEST(test_panel_manager_singleton_initialization); panel_manager_tearDown();
    panel_manager_setUp(); RUN_TEST(test_panel_registration_and_creation); panel_manager_tearDown();
    panel_manager_setUp(); RUN_TEST(test_panel_creation_with_invalid_name); panel_manager_tearDown();
    panel_manager_setUp(); RUN_TEST(test_panel_lifecycle_completion); panel_manager_tearDown();
    panel_manager_setUp(); RUN_TEST(test_panel_update_functionality); panel_manager_tearDown();
    panel_manager_setUp(); RUN_TEST(test_panel_switching_cleanup); panel_manager_tearDown();
    panel_manager_setUp(); RUN_TEST(test_panel_manager_splash_workflow); panel_manager_tearDown();
    panel_manager_setUp(); RUN_TEST(test_trigger_driven_panel_switching); panel_manager_tearDown();
    panel_manager_setUp(); RUN_TEST(test_restoration_panel_tracking); panel_manager_tearDown();
    panel_manager_setUp(); RUN_TEST(test_interrupt_manager_integration); panel_manager_tearDown();
}

#endif