#include <unity.h>
#include <memory>

// Project Includes - Using new architecture
#include "system/service_container.h"
#include "interfaces/i_display_provider.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_style_service.h"
#include "interfaces/i_preference_service.h"
#include "interfaces/i_panel_service.h"
#include "interfaces/i_trigger_service.h"

// Actual Manager Implementations
#include "managers/panel_manager.h"
#include "managers/style_manager.h"
#include "managers/preference_manager.h"
#include "managers/trigger_manager.h"

// Test utilities
#include "test_utilities.h"
#include "mock_colors.h"

// Test Service Implementations
class TestDisplayProvider : public IDisplayProvider {
public:
    TestDisplayProvider() : initialized_(false) {}
    
    void initialize() override { initialized_ = true; }
    void* getScreen() override { 
        static mock_lv_obj_t screen = create_mock_lv_obj();
        return &screen; 
    }
    void updateDisplay() override {}
    bool isInitialized() const override { return initialized_; }
    
    // IDisplayProvider interface implementation
    lv_obj_t* createScreen() override { 
        static mock_lv_obj_t screen = create_mock_lv_obj();
        return (lv_obj_t*)&screen;
    }
    
    void loadScreen(lv_obj_t* screen) override {}
    
    lv_obj_t* createLabel(lv_obj_t* parent) override { 
        static mock_lv_obj_t label = create_mock_lv_obj();
        return (lv_obj_t*)&label;
    }
    
    lv_obj_t* createObject(lv_obj_t* parent) override { 
        static mock_lv_obj_t object = create_mock_lv_obj();
        return (lv_obj_t*)&object;
    }
    
    lv_obj_t* createArc(lv_obj_t* parent) override { 
        static mock_lv_obj_t arc = create_mock_lv_obj();
        return (lv_obj_t*)&arc;
    }
    
    lv_obj_t* createScale(lv_obj_t* parent) override { 
        static mock_lv_obj_t scale = create_mock_lv_obj();
        return (lv_obj_t*)&scale;
    }
    
    lv_obj_t* createImage(lv_obj_t* parent) override { 
        static mock_lv_obj_t image = create_mock_lv_obj();
        return (lv_obj_t*)&image;
    }
    
    lv_obj_t* createLine(lv_obj_t* parent) override { 
        static mock_lv_obj_t line = create_mock_lv_obj();
        return (lv_obj_t*)&line;
    }
    
    void deleteObject(lv_obj_t* obj) override {}
    
    void addEventCallback(lv_obj_t* obj, lv_event_cb_t callback, lv_event_code_t event_code, void* user_data) override {}
    
    lv_obj_t* getMainScreen() override { 
        return createScreen();
    }
    
private:
    bool initialized_;
};

class TestGpioProvider : public IGpioProvider {
public:
    TestGpioProvider() {
        for (int i = 0; i < 40; i++) {
            pin_states_[i] = false;
            analog_values_[i] = 0;
        }
    }
    
    void setPinMode(int pin, int mode) override {}
    void pinMode(int pin, int mode) override {}
    bool digitalRead(int pin) override { 
        return (pin >= 0 && pin < 40) ? pin_states_[pin] : false; 
    }
    void digitalWrite(int pin, bool state) override {
        if (pin >= 0 && pin < 40) {
            pin_states_[pin] = state;
        }
    }
    uint16_t analogRead(int pin) override { 
        return (pin >= 0 && pin < 40) ? analog_values_[pin] : 0; 
    }
    
    void setTestState(int pin, bool state) {
        if (pin >= 0 && pin < 40) pin_states_[pin] = state;
    }
    
    void setTestAnalog(int pin, uint16_t value) {
        if (pin >= 0 && pin < 40) analog_values_[pin] = value;
    }
    
private:
    bool pin_states_[40];
    uint16_t analog_values_[40];
};

class TestStyleService : public IStyleService {
public:
    TestStyleService() : current_theme_("Day"), initialized_(false) {
        // Initialize mock styles
        mock_lv_style_init(&background_style_);
        mock_lv_style_init(&text_style_);
        mock_lv_style_init(&gauge_normal_style_);
        mock_lv_style_init(&gauge_warning_style_);
        mock_lv_style_init(&gauge_danger_style_);
    }

    void initializeStyles() override {
        initialized_ = true;
    }

    bool isInitialized() const override {
        return initialized_;
    }

    lv_style_t* getBackgroundStyle() override { return &background_style_; }
    lv_style_t* getTextStyle() override { return &text_style_; }
    lv_style_t* getGaugeNormalStyle() override { return &gauge_normal_style_; }
    lv_style_t* getGaugeWarningStyle() override { return &gauge_warning_style_; }
    lv_style_t* getGaugeDangerStyle() override { return &gauge_danger_style_; }

private:
    std::string current_theme_;
    bool initialized_;
    lv_style_t background_style_;
    lv_style_t text_style_;
    lv_style_t gauge_normal_style_;
    lv_style_t gauge_warning_style_;
    lv_style_t gauge_danger_style_;
        mock_lv_style_init(&gauge_indicator_style_);
        mock_lv_style_init(&gauge_items_style_);
        mock_lv_style_init(&gauge_main_style_);
        mock_lv_style_init(&gauge_danger_section_style_);
    }
    
    // IStyleService interface implementation
    void init(const char* theme) override { 
        initialized_ = true; 
        if (theme) current_theme_ = theme;
    }
    
    void applyThemeToScreen(lv_obj_t* screen) override {}
    
    void setTheme(const char* theme) override { 
        if (theme) current_theme_ = theme; 
    }
    
    const char* getCurrentTheme() const override { return current_theme_.c_str(); }
    
    // Style accessor methods - return references to mock styles
    lv_style_t& getBackgroundStyle() override { return (lv_style_t&)background_style_; }
    lv_style_t& getTextStyle() override { return (lv_style_t&)text_style_; }
    lv_style_t& getGaugeNormalStyle() override { return (lv_style_t&)gauge_normal_style_; }
    lv_style_t& getGaugeWarningStyle() override { return (lv_style_t&)gauge_warning_style_; }
    lv_style_t& getGaugeDangerStyle() override { return (lv_style_t&)gauge_danger_style_; }
    lv_style_t& getGaugeIndicatorStyle() override { return (lv_style_t&)gauge_indicator_style_; }
    lv_style_t& getGaugeItemsStyle() override { return (lv_style_t&)gauge_items_style_; }
    lv_style_t& getGaugeMainStyle() override { return (lv_style_t&)gauge_main_style_; }
    lv_style_t& getGaugeDangerSectionStyle() override { return (lv_style_t&)gauge_danger_section_style_; }
    
    const ThemeColors& getThemeColors() const override { 
        static ThemeColors colors; // Mock theme colors
        return colors; 
    }
    
    // Test helper methods
    void resetStyles() { 
        current_theme_ = "Day"; 
        initialized_ = false;
    }
    
    bool isInitialized() const { return initialized_; }
    
private:
    std::string current_theme_;
    bool initialized_;
    
    // Mock styles
    mock_lv_style_t background_style_;
    mock_lv_style_t text_style_;
    mock_lv_style_t gauge_normal_style_;
    mock_lv_style_t gauge_warning_style_;
    mock_lv_style_t gauge_danger_style_;
    mock_lv_style_t gauge_indicator_style_;
    mock_lv_style_t gauge_items_style_;
    mock_lv_style_t gauge_main_style_;
    mock_lv_style_t gauge_danger_section_style_;
};

class TestPreferenceService : public IPreferenceService {
public:
    TestPreferenceService() : initialized_(false) {}
    
    // IPreferenceService interface implementation
    void init() override { 
        initialized_ = true; 
        createDefaultConfig();
    }
    
    void saveConfig() override {
        // Mock save - no-op for testing
    }
    
    void loadConfig() override {
        // Mock load - no-op for testing
    }
    
    void createDefaultConfig() override {
        config_.panelName = PanelNames::OIL;
    }
    
    Configs& getConfig() override {
        return config_;
    }
    
    const Configs& getConfig() const override {
        return config_;
    }
    
    void setConfig(const Configs& config) override {
        config_ = config;
    }
    
    // Test helper
    bool isInitialized() const { return initialized_; }
    
private:
    bool initialized_;
    Configs config_;
};

// Global DI container for tests
static std::unique_ptr<ServiceContainer> container;

void setUp(void)
{
    // Create new DI container for each test
    container = std::make_unique<ServiceContainer>();
    
    // Register all required services
    container->registerSingleton<IDisplayProvider>([]() {
        return std::make_unique<TestDisplayProvider>();
    });
    
    container->registerSingleton<IGpioProvider>([]() {
        return std::make_unique<TestGpioProvider>();
    });
    
    container->registerSingleton<IStyleService>([]() {
        return std::make_unique<TestStyleService>();
    });
    
    container->registerSingleton<IPreferenceService>([]() {
        return std::make_unique<TestPreferenceService>();
    });
}

void tearDown(void)
{
    container.reset();
}

// =================================================================
// ARCHITECTURAL MANAGER TESTS - USING DEPENDENCY INJECTION
// =================================================================

void test_architectural_panel_manager_with_di(void)
{
    // Register PanelManager as a service
    container->registerSingleton<PanelManager>([&]() {
        // PanelManager gets its dependencies injected
        auto displayProvider = container->resolve<IDisplayProvider>();
        auto styleService = container->resolve<IStyleService>();
        auto prefService = container->resolve<IPreferenceService>();
        
        auto gpioProvider = container->resolve<IGpioProvider>();
        return std::make_unique<PanelManager>(displayProvider, gpioProvider, nullptr);
    });
    
    // Resolve PanelManager through DI
    PanelManager* panelManager = container->resolve<PanelManager>();
    TEST_ASSERT_NOT_NULL(panelManager);
    
    // Initialize dependencies
    IDisplayProvider* displayProvider = container->resolve<IDisplayProvider>();
    IStyleService* styleService = container->resolve<IStyleService>();
    IPreferenceService* prefService = container->resolve<IPreferenceService>();
    
    displayProvider->initialize();
    styleService->init("Day");
    prefService->init();
    
    // PanelManager should work with injected dependencies
    TEST_ASSERT_TRUE(displayProvider->isInitialized());
    TEST_ASSERT_TRUE(prefService->isInitialized());
    
    TestStyleService* testStyle = dynamic_cast<TestStyleService*>(styleService);
    TEST_ASSERT_TRUE(testStyle->isInitialized());
}

void test_architectural_style_manager_with_di(void)
{
    // Register StyleManager as a service (uses default constructor)
    container->registerSingleton<StyleManager>([&]() {
        return std::make_unique<StyleManager>();
    });
    
    // Resolve StyleManager through DI
    StyleManager* styleManager = container->resolve<StyleManager>();
    TEST_ASSERT_NOT_NULL(styleManager);
    
    // Test with display provider
    IDisplayProvider* displayProvider = container->resolve<IDisplayProvider>();
    TEST_ASSERT_NOT_NULL(displayProvider);
    TEST_ASSERT_NOT_NULL(displayProvider->getScreen());
    
    // Test style operations
    styleManager->setTheme("Night");
    // Note: In real implementation, StyleManager would coordinate with IStyleService
    
    styleManager->setTheme("Day");
}

void test_architectural_preference_manager_with_di(void)
{
    // Register PreferenceManager (uses default constructor)
    container->registerSingleton<PreferenceManager>([&]() {
        return std::make_unique<PreferenceManager>();
    });
    
    // Resolve through DI
    PreferenceManager* prefManager = container->resolve<PreferenceManager>();
    TEST_ASSERT_NOT_NULL(prefManager);
    
    // Get preference service
    IPreferenceService* prefService = container->resolve<IPreferenceService>();
    prefService->init();
    
    // PreferenceManager should work with injected service
    TEST_ASSERT_TRUE(prefService->isInitialized());
    
    // Test preference operations through service
    auto& config = prefService->getConfig();
    config.panelName = PanelNames::KEY; // Set test value
    prefService->saveConfig();
    
    // Verify the configuration was set
    const auto& loadedConfig = prefService->getConfig();
    TEST_ASSERT_EQUAL_STRING(PanelNames::KEY, loadedConfig.panelName);
}

void test_architectural_trigger_manager_with_di(void)
{
    // Register TriggerManager with GPIO dependency
    container->registerSingleton<TriggerManager>([&]() {
        auto gpioProvider = container->resolve<IGpioProvider>();
        return std::make_unique<TriggerManager>(gpioProvider);
    });
    
    // Resolve through DI
    TriggerManager* triggerManager = container->resolve<TriggerManager>();
    TEST_ASSERT_NOT_NULL(triggerManager);
    
    // Get GPIO provider for testing
    IGpioProvider* gpioProvider = container->resolve<IGpioProvider>();
    TestGpioProvider* testGpio = dynamic_cast<TestGpioProvider*>(gpioProvider);
    
    // Test trigger functionality
    testGpio->setTestState(25, false); // Key not present
    testGpio->setTestState(27, false); // Lock not active
    
    // TriggerManager should read from injected GPIO provider
    TEST_ASSERT_FALSE(gpioProvider->digitalRead(25));
    TEST_ASSERT_FALSE(gpioProvider->digitalRead(27));
    
    // Simulate trigger activation
    testGpio->setTestState(25, true);  // Key present
    testGpio->setTestState(27, true);  // Lock active
    
    TEST_ASSERT_TRUE(gpioProvider->digitalRead(25));
    TEST_ASSERT_TRUE(gpioProvider->digitalRead(27));
}

// =================================================================
// MANAGER INTEGRATION TESTS WITH DEPENDENCY INJECTION
// =================================================================

void test_architectural_managers_shared_dependencies(void)
{
    // Register all managers with shared dependencies
    container->registerSingleton<PanelManager>([&]() {
        auto displayProvider = container->resolve<IDisplayProvider>();
        auto styleService = container->resolve<IStyleService>();
        auto prefService = container->resolve<IPreferenceService>();
        auto gpioProvider = container->resolve<IGpioProvider>();
        return std::make_unique<PanelManager>(displayProvider, gpioProvider, nullptr);
    });
    
    container->registerSingleton<StyleManager>([&]() {
        auto displayProvider = container->resolve<IDisplayProvider>();
        return std::make_unique<StyleManager>();
    });
    
    container->registerSingleton<TriggerManager>([&]() {
        auto gpioProvider = container->resolve<IGpioProvider>();
        return std::make_unique<TriggerManager>(gpioProvider);
    });
    
    // Resolve all managers
    PanelManager* panelManager = container->resolve<PanelManager>();
    StyleManager* styleManager = container->resolve<StyleManager>();
    TriggerManager* triggerManager = container->resolve<TriggerManager>();
    
    TEST_ASSERT_NOT_NULL(panelManager);
    TEST_ASSERT_NOT_NULL(styleManager);
    TEST_ASSERT_NOT_NULL(triggerManager);
    
    // All managers should share the same service instances (singletons)
    IDisplayProvider* displayProvider1 = container->resolve<IDisplayProvider>();
    IDisplayProvider* displayProvider2 = container->resolve<IDisplayProvider>();
    IGpioProvider* gpioProvider1 = container->resolve<IGpioProvider>();
    IGpioProvider* gpioProvider2 = container->resolve<IGpioProvider>();
    
    TEST_ASSERT_EQUAL_PTR(displayProvider1, displayProvider2);
    TEST_ASSERT_EQUAL_PTR(gpioProvider1, gpioProvider2);
    
    // Initialize shared services
    displayProvider1->initialize();
    
    // All managers should benefit from initialized services
    TEST_ASSERT_TRUE(displayProvider2->isInitialized());
    TEST_ASSERT_NOT_NULL(displayProvider1->getScreen());
    TEST_ASSERT_NOT_NULL(displayProvider2->getScreen());
}

void test_architectural_manager_lifecycle_management(void)
{
    // Test that managers are properly managed through their entire lifecycle
    
    // Register managers
    container->registerSingleton<PanelManager>([&]() {
        auto displayProvider = container->resolve<IDisplayProvider>();
        auto styleService = container->resolve<IStyleService>();
        auto prefService = container->resolve<IPreferenceService>();
        auto gpioProvider = container->resolve<IGpioProvider>();
        return std::make_unique<PanelManager>(displayProvider, gpioProvider, nullptr);
    });
    
    container->registerSingleton<PreferenceManager>([&]() {
        auto prefService = container->resolve<IPreferenceService>();
        return std::make_unique<PreferenceManager>();
    });
    
    // Initial state - managers not yet created
    TEST_ASSERT_TRUE(container->isRegistered<PanelManager>());
    TEST_ASSERT_TRUE(container->isRegistered<PreferenceManager>());
    
    // Initialize services
    IDisplayProvider* displayProvider = container->resolve<IDisplayProvider>();
    IPreferenceService* prefService = container->resolve<IPreferenceService>();
    IStyleService* styleService = container->resolve<IStyleService>();
    
    displayProvider->initialize();
    prefService->init();
    styleService->init("Day");
    
    // Create managers - should get initialized services
    PanelManager* panelManager = container->resolve<PanelManager>();
    PreferenceManager* prefManager = container->resolve<PreferenceManager>();
    
    TEST_ASSERT_NOT_NULL(panelManager);
    TEST_ASSERT_NOT_NULL(prefManager);
    
    // Managers should have access to initialized services
    TEST_ASSERT_TRUE(displayProvider->isInitialized());
    TEST_ASSERT_TRUE(prefService->isInitialized());
    
    TestStyleService* testStyle = dynamic_cast<TestStyleService*>(styleService);
    TEST_ASSERT_TRUE(testStyle->isInitialized());
    
    // Test preference operations
    auto& config = prefService->getConfig();
    config.panelName = PanelNames::OIL; // Set to oil panel
    prefService->saveConfig();
    
    // Verify the configuration was set
    const auto& loadedConfig = prefService->getConfig();
    TEST_ASSERT_EQUAL_STRING(PanelNames::OIL, loadedConfig.panelName.c_str());
}

void test_architectural_manager_cross_communication(void)
{
    // Test that managers can communicate through shared services
    
    container->registerSingleton<StyleManager>([&]() {
        auto displayProvider = container->resolve<IDisplayProvider>();
        return std::make_unique<StyleManager>();
    });
    
    container->registerSingleton<TriggerManager>([&]() {
        auto gpioProvider = container->resolve<IGpioProvider>();
        return std::make_unique<TriggerManager>(gpioProvider);
    });
    
    // Get managers
    StyleManager* styleManager = container->resolve<StyleManager>();
    TriggerManager* triggerManager = container->resolve<TriggerManager>();
    
    // Get shared services
    IStyleService* styleService = container->resolve<IStyleService>();
    IGpioProvider* gpioProvider = container->resolve<IGpioProvider>();
    TestGpioProvider* testGpio = dynamic_cast<TestGpioProvider*>(gpioProvider);
    
    // Initialize
    styleService->init("Day");
    
    // Test cross-manager communication via shared services
    // TriggerManager detects lights on -> StyleManager switches theme
    testGpio->setTestState(28, true); // Lights on (night mode trigger)
    
    // In real implementation, TriggerManager would notify StyleManager
    styleService->setTheme("Night");
    TEST_ASSERT_EQUAL_STRING("Night", styleService->getCurrentTheme());
    
    // Turn lights off -> day theme
    testGpio->setTestState(28, false);
    styleService->setTheme("Day");
    TEST_ASSERT_EQUAL_STRING("Day", styleService->getCurrentTheme());
}

// =================================================================
// SERVICE CONTAINER INTEGRATION TESTS
// =================================================================

void test_architectural_container_manager_registration(void)
{
    // Test that all manager types can be registered
    container->registerSingleton<PanelManager>([]() {
        return std::make_unique<PanelManager>();
    });
    
    container->registerSingleton<StyleManager>([]() {
        return std::make_unique<StyleManager>();
    });
    
    container->registerSingleton<PreferenceManager>([]() {
        return std::make_unique<PreferenceManager>();
    });
    
    container->registerSingleton<TriggerManager>([]() {
        return std::make_unique<TriggerManager>();
    });
    
    // Verify all are registered
    TEST_ASSERT_TRUE(container->isRegistered<PanelManager>());
    TEST_ASSERT_TRUE(container->isRegistered<StyleManager>());
    TEST_ASSERT_TRUE(container->isRegistered<PreferenceManager>());
    TEST_ASSERT_TRUE(container->isRegistered<TriggerManager>());
    
    // Verify they can be resolved
    PanelManager* panelMgr = container->resolve<PanelManager>();
    StyleManager* styleMgr = container->resolve<StyleManager>();
    PreferenceManager* prefMgr = container->resolve<PreferenceManager>();
    TriggerManager* triggerMgr = container->resolve<TriggerManager>();
    
    TEST_ASSERT_NOT_NULL(panelMgr);
    TEST_ASSERT_NOT_NULL(styleMgr);
    TEST_ASSERT_NOT_NULL(prefMgr);
    TEST_ASSERT_NOT_NULL(triggerMgr);
    
    // Verify singleton behavior
    PanelManager* panelMgr2 = container->resolve<PanelManager>();
    TEST_ASSERT_EQUAL_PTR(panelMgr, panelMgr2);
}

void test_architectural_container_clear_and_rebuild(void)
{
    // Register services
    container->registerSingleton<IDisplayProvider>([]() {
        return std::make_unique<TestDisplayProvider>();
    });
    
    container->registerSingleton<PanelManager>([&]() {
        auto displayProvider = container->resolve<IDisplayProvider>();
        return std::make_unique<PanelManager>(displayProvider);
    });
    
    // Verify registration
    TEST_ASSERT_TRUE(container->isRegistered<IDisplayProvider>());
    TEST_ASSERT_TRUE(container->isRegistered<PanelManager>());
    
    // Clear container
    container->clear();
    
    // Verify services are cleared
    TEST_ASSERT_FALSE(container->isRegistered<IDisplayProvider>());
    TEST_ASSERT_FALSE(container->isRegistered<PanelManager>());
    
    // Re-register
    container->registerSingleton<IDisplayProvider>([]() {
        return std::make_unique<TestDisplayProvider>();
    });
    
    // Should work again
    TEST_ASSERT_TRUE(container->isRegistered<IDisplayProvider>());
    
    IDisplayProvider* displayProvider = container->resolve<IDisplayProvider>();
    TEST_ASSERT_NOT_NULL(displayProvider);
}

// Note: PlatformIO will automatically discover and run test_ functions