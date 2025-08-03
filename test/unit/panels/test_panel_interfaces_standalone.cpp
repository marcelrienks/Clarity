#include <unity.h>
#include <memory>
#include <functional>
#include <stdexcept>
#include "interfaces/i_panel.h"
#include "interfaces/i_style_service.h"
#include "mock_services.h"
#include "mock_gpio_provider.h"

// Simple standalone mock panel implementation for interface testing
class StandaloneTestPanel : public IPanel {
public:
    // Make protected members accessible for testing
    using IPanel::screen_;
    using IPanel::display_;
    
    StandaloneTestPanel() {
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

// Standalone mock services
static MockDisplayProvider* standalone_panel_display = nullptr;
static MockGpioProvider* standalone_panel_gpio = nullptr;

void test_standalone_panel_construction() {
    standalone_panel_display = new MockDisplayProvider();
    standalone_panel_gpio = new MockGpioProvider();
    standalone_panel_display->initialize();
    
    auto panel = std::make_unique<StandaloneTestPanel>();
    
    TEST_ASSERT_NOT_NULL(panel.get());
    TEST_ASSERT_FALSE(panel->init_called);
    TEST_ASSERT_FALSE(panel->load_called);
    TEST_ASSERT_FALSE(panel->update_called);
    TEST_ASSERT_FALSE(panel->show_called);
    TEST_ASSERT_FALSE(panel->callback_executed);
    
    delete standalone_panel_display;
    delete standalone_panel_gpio;
}

void test_standalone_panel_init() {
    standalone_panel_display = new MockDisplayProvider();
    standalone_panel_gpio = new MockGpioProvider();
    standalone_panel_display->initialize();
    
    auto panel = std::make_unique<StandaloneTestPanel>();
    
    panel->init(standalone_panel_gpio, standalone_panel_display);
    
    TEST_ASSERT_TRUE(panel->init_called);
    TEST_ASSERT_EQUAL_PTR(standalone_panel_gpio, panel->last_gpio);
    TEST_ASSERT_EQUAL_PTR(standalone_panel_display, panel->last_display_init);
    TEST_ASSERT_NOT_NULL(panel->screen_);
    TEST_ASSERT_EQUAL_PTR(standalone_panel_display, panel->display_);
    
    delete standalone_panel_display;
    delete standalone_panel_gpio;
}

void test_standalone_panel_load() {
    standalone_panel_display = new MockDisplayProvider();
    standalone_panel_gpio = new MockGpioProvider();
    standalone_panel_display->initialize();
    
    auto panel = std::make_unique<StandaloneTestPanel>();
    
    bool callback_called = false;
    auto callback = [&callback_called]() {
        callback_called = true;
    };
    
    panel->load(callback, standalone_panel_gpio, standalone_panel_display);
    
    TEST_ASSERT_TRUE(panel->load_called);
    TEST_ASSERT_TRUE(panel->callback_executed);
    TEST_ASSERT_TRUE(callback_called);
    TEST_ASSERT_EQUAL_PTR(standalone_panel_gpio, panel->last_gpio);
    TEST_ASSERT_EQUAL_PTR(standalone_panel_display, panel->last_display_load);
    TEST_ASSERT_NOT_NULL(panel->last_callback);
    
    delete standalone_panel_display;
    delete standalone_panel_gpio;
}

void test_standalone_panel_update() {
    standalone_panel_display = new MockDisplayProvider();
    standalone_panel_gpio = new MockGpioProvider();
    standalone_panel_display->initialize();
    
    auto panel = std::make_unique<StandaloneTestPanel>();
    
    bool callback_called = false;
    auto callback = [&callback_called]() {
        callback_called = true;
    };
    
    panel->update(callback, standalone_panel_gpio, standalone_panel_display);
    
    TEST_ASSERT_TRUE(panel->update_called);
    TEST_ASSERT_TRUE(panel->callback_executed);
    TEST_ASSERT_TRUE(callback_called);
    TEST_ASSERT_EQUAL_PTR(standalone_panel_gpio, panel->last_gpio);
    TEST_ASSERT_EQUAL_PTR(standalone_panel_display, panel->last_display_update);
    TEST_ASSERT_NOT_NULL(panel->last_callback);
    
    delete standalone_panel_display;
    delete standalone_panel_gpio;
}

void test_standalone_panel_show() {
    standalone_panel_display = new MockDisplayProvider();
    standalone_panel_gpio = new MockGpioProvider();
    standalone_panel_display->initialize();
    
    auto panel = std::make_unique<StandaloneTestPanel>();
    
    // Initialize panel first
    panel->init(standalone_panel_gpio, standalone_panel_display);
    
    panel->show();
    
    TEST_ASSERT_TRUE(panel->show_called);
    
    delete standalone_panel_display;
    delete standalone_panel_gpio;
}

void test_standalone_panel_complete_lifecycle() {
    standalone_panel_display = new MockDisplayProvider();
    standalone_panel_gpio = new MockGpioProvider();
    standalone_panel_display->initialize();
    
    auto panel = std::make_unique<StandaloneTestPanel>();
    
    // 1. Initialize panel
    panel->init(standalone_panel_gpio, standalone_panel_display);
    
    TEST_ASSERT_TRUE(panel->init_called);
    TEST_ASSERT_NOT_NULL(panel->screen_);
    
    // 2. Load panel with callback
    bool load_callback_called = false;
    auto load_callback = [&load_callback_called]() {
        load_callback_called = true;
    };
    
    panel->load(load_callback, standalone_panel_gpio, standalone_panel_display);
    
    TEST_ASSERT_TRUE(panel->load_called);
    TEST_ASSERT_TRUE(load_callback_called);
    
    // 3. Update panel with callback
    bool update_callback_called = false;
    auto update_callback = [&update_callback_called]() {
        update_callback_called = true;
    };
    
    panel->update(update_callback, standalone_panel_gpio, standalone_panel_display);
    
    TEST_ASSERT_TRUE(panel->update_called);
    TEST_ASSERT_TRUE(update_callback_called);
    
    // 4. Show panel
    panel->show();
    
    TEST_ASSERT_TRUE(panel->show_called);
    
    delete standalone_panel_display;
    delete standalone_panel_gpio;
}

void runStandalonePanelTests() {
    RUN_TEST(test_standalone_panel_construction);
    RUN_TEST(test_standalone_panel_init);
    RUN_TEST(test_standalone_panel_load);
    RUN_TEST(test_standalone_panel_update);
    RUN_TEST(test_standalone_panel_show);
    RUN_TEST(test_standalone_panel_complete_lifecycle);
}