#ifndef TEST_ARCHITECTURAL_HELPERS_H
#define TEST_ARCHITECTURAL_HELPERS_H

#include <memory>
#include <map>
#include <string>
#include <set>

// Project Includes
#include "system/service_container.h"
#include "system/component_registry.h"
#include "interfaces/i_display_provider.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_style_service.h"
#include "interfaces/i_preference_service.h"

/**
 * @brief Architectural Test Helpers
 * 
 * This header provides helper classes and utilities for testing with the new
 * architecture, ensuring all tests use dependency injection and the service container.
 */

namespace ArchitecturalTestHelpers {

    /**
     * @brief Test Display Provider that implements IDisplayProvider
     * for dependency injection in tests
     */
    class TestDisplayProvider : public IDisplayProvider {
    public:
        TestDisplayProvider() : main_screen_(nullptr) {}
        
        // IDisplayProvider interface implementation
        lv_obj_t* createScreen() override { 
            static lv_obj_t screen;
            return &screen; 
        }
        
        void loadScreen(lv_obj_t* screen) override { 
            main_screen_ = screen; 
        }
        
        lv_obj_t* createLabel(lv_obj_t* parent) override { 
            static lv_obj_t label;
            return &label; 
        }
        
        lv_obj_t* createObject(lv_obj_t* parent) override { 
            static lv_obj_t obj;
            return &obj; 
        }
        
        lv_obj_t* createArc(lv_obj_t* parent) override { 
            static lv_obj_t arc;
            return &arc; 
        }
        
        lv_obj_t* createScale(lv_obj_t* parent) override { 
            static lv_obj_t scale;
            return &scale; 
        }
        
        lv_obj_t* createImage(lv_obj_t* parent) override { 
            static lv_obj_t image;
            return &image; 
        }
        
        lv_obj_t* createLine(lv_obj_t* parent) override { 
            static lv_obj_t line;
            return &line; 
        }
        
        void deleteObject(lv_obj_t* obj) override {}
        
        void addEventCallback(lv_obj_t* obj, lv_event_cb_t callback, lv_event_code_t event_code, void* user_data) override {}
        
        lv_obj_t* getMainScreen() override { 
            if (!main_screen_) {
                main_screen_ = createScreen();
            }
            return main_screen_; 
        }
        
    private:
        lv_obj_t* main_screen_;
    };

    /**
     * @brief Test GPIO Provider that implements IGpioProvider
     * with controllable state for testing
     */
    class TestGpioProvider : public IGpioProvider {
    public:
        TestGpioProvider() {
            reset();
        }
        
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
            if (pin >= 0 && pin < 40) {
                return failed_pins_.count(pin) ? 0 : analog_values_[pin];
            }
            return 0;
        }
        
        // Test helper methods
        void setTestGpioState(int pin, bool state) {
            if (pin >= 0 && pin < 40) {
                pin_states_[pin] = state;
            }
        }
        
        void setTestAnalogValue(int pin, uint16_t value) {
            if (pin >= 0 && pin < 40) {
                analog_values_[pin] = value;
                failed_pins_.erase(pin); // Clear failure if setting value
            }
        }
        
        void simulateFailure(int pin, bool fail) {
            if (pin >= 0 && pin < 40) {
                if (fail) {
                    failed_pins_.insert(pin);
                } else {
                    failed_pins_.erase(pin);
                }
            }
        }
        
        void reset() {
            for (int i = 0; i < 40; i++) {
                pin_states_[i] = false;
                analog_values_[i] = 0;
            }
            failed_pins_.clear();
            
            // Set realistic defaults for oil sensors
            analog_values_[34] = 2048; // Normal oil pressure
            analog_values_[35] = 1500; // Normal oil temperature
        }
        
    private:
        bool pin_states_[40];
        uint16_t analog_values_[40];
        std::set<int> failed_pins_;
    };

    /**
     * @brief Test Style Service that implements IStyleService
     */
    class TestStyleService : public IStyleService {
    public:
        TestStyleService() : current_theme_("Day"), initialized_(false) {}
        
        // IStyleService interface implementation
        void init(const char* theme) override { 
            initialized_ = true; 
            if (theme) current_theme_ = theme;
        }
        
        void applyThemeToScreen(lv_obj_t* screen) override {
            last_screen_applied_ = screen;
        }
        
        void setTheme(const char* theme) override { 
            if (theme) {
                current_theme_ = theme;
                theme_changes_.push_back(theme);
            }
        }
        
        // Style accessor methods - return mock styles
        lv_style_t& getBackgroundStyle() override { return background_style_; }
        lv_style_t& getTextStyle() override { return text_style_; }
        lv_style_t& getGaugeNormalStyle() override { return gauge_normal_style_; }
        lv_style_t& getGaugeWarningStyle() override { return gauge_warning_style_; }
        lv_style_t& getGaugeDangerStyle() override { return gauge_danger_style_; }
        lv_style_t& getGaugeIndicatorStyle() override { return gauge_indicator_style_; }
        lv_style_t& getGaugeItemsStyle() override { return gauge_items_style_; }
        lv_style_t& getGaugeMainStyle() override { return gauge_main_style_; }
        lv_style_t& getGaugeDangerSectionStyle() override { return gauge_danger_section_style_; }
        
        const char* getCurrentTheme() const override { 
            return current_theme_.c_str(); 
        }
        
        const ThemeColors& getThemeColors() const override {
            return theme_colors_;
        }
        
        // Test helper methods
        bool isInitialized() const { return initialized_; }
        const std::vector<std::string>& getThemeChanges() const { return theme_changes_; }
        void* getLastScreenApplied() const { return last_screen_applied_; }
        
    private:
        std::string current_theme_;
        bool initialized_;
        std::vector<std::string> theme_changes_;
        void* last_screen_applied_ = nullptr;
        
        // Mock style objects
        lv_style_t background_style_;
        lv_style_t text_style_;
        lv_style_t gauge_normal_style_;
        lv_style_t gauge_warning_style_;
        lv_style_t gauge_danger_style_;
        lv_style_t gauge_indicator_style_;
        lv_style_t gauge_items_style_;
        lv_style_t gauge_main_style_;
        lv_style_t gauge_danger_section_style_;
        ThemeColors theme_colors_;
    };

    /**
     * @brief Test Preference Service that implements IPreferenceService
     */
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

    /**
     * @brief Test Setup Helper - Creates and configures a complete DI container
     */
    class TestSetup {
    public:
        TestSetup() {
            container_ = std::make_unique<ServiceContainer>();
            registry_ = std::make_unique<ComponentRegistry>(*container_);
            
            setupServices();
        }
        
        ~TestSetup() = default;
        
        ServiceContainer& getContainer() { return *container_; }
        ComponentRegistry& getRegistry() { return *registry_; }
        
        // Get typed test services for direct manipulation
        TestDisplayProvider* getTestDisplayProvider() {
            return dynamic_cast<TestDisplayProvider*>(container_->resolve<IDisplayProvider>());
        }
        
        TestGpioProvider* getTestGpioProvider() {
            return dynamic_cast<TestGpioProvider*>(container_->resolve<IGpioProvider>());
        }
        
        TestStyleService* getTestStyleService() {
            return dynamic_cast<TestStyleService*>(container_->resolve<IStyleService>());
        }
        
        TestPreferenceService* getTestPreferenceService() {
            return dynamic_cast<TestPreferenceService*>(container_->resolve<IPreferenceService>());
        }
        
        // Initialize all services for testing
        void initializeServices() {
            auto styleService = getTestStyleService();
            auto prefService = getTestPreferenceService();
            
            if (styleService) styleService->init("Day");
            if (prefService) prefService->init();
        }
        
        // Reset all services to clean state
        void resetServices() {
            auto gpioProvider = getTestGpioProvider();
            auto styleService = getTestStyleService();
            auto prefService = getTestPreferenceService();
            
            if (gpioProvider) gpioProvider->reset();
            if (styleService) styleService->init("Day");  // Reset to default
            if (prefService) prefService->createDefaultConfig();  // Reset to defaults
        }
        
    private:
        void setupServices() {
            // Register all standard test services
            container_->registerSingleton<IDisplayProvider>([]() {
                return std::make_unique<TestDisplayProvider>();
            });
            
            container_->registerSingleton<IGpioProvider>([]() {
                return std::make_unique<TestGpioProvider>();
            });
            
            container_->registerSingleton<IStyleService>([]() {
                return std::make_unique<TestStyleService>();
            });
            
            container_->registerSingleton<IPreferenceService>([]() {
                return std::make_unique<TestPreferenceService>();
            });
        }
        
        std::unique_ptr<ServiceContainer> container_;
        std::unique_ptr<ComponentRegistry> registry_;
    };

    /**
     * @brief Scenario Test Helper - For integration tests with realistic scenarios
     */
    class ScenarioTestHelper {
    public:
        ScenarioTestHelper(TestSetup& setup) : setup_(setup) {}
        
        // Simulate engine startup sequence
        void simulateEngineStartup() {
            auto gpio = setup_.getTestGpioProvider();
            if (!gpio) return;
            
            // Cold start - no pressure, cold temperature
            gpio->setTestAnalogValue(34, 0);     // No oil pressure
            gpio->setTestAnalogValue(35, 1200);  // Cold temperature (20°C)
            
            // Cranking - building pressure
            gpio->setTestAnalogValue(34, 500);   // Low pressure while cranking
            
            // Running - normal operating conditions
            gpio->setTestAnalogValue(34, 2048);  // Normal oil pressure (75 PSI)
            gpio->setTestAnalogValue(35, 1500);  // Normal operating temperature (85°C)
        }
        
        // Simulate trigger activation patterns
        void simulateKeyPresentSequence() {
            auto gpio = setup_.getTestGpioProvider();
            if (!gpio) return;
            
            gpio->setTestGpioState(25, true);  // Key present
        }
        
        void simulateLockActiveSequence() {
            auto gpio = setup_.getTestGpioProvider();
            if (!gpio) return;
            
            gpio->setTestGpioState(27, true);  // Lock active
        }
        
        void simulateNightModeSequence() {
            auto gpio = setup_.getTestGpioProvider();
            auto style = setup_.getTestStyleService();
            if (!gpio || !style) return;
            
            gpio->setTestGpioState(28, true);  // Lights on
            style->setTheme("Night");          // Night theme
        }
        
        // Simulate warning conditions
        void simulateLowOilPressureWarning() {
            auto gpio = setup_.getTestGpioProvider();
            if (!gpio) return;
            
            gpio->setTestAnalogValue(34, 200);  // Critically low pressure (3 PSI)
        }
        
        void simulateOverheatingWarning() {
            auto gpio = setup_.getTestGpioProvider();
            if (!gpio) return;
            
            gpio->setTestAnalogValue(35, 3500); // Overheating (125°C)
        }
        
        // Reset to normal conditions
        void resetToNormalConditions() {
            auto gpio = setup_.getTestGpioProvider();
            auto style = setup_.getTestStyleService();
            if (!gpio || !style) return;
            
            gpio->reset(); // This sets normal defaults
            style->setTheme("Day");
        }
        
    private:
        TestSetup& setup_;
    };

} // namespace ArchitecturalTestHelpers

// Convenience macros for architectural tests
#define ARCHITECTURAL_TEST_SETUP() \
    ArchitecturalTestHelpers::TestSetup testSetup; \
    testSetup.initializeServices();

#define ARCHITECTURAL_TEST_TEARDOWN() \
    testSetup.resetServices();

#endif // TEST_ARCHITECTURAL_HELPERS_H