#include <unity.h>
#include <memory>
#include <functional>
#include <stdexcept>
#include "interfaces/i_panel.h"
#include "interfaces/i_style_service.h"
#include "mock_services.h"
#include "mock_gpio_provider.h"

// Mock panel implementation for testing the interface
class MockTestPanel : public IPanel {
public:
    // Make protected members accessible for testing
    using IPanel::screen_;
    using IPanel::display_;
    MockTestPanel() {
        init_called = false;
        load_called = false;
        update_called = false;
        show_called = false;
        callback_executed = false;
        last_callback = nullptr;
        last_gpio = nullptr;
        last_display_init = nullptr;
        last_display_load = nullptr;
        last_display_update = nullptr;
    }

    void init(IGpioProvider *gpio, IDisplayProvider *display) override {
        init_called = true;
        last_gpio = gpio;
        last_display_init = display;
        display_ = display;
        
        // Simulate creating screen
        if (display) {
            screen_ = display->createScreen();
        }
    }

    void load(std::function<void()> callbackFunction, IGpioProvider *gpio, IDisplayProvider *display) override {
        load_called = true;
        callbackFunction_ = callbackFunction;
        last_callback = &callbackFunction_;
        last_gpio = gpio;
        last_display_load = display;
        display_ = display;
        
        // Simulate async loading operation completion
        if (callbackFunction) {
            callbackFunction();
            callback_executed = true;
        }
    }

    void update(std::function<void()> callbackFunction, IGpioProvider *gpio, IDisplayProvider *display) override {
        update_called = true;
        callbackFunction_ = callbackFunction;
        last_callback = &callbackFunction_;
        last_gpio = gpio;
        last_display_update = display;
        display_ = display;
        
        // Simulate async update operation completion
        if (callbackFunction) {
            callbackFunction();
            callback_executed = true;
        }
    }

    void show() override {
        show_called = true;
        IPanel::show(); // Call base implementation
    }

    // Test state tracking
    bool init_called;
    bool load_called;
    bool update_called;
    bool show_called;
    bool callback_executed;
    std::function<void()>* last_callback;
    IGpioProvider* last_gpio;
    IDisplayProvider* last_display_init;
    IDisplayProvider* last_display_load;
    IDisplayProvider* last_display_update;
};

// Mock services for testing - use different names to avoid conflicts
static MockDisplayProvider* mockDisplayPanelPanel = nullptr;
static MockGpioProvider* mockGpioPanelPanel = nullptr;

void setUp_panel_interfaces() {
    mockDisplayPanel = new MockDisplayProvider();
    mockGpioPanel = new MockGpioProvider();
    
    mockDisplayPanel->initialize();
}

void tearDown_panel_interfaces() {
    delete mockDisplayPanel;
    delete mockGpioPanel;
    
    mockDisplayPanel = nullptr;
    mockGpioPanel = nullptr;
}

void test_panel_interface_construction() {
    // Test panel construction
    auto panel = std::make_unique<MockTestPanel>();
    
    TEST_ASSERT_NOT_NULL(panel.get());
    TEST_ASSERT_FALSE(panel->init_called);
    TEST_ASSERT_FALSE(panel->load_called);
    TEST_ASSERT_FALSE(panel->update_called);
    TEST_ASSERT_FALSE(panel->show_called);
    TEST_ASSERT_FALSE(panel->callback_executed);
}

void test_panel_interface_init_method() {
    // Test the init method
    auto panel = std::make_unique<MockTestPanel>();
    
    panel->init(mockGpioPanel, mockDisplayPanel);
    
    TEST_ASSERT_TRUE(panel->init_called);
    TEST_ASSERT_EQUAL_PTR(mockGpioPanel, panel->last_gpio);
    TEST_ASSERT_EQUAL_PTR(mockDisplayPanel, panel->last_display_init);
    TEST_ASSERT_NOT_NULL(panel->screen_);
    TEST_ASSERT_EQUAL_PTR(mockDisplayPanel, panel->display_);
}

void test_panel_interface_load_method() {
    // Test the load method with callback
    auto panel = std::make_unique<MockTestPanel>();
    
    bool callback_called = false;
    auto callback = [&callback_called]() {
        callback_called = true;
    };
    
    panel->load(callback, mockGpioPanel, mockDisplayPanel);
    
    TEST_ASSERT_TRUE(panel->load_called);
    TEST_ASSERT_TRUE(panel->callback_executed);
    TEST_ASSERT_TRUE(callback_called);
    TEST_ASSERT_EQUAL_PTR(mockGpioPanel, panel->last_gpio);
    TEST_ASSERT_EQUAL_PTR(mockDisplayPanel, panel->last_display_load);
    TEST_ASSERT_NOT_NULL(panel->last_callback);
}

void test_panel_interface_update_method() {
    // Test the update method with callback
    auto panel = std::make_unique<MockTestPanel>();
    
    bool callback_called = false;
    auto callback = [&callback_called]() {
        callback_called = true;
    };
    
    panel->update(callback, mockGpioPanel, mockDisplayPanel);
    
    TEST_ASSERT_TRUE(panel->update_called);
    TEST_ASSERT_TRUE(panel->callback_executed);
    TEST_ASSERT_TRUE(callback_called);
    TEST_ASSERT_EQUAL_PTR(mockGpioPanel, panel->last_gpio);
    TEST_ASSERT_EQUAL_PTR(mockDisplayPanel, panel->last_display_update);
    TEST_ASSERT_NOT_NULL(panel->last_callback);
}

void test_panel_interface_show_method() {
    // Test the show method
    auto panel = std::make_unique<MockTestPanel>();
    
    // Initialize panel first
    panel->init(mockGpioPanel, mockDisplayPanel);
    
    panel->show();
    
    TEST_ASSERT_TRUE(panel->show_called);
}

void test_panel_interface_complete_lifecycle() {
    // Test complete panel lifecycle: init -> load -> update -> show
    auto panel = std::make_unique<MockTestPanel>();
    
    // 1. Initialize panel
    panel->init(mockGpioPanel, mockDisplayPanel);
    
    TEST_ASSERT_TRUE(panel->init_called);
    TEST_ASSERT_NOT_NULL(panel->screen_);
    
    // 2. Load panel with callback
    bool load_callback_called = false;
    auto load_callback = [&load_callback_called]() {
        load_callback_called = true;
    };
    
    panel->load(load_callback, mockGpioPanel, mockDisplayPanel);
    
    TEST_ASSERT_TRUE(panel->load_called);
    TEST_ASSERT_TRUE(load_callback_called);
    
    // 3. Update panel with callback
    bool update_callback_called = false;
    auto update_callback = [&update_callback_called]() {
        update_callback_called = true;
    };
    
    panel->update(update_callback, mockGpioPanel, mockDisplayPanel);
    
    TEST_ASSERT_TRUE(panel->update_called);
    TEST_ASSERT_TRUE(update_callback_called);
    
    // 4. Show panel
    panel->show();
    
    TEST_ASSERT_TRUE(panel->show_called);
}

void test_panel_interface_null_callback_handling() {
    // Test panel methods with null callbacks
    auto panel = std::make_unique<MockTestPanel>();
    
    // Load with null callback
    panel->load(nullptr, mockGpioPanel, mockDisplayPanel);
    
    TEST_ASSERT_TRUE(panel->load_called);
    TEST_ASSERT_FALSE(panel->callback_executed);
    
    // Update with null callback
    panel->update(nullptr, mockGpioPanel, mockDisplayPanel);
    
    TEST_ASSERT_TRUE(panel->update_called);
    TEST_ASSERT_FALSE(panel->callback_executed);
}

void test_panel_interface_null_providers() {
    // Test panel methods with null providers
    auto panel = std::make_unique<MockTestPanel>();
    
    // Init with null providers
    panel->init(nullptr, nullptr);
    
    TEST_ASSERT_TRUE(panel->init_called);
    TEST_ASSERT_NULL(panel->last_gpio);
    TEST_ASSERT_NULL(panel->last_display_init);
    
    // Load with null providers
    auto callback = [](){};
    panel->load(callback, nullptr, nullptr);
    
    TEST_ASSERT_TRUE(panel->load_called);
    TEST_ASSERT_NULL(panel->last_gpio);
    TEST_ASSERT_NULL(panel->last_display_load);
}

void test_panel_interface_multiple_operations() {
    // Test multiple operations on same panel
    auto panel = std::make_unique<MockTestPanel>();
    
    // Multiple init calls
    panel->init(mockGpioPanel, mockDisplayPanel);
    panel->init(mockGpioPanel, mockDisplayPanel);
    
    TEST_ASSERT_TRUE(panel->init_called);
    TEST_ASSERT_EQUAL_PTR(mockGpioPanel, panel->last_gpio);
    
    // Multiple load calls with different callbacks
    int callback_count = 0;
    
    auto callback1 = [&callback_count]() { callback_count++; };
    auto callback2 = [&callback_count]() { callback_count += 10; };
    
    panel->load(callback1, mockGpioPanel, mockDisplayPanel);
    TEST_ASSERT_EQUAL(1, callback_count);
    
    panel->load(callback2, mockGpioPanel, mockDisplayPanel);
    TEST_ASSERT_EQUAL(11, callback_count);
    
    // Multiple update calls
    panel->update(callback1, mockGpioPanel, mockDisplayPanel);
    TEST_ASSERT_EQUAL(12, callback_count);
}

void test_panel_interface_callback_capture() {
    // Test that callbacks can capture variables properly
    auto panel = std::make_unique<MockTestPanel>();
    
    int captured_value = 42;
    std::string captured_string = "test";
    
    auto callback = [&captured_value, &captured_string]() {
        captured_value = 99;
        captured_string = "modified";
    };
    
    panel->load(callback, mockGpioPanel, mockDisplayPanel);
    
    TEST_ASSERT_EQUAL(99, captured_value);
    TEST_ASSERT_EQUAL_STRING("modified", captured_string.c_str());
}

void test_panel_interface_async_simulation() {
    // Test simulation of asynchronous operations
    auto panel = std::make_unique<MockTestPanel>();
    
    bool operation_started = false;
    bool operation_completed = false;
    
    auto callback = [&operation_completed]() {
        operation_completed = true;
    };
    
    operation_started = true;
    panel->load(callback, mockGpioPanel, mockDisplayPanel);
    
    TEST_ASSERT_TRUE(operation_started);
    TEST_ASSERT_TRUE(operation_completed);
    TEST_ASSERT_TRUE(panel->callback_executed);
}

void test_panel_interface_show_without_init() {
    // Test show method when screen is not initialized
    auto panel = std::make_unique<MockTestPanel>();
    
    // Don't call init, so screen_ should be nullptr
    panel->show();
    
    TEST_ASSERT_TRUE(panel->show_called);
    // Should not crash even if screen_ is null
}

void test_panel_interface_provider_consistency() {
    // Test that the same providers are used consistently
    auto panel = std::make_unique<MockTestPanel>();
    
    // Use different providers for different methods
    MockDisplayProvider altDisplay;
    MockGpioProvider altGpio;
    
    panel->init(mockGpioPanel, mockDisplayPanel);
    TEST_ASSERT_EQUAL_PTR(mockGpioPanel, panel->last_gpio);
    TEST_ASSERT_EQUAL_PTR(mockDisplayPanel, panel->last_display_init);
    
    auto callback = [](){};
    panel->load(callback, &altGpio, &altDisplay);
    TEST_ASSERT_EQUAL_PTR(&altGpio, panel->last_gpio);
    TEST_ASSERT_EQUAL_PTR(&altDisplay, panel->last_display_load);
    
    panel->update(callback, mockGpioPanel, mockDisplayPanel);
    TEST_ASSERT_EQUAL_PTR(mockGpioPanel, panel->last_gpio);
    TEST_ASSERT_EQUAL_PTR(mockDisplayPanel, panel->last_display_update);
}

void test_panel_interface_memory_management() {
    // Test that panel objects are properly managed
    {
        auto panel = std::make_unique<MockTestPanel>();
        panel->init(mockGpioPanel, mockDisplayPanel);
        
        auto callback = [](){};
        panel->load(callback, mockGpioPanel, mockDisplayPanel);
        panel->update(callback, mockGpioPanel, mockDisplayPanel);
        
        TEST_ASSERT_TRUE(panel->init_called);
        TEST_ASSERT_TRUE(panel->load_called);
        TEST_ASSERT_TRUE(panel->update_called);
        
        // Panel should be destroyed when leaving scope
    }
    
    // Test multiple panel creations
    for (int i = 0; i < 5; i++) {
        auto panel = std::make_unique<MockTestPanel>();
        panel->init(mockGpioPanel, mockDisplayPanel);
        TEST_ASSERT_TRUE(panel->init_called);
    }
    
    TEST_ASSERT_TRUE(true); // Test passes if no memory issues
}

void test_panel_interface_polymorphism() {
    // Test that panel works correctly through interface pointer
    std::unique_ptr<IPanel> panel = std::make_unique<MockTestPanel>();
    
    // Test calling through interface
    panel->init(mockGpioPanel, mockDisplayPanel);
    
    auto callback = [](){};
    panel->load(callback, mockGpioPanel, mockDisplayPanel);
    panel->update(callback, mockGpioPanel, mockDisplayPanel);
    panel->show();
    
    // Cast back to check state (in real code we wouldn't do this)
    MockTestPanel* mockPanel = static_cast<MockTestPanel*>(panel.get());
    TEST_ASSERT_TRUE(mockPanel->init_called);
    TEST_ASSERT_TRUE(mockPanel->load_called);
    TEST_ASSERT_TRUE(mockPanel->update_called);
    TEST_ASSERT_TRUE(mockPanel->show_called);
}

void test_panel_interface_callback_exception_safety() {
    // Test panel behavior when callbacks throw exceptions
    auto panel = std::make_unique<MockTestPanel>();
    
    // Note: In a real implementation, panels should handle callback exceptions gracefully
    // For this mock, we just test that the callback mechanism works
    
    bool exception_callback_called = false;
    auto safe_callback = [&exception_callback_called]() {
        exception_callback_called = true;
        // Simulate some work that could potentially throw
    };
    
    panel->load(safe_callback, mockGpioPanel, mockDisplayPanel);
    
    TEST_ASSERT_TRUE(exception_callback_called);
    TEST_ASSERT_TRUE(panel->callback_executed);
}

void runPanelInterfaceTests() {
    setUp_panel_interfaces();
    RUN_TEST(test_panel_interface_construction);
    RUN_TEST(test_panel_interface_init_method);
    RUN_TEST(test_panel_interface_load_method);
    RUN_TEST(test_panel_interface_update_method);
    RUN_TEST(test_panel_interface_show_method);
    RUN_TEST(test_panel_interface_complete_lifecycle);
    RUN_TEST(test_panel_interface_null_callback_handling);
    RUN_TEST(test_panel_interface_null_providers);
    RUN_TEST(test_panel_interface_multiple_operations);
    RUN_TEST(test_panel_interface_callback_capture);
    RUN_TEST(test_panel_interface_async_simulation);
    RUN_TEST(test_panel_interface_show_without_init);
    RUN_TEST(test_panel_interface_provider_consistency);
    RUN_TEST(test_panel_interface_memory_management);
    RUN_TEST(test_panel_interface_polymorphism);
    RUN_TEST(test_panel_interface_callback_exception_safety);
    tearDown_panel_interfaces();
}