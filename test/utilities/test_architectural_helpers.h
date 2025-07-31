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
        TestDisplayProvider() : initialized_(false) {}
        
        void initialize() override { initialized_ = true; }
        void* getScreen() override { 
            static mock_lv_obj_t screen = create_mock_lv_obj();
            return &screen; 
        }
        void updateDisplay() override {}
        bool isInitialized() const override { return initialized_; }
        
    private:
        bool initialized_;
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
        
        void setPinMode(int pin, int mode) override {}
        
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
        
        void initializeStyles() override { initialized_ = true; }
        
        void setTheme(const char* theme) override { 
            if (theme) {
                current_theme_ = theme;
                theme_changes_.push_back(theme);
            }
        }
        
        const char* getCurrentTheme() const override { 
            return current_theme_.c_str(); 
        }
        
        void applyToScreen(void* screen) override {
            last_screen_applied_ = screen;
        }
        
        void resetStyles() override { 
            current_theme_ = "Day"; 
            initialized_ = false;
            theme_changes_.clear();
            last_screen_applied_ = nullptr;
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
    };

    /**
     * @brief Test Preference Service that implements IPreferenceService
     */
    class TestPreferenceService : public IPreferenceService {
    public:
        TestPreferenceService() : initialized_(false) {}
        
        void init() override { initialized_ = true; }
        bool isInitialized() const override { return initialized_; }
        
        void saveConfig(const char* key, const char* value) override {
            if (key && value) {
                config_[key] = value;
                save_history_.push_back({key, value});
            }
        }
        
        std::string loadConfig(const char* key, const char* defaultValue) override {
            if (!key) return defaultValue ? defaultValue : "";
            
            load_history_.push_back(key);
            auto it = config_.find(key);
            return (it != config_.end()) ? it->second : (defaultValue ? defaultValue : "");
        }
        
        // Test helper methods
        void clearConfig() { 
            config_.clear(); 
            save_history_.clear();
            load_history_.clear();
        }
        
        const std::map<std::string, std::string>& getConfig() const { return config_; }
        const std::vector<std::pair<std::string, std::string>>& getSaveHistory() const { return save_history_; }
        const std::vector<std::string>& getLoadHistory() const { return load_history_; }
        
    private:
        bool initialized_;
        std::map<std::string, std::string> config_;
        std::vector<std::pair<std::string, std::string>> save_history_;
        std::vector<std::string> load_history_;
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
            auto displayProvider = getTestDisplayProvider();
            auto styleService = getTestStyleService();
            auto prefService = getTestPreferenceService();
            
            if (displayProvider) displayProvider->initialize();
            if (styleService) styleService->initializeStyles();
            if (prefService) prefService->init();
        }
        
        // Reset all services to clean state
        void resetServices() {
            auto gpioProvider = getTestGpioProvider();
            auto styleService = getTestStyleService();
            auto prefService = getTestPreferenceService();
            
            if (gpioProvider) gpioProvider->reset();
            if (styleService) styleService->resetStyles();
            if (prefService) prefService->clearConfig();
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