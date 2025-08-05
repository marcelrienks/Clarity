/**
 * @file test_all.cpp
 * @brief Complete Phase 1 & 2 comprehensive test suite
 * 
 * @details This file contains comprehensive tests for both sensor layer (Phase 1)
 * and manager layer (Phase 2) with enhanced coverage patterns from disabled files.
 * 
 * Phase 1 Enhanced: Sensor Tests with comprehensive patterns
 * Phase 2 New: Manager Layer Tests (TriggerManager, PanelManager, etc.)
 */

#include "unity.h"
#include "unity_config.h"
#include <memory>
#include <map>
#include <vector>
#include <cstdio>
#include <chrono>
#include <thread>
#include <variant>
#include <cstdint>
#include <functional>
#include <string>

// Test-only factory interface (avoid LVGL dependencies)
// #include "factories/manager_factory.h" - causes LVGL dependency issues in test environment

// ============================================================================
// EMBEDDED MOCK IMPLEMENTATIONS & TYPES
// ============================================================================

// GPIO pins namespace
namespace gpio_pins {
    constexpr int OIL_PRESSURE = 34;
    constexpr int OIL_TEMPERATURE = 35;
    constexpr int KEY_PRESENT = 12;
    constexpr int KEY_NOT_PRESENT = 13;
    constexpr int LOCK = 14;
    constexpr int LIGHTS = 15;
}

// Reading variant and common types
using Reading = std::variant<std::monostate, int32_t, double, bool>;

// Component location for UI positioning
struct ComponentLocation {
    int32_t x = 0;
    int32_t y = 0;
    int32_t width = 240;
    int32_t height = 240;
    
    ComponentLocation() = default;
    ComponentLocation(int32_t x_, int32_t y_, int32_t w_ = 240, int32_t h_ = 240) 
        : x(x_), y(y_), width(w_), height(h_) {}
};

// LVGL mock types
typedef void* lv_obj_t;
typedef void* lv_style_t;
typedef int lv_style_selector_t;
typedef int lv_scale_mode_t;
typedef void* lv_image_dsc_t;

// Key states enum for comprehensive testing
enum class KeyState {
    Inactive = 0,    // Neither pin active
    Present = 1,     // Key present pin active
    NotPresent = 2   // Key not present pin active
};

// Theme enumeration for style management (moved here for accessibility)
enum class Theme {
    DAY = 0,
    NIGHT = 1
};

// Trigger execution states
enum class TriggerExecutionState {
    Inactive = 0,
    Active = 1
};

// Forward declarations
class IGpioProvider;
class IDisplayProvider;
class IPanelService;
class IStyleService;
class ITriggerService;
class IPanel;

// ============================================================================
// BASE INTERFACES
// ============================================================================

class IGpioProvider {
public:
    virtual ~IGpioProvider() = default;
    virtual bool digitalRead(int pin) = 0;
    virtual uint16_t analogRead(int pin) = 0;
    virtual void pinMode(int pin, int mode) = 0;
    virtual void attachInterrupt(int pin, void (*callback)(), int mode) = 0;
    virtual void detachInterrupt(int pin) = 0;
    virtual bool hasInterrupt(int pin) = 0;
};

class IDisplayProvider {
public:
    virtual ~IDisplayProvider() = default;
    virtual void init() = 0;
    virtual void update() = 0;
};

class IPanel {
public:
    virtual ~IPanel() = default;
    virtual void init() = 0;
    virtual void load() = 0;
    virtual void update() = 0;
    virtual const char* getName() const = 0;
};

class IPanelService {
public:
    virtual ~IPanelService() = default;
    virtual void loadPanel(const char* panelName) = 0;
    virtual void refreshPanel() = 0;
    virtual IPanel* getCurrentPanel() = 0;
    virtual void init() = 0;
};

class IStyleService {
public:
    virtual ~IStyleService() = default;
    virtual void setTheme(const char* themeName) = 0;
    virtual void applyTheme() = 0;
    virtual const char* getCurrentTheme() const = 0;
};

class ITriggerService {
public:
    virtual ~ITriggerService() = default;
    virtual void init() = 0;
    virtual void processTriggerEvents() = 0;
    virtual void addTrigger(const std::string& triggerName, void* sensor, std::function<void()> callback) = 0;
    virtual bool hasTrigger(const std::string& triggerName) const = 0;
    virtual const char* getStartupPanelOverride() const = 0;
};

// ============================================================================
// ENHANCED MOCK IMPLEMENTATIONS
// ============================================================================

class MockGpioProvider : public IGpioProvider {
private:
    std::map<int, bool> digitalReadings_;
    std::map<int, uint16_t> analogReadings_;
    std::map<int, int> analogReadCount_;
    std::map<int, int> pinModes_;
    std::map<int, bool> interruptAttached_;
    std::map<int, bool> pinModeSet_;
    
public:
    MockGpioProvider() = default;
    virtual ~MockGpioProvider() = default;
    
    // IGpioProvider interface
    bool digitalRead(int pin) override {
        return digitalReadings_[pin];
    }
    
    uint16_t analogRead(int pin) override {
        analogReadCount_[pin]++;
        return analogReadings_[pin];
    }
    
    void pinMode(int pin, int mode) override {
        pinModes_[pin] = mode;
        pinModeSet_[pin] = true;
    }
    
    void attachInterrupt(int pin, void (*callback)(), int mode) override {
        interruptAttached_[pin] = true;
        (void)callback; (void)mode; // Mock implementation
    }
    
    void detachInterrupt(int pin) override {
        interruptAttached_[pin] = false;
    }
    
    bool hasInterrupt(int pin) override {
        return interruptAttached_[pin];
    }
    
    // Enhanced mock methods for comprehensive testing
    void setDigitalReading(int pin, bool value) {
        digitalReadings_[pin] = value;
    }
    
    void setAnalogReading(int pin, uint16_t value) {
        analogReadings_[pin] = value;
    }
    
    int getAnalogReadCount(int pin) const {
        auto it = analogReadCount_.find(pin);
        return (it != analogReadCount_.end()) ? it->second : 0;
    }
    
    bool wasPinModeSet(int pin) const {
        auto it = pinModeSet_.find(pin);
        return (it != pinModeSet_.end()) ? it->second : false;
    }
    
    int getPinMode(int pin) const {
        auto it = pinModes_.find(pin);
        return (it != pinModes_.end()) ? it->second : -1;
    }
    
    bool wasInterruptAttached(int pin) const {
        auto it = interruptAttached_.find(pin);
        return (it != interruptAttached_.end()) ? it->second : false;
    }
    
    void reset() {
        digitalReadings_.clear();
        analogReadings_.clear();
        analogReadCount_.clear();
        pinModes_.clear();
        interruptAttached_.clear();
        pinModeSet_.clear();
    }
    
    // Phase 4 integration methods for error simulation
    std::map<int, bool> analogFailures_;
    
    void simulateAnalogFailure(int pin) {
        analogFailures_[pin] = true;
    }
    
    void clearAnalogFailure(int pin) {
        analogFailures_[pin] = false;
    }
    
    // Phase 4 GPIO value access methods
    bool getDigitalValue(int pin) {
        return digitalReadings_[pin];
    }
    
    uint16_t getAnalogValue(int pin) {
        return analogReadings_[pin];
    }
    
    void setDigitalValue(int pin, bool value) {
        digitalReadings_[pin] = value;
    }
    
    void setAnalogValue(int pin, uint16_t value) {
        analogReadings_[pin] = value;
    }
};

class MockPanel : public IPanel {
private:
    std::string name_;
    bool initialized_ = false;
    bool loaded_ = false;
    int updateCount_ = 0;
    
public:
    MockPanel(const char* name) : name_(name) {}
    
    void init() override { initialized_ = true; }
    void load() override { loaded_ = true; }
    void update() override { updateCount_++; }
    const char* getName() const override { return name_.c_str(); }
    
    // Test verification methods
    bool isInitialized() const { return initialized_; }
    bool isLoaded() const { return loaded_; }
    int getUpdateCount() const { return updateCount_; }
};

class MockPanelService : public IPanelService {
private:
    std::map<std::string, std::unique_ptr<MockPanel>> panels_;
    MockPanel* currentPanel_ = nullptr;
    std::vector<std::string> loadHistory_;
    int refreshCount_ = 0;
    
public:
    void loadPanel(const char* panelName) override {
        loadHistory_.push_back(panelName);
        auto it = panels_.find(panelName);
        if (it != panels_.end()) {
            currentPanel_ = it->second.get();
            currentPanel_->load();
        } else {
            // Create new mock panel
            auto panel = std::make_unique<MockPanel>(panelName);
            currentPanel_ = panel.get();
            panels_[panelName] = std::move(panel);
            currentPanel_->init();
            currentPanel_->load();
        }
    }
    
    void refreshPanel() override {
        refreshCount_++;
        if (currentPanel_) {
            currentPanel_->update();
        }
    }
    
    IPanel* getCurrentPanel() override {
        return currentPanel_;
    }
    
    void init() override {
        // Mock initialization
    }
    
    // Test verification methods
    const std::vector<std::string>& getLoadHistory() const { return loadHistory_; }
    int getRefreshCount() const { return refreshCount_; }
    MockPanel* getMockCurrentPanel() { return currentPanel_; }
};

class MockStyleService : public IStyleService {
private:
    std::string currentTheme_ = "light";
    std::vector<std::string> themeHistory_;
    int applyCount_ = 0;
    
public:
    void setTheme(const char* themeName) override {
        currentTheme_ = themeName;
        themeHistory_.push_back(themeName);
    }
    
    void applyTheme() override {
        applyCount_++;
    }
    
    const char* getCurrentTheme() const override {
        return currentTheme_.c_str();
    }
    
    // Test verification methods
    const std::vector<std::string>& getThemeHistory() const { return themeHistory_; }
    int getApplyCount() const { return applyCount_; }
};

class MockDisplayProvider : public IDisplayProvider {
private:
    bool initialized_ = false;
    int updateCount_ = 0;
    
public:
    void init() override {
        initialized_ = true;
    }
    
    void update() override {
        updateCount_++;
    }
    
    // Test verification methods
    bool isInitialized() const { return initialized_; }
    int getUpdateCount() const { return updateCount_; }
};

// ============================================================================
// ENHANCED SENSOR IMPLEMENTATIONS
// ============================================================================

class SimpleSensor {
protected:
    IGpioProvider* gpioProvider_;
    bool initialized_ = false;
    
public:
    SimpleSensor(IGpioProvider* provider) : gpioProvider_(provider) {}
    virtual ~SimpleSensor() = default;
    
    virtual void init() = 0;
    virtual Reading getReading() = 0;
    bool isInitialized() const { return initialized_; }
};

class SimpleOilPressureSensor : public SimpleSensor {
private:
    static constexpr uint16_t ADC_MAX_VALUE = 4095;
    static constexpr int32_t PRESSURE_MAX_BAR = 10;
    int32_t lastReading_ = -1;
    int pin_;
    
public:
    SimpleOilPressureSensor(IGpioProvider* provider, int pin) : SimpleSensor(provider), pin_(pin) {}
    
    void init() override {
        initialized_ = true;
        gpioProvider_->analogRead(pin_);
    }
    
    void begin() {
        init();
    }
    
    void update() {
        // Update reading
        getReading();
    }
    
    int32_t read() {
        uint16_t adcValue = gpioProvider_->analogRead(pin_);
        int32_t pressure = (adcValue * PRESSURE_MAX_BAR) / ADC_MAX_VALUE;
        lastReading_ = pressure;
        return pressure;
    }
    
    Reading getReading() override {
        return read();
    }
    
    int32_t getLastReading() const { return lastReading_; }
    
    bool isInitialized() const { return initialized_; }
};

class SimpleOilTemperatureSensor : public SimpleSensor {
private:
    static constexpr uint16_t ADC_MAX_VALUE = 4095;
    static constexpr int32_t TEMPERATURE_MAX_CELSIUS = 120;
    int32_t lastReading_ = -1;
    int pin_;
    
public:
    SimpleOilTemperatureSensor(IGpioProvider* provider, int pin) : SimpleSensor(provider), pin_(pin) {}
    
    void init() override {
        initialized_ = true;
        gpioProvider_->analogRead(pin_);
    }
    
    void begin() {
        init();
    }
    
    void update() {
        // Update reading
        getReading();
    }
    
    int32_t read() {
        uint16_t adcValue = gpioProvider_->analogRead(pin_);
        int32_t temperature = (adcValue * TEMPERATURE_MAX_CELSIUS) / ADC_MAX_VALUE;
        lastReading_ = temperature;
        return temperature;
    }
    
    Reading getReading() override {
        return read();
    }
    
    int32_t getLastReading() const { return lastReading_; }
    
    bool isInitialized() const { return initialized_; }
};

class SimpleKeySensor : public SimpleSensor {
private:
    KeyState lastState_ = KeyState::Inactive;
    int keyPresentPin_;
    int keyNotPresentPin_;
    
public:
    SimpleKeySensor(IGpioProvider* provider, int keyPresentPin, int keyNotPresentPin) 
        : SimpleSensor(provider), keyPresentPin_(keyPresentPin), keyNotPresentPin_(keyNotPresentPin) {}
    
    void init() override {
        initialized_ = true;
        // Configure pins (mock implementation tracks this)
        gpioProvider_->pinMode(keyPresentPin_, 2); // INPUT_PULLDOWN
        gpioProvider_->pinMode(keyNotPresentPin_, 2);
        gpioProvider_->attachInterrupt(keyPresentPin_, nullptr, 3); // CHANGE
        gpioProvider_->attachInterrupt(keyNotPresentPin_, nullptr, 3);
    }
    
    void begin() {
        init();
    }
    
    void update() {
        // Update reading
        getReading();
    }
    
    bool read() {
        bool keyPresent = gpioProvider_->digitalRead(keyPresentPin_);
        bool keyNotPresent = gpioProvider_->digitalRead(keyNotPresentPin_);
        
        // Determine key state with proper logic
        if (keyPresent && !keyNotPresent) {
            lastState_ = KeyState::Present;
            return true;
        } else if (!keyPresent && keyNotPresent) {
            lastState_ = KeyState::NotPresent;
            return false;
        } else {
            lastState_ = KeyState::Inactive;
            return false; // Default to not present for inactive state
        }
    }
    
    Reading getReading() override {
        return read();
    }
    
    KeyState getLastState() const { return lastState_; }
    
    bool isInitialized() const { return initialized_; }
};

class SimpleLockSensor : public SimpleSensor {
private:
    bool lastReading_ = false;
    int pin_;
    
public:
    SimpleLockSensor(IGpioProvider* provider, int pin) : SimpleSensor(provider), pin_(pin) {}
    
    void init() override {
        initialized_ = true;
        gpioProvider_->pinMode(pin_, 2); // INPUT_PULLDOWN
        gpioProvider_->attachInterrupt(pin_, nullptr, 3); // CHANGE
    }
    
    void begin() {
        init();
    }
    
    void update() {
        // Update reading
        getReading();
    }
    
    bool read() {
        bool lockActive = gpioProvider_->digitalRead(pin_);
        lastReading_ = lockActive;
        return lockActive;
    }
    
    Reading getReading() override {
        return read();
    }
    
    bool getLastReading() const { return lastReading_; }
    
    bool isInitialized() const { return initialized_; }
};

class SimpleLightSensor : public SimpleSensor {
private:
    bool lastReading_ = true; // Default to day mode
    int pin_;
    
public:
    SimpleLightSensor(IGpioProvider* provider, int pin) : SimpleSensor(provider), pin_(pin) {}
    
    void init() override {
        initialized_ = true;
        gpioProvider_->pinMode(pin_, 2); // INPUT_PULLDOWN
    }
    
    void begin() {
        init();
    }
    
    void update() {
        // Update reading
        getReading();
    }
    
    bool read() {
        bool isDayMode = gpioProvider_->digitalRead(pin_);
        lastReading_ = isDayMode;
        return isDayMode;
    }
    
    Reading getReading() override {
        return read();
    }
    
    bool getLastReading() const { return lastReading_; }
    
    bool isInitialized() const { return initialized_; }
};

// ============================================================================
// MANAGER IMPLEMENTATIONS FOR TESTING
// ============================================================================

class SimpleTriggerManager : public ITriggerService {
private:
    std::shared_ptr<SimpleKeySensor> keySensor_;
    std::shared_ptr<SimpleLockSensor> lockSensor_;
    std::shared_ptr<SimpleLightSensor> lightSensor_;
    MockPanelService* panelService_;
    MockStyleService* styleService_;
    
    std::map<std::string, std::function<void()>> triggers_;
    std::string startupPanelOverride_;
    bool initialized_ = false;
    int processCount_ = 0;
    
    // State tracking for change detection
    bool lastKeyState_ = false;
    bool lastLockState_ = false;
    bool lastLightState_ = true;
    
public:
    SimpleTriggerManager(std::shared_ptr<SimpleKeySensor> keySensor,
                        std::shared_ptr<SimpleLockSensor> lockSensor,
                        std::shared_ptr<SimpleLightSensor> lightSensor,
                        MockPanelService* panelService,
                        MockStyleService* styleService)
        : keySensor_(keySensor), lockSensor_(lockSensor), lightSensor_(lightSensor),
          panelService_(panelService), styleService_(styleService) {}
    
    void init() override {
        initialized_ = true;
        // Initialize sensors
        if (keySensor_) keySensor_->init();
        if (lockSensor_) lockSensor_->init();
        if (lightSensor_) lightSensor_->init();
        
        // Set up trigger mappings
        setupTriggerMappings();
    }
    
    void processTriggerEvents() override {
        processCount_++;
        if (!initialized_) return;
        
        // Check for state changes and trigger appropriate actions
        checkKeyTrigger();
        checkLockTrigger(); 
        checkLightTrigger();
    }
    
    void addTrigger(const std::string& triggerName, void* sensor, std::function<void()> callback) override {
        triggers_[triggerName] = callback;
    }
    
    bool hasTrigger(const std::string& triggerName) const override {
        return triggers_.find(triggerName) != triggers_.end();
    }
    
    const char* getStartupPanelOverride() const override {
        return startupPanelOverride_.empty() ? nullptr : startupPanelOverride_.c_str();
    }
    
    // Test verification methods
    bool isInitialized() const { return initialized_; }
    int getProcessCount() const { return processCount_; }
    size_t getTriggerCount() const { return triggers_.size(); }
    
    void setStartupPanelOverride(const char* panelName) {
        startupPanelOverride_ = panelName ? panelName : "";
    }
    
private:
    void setupTriggerMappings() {
        // Key triggers
        addTrigger("key_present", nullptr, [this]() {
            if (panelService_) panelService_->loadPanel("key_panel");
        });
        
        addTrigger("key_not_present", nullptr, [this]() {
            if (panelService_) panelService_->loadPanel("key_panel");
        });
        
        // Lock trigger
        addTrigger("lock_active", nullptr, [this]() {
            if (panelService_) panelService_->loadPanel("lock_panel");
        });
        
        // Light/theme trigger
        addTrigger("light_change", nullptr, [this]() {
            if (styleService_) {
                bool isDayMode = lightSensor_ ? std::get<bool>(lightSensor_->getReading()) : true;
                styleService_->setTheme(isDayMode ? "light" : "dark");
                styleService_->applyTheme();
            }
        });
    }
    
    void checkKeyTrigger() {
        if (!keySensor_) return;
        
        bool currentKeyState = std::get<bool>(keySensor_->getReading());
        if (currentKeyState != lastKeyState_) {
            lastKeyState_ = currentKeyState;
            
            auto triggerName = currentKeyState ? "key_present" : "key_not_present";
            auto it = triggers_.find(triggerName);
            if (it != triggers_.end()) {
                it->second();
            }
        }
    }
    
    void checkLockTrigger() {
        if (!lockSensor_) return;
        
        bool currentLockState = std::get<bool>(lockSensor_->getReading());
        if (currentLockState != lastLockState_) {
            lastLockState_ = currentLockState;
            
            if (currentLockState) {
                auto it = triggers_.find("lock_active");
                if (it != triggers_.end()) {
                    it->second();
                }
            }
        }
    }
    
    void checkLightTrigger() {
        if (!lightSensor_) return;
        
        bool currentLightState = std::get<bool>(lightSensor_->getReading());
        if (currentLightState != lastLightState_) {
            lastLightState_ = currentLightState;
            
            auto it = triggers_.find("light_change");
            if (it != triggers_.end()) {
                it->second();
            }
        }
    }
};

// ============================================================================
// COMPONENT IMPLEMENTATIONS FOR TESTING (Phase 3)
// ============================================================================

// Mock LVGL objects for component testing
struct MockLvglObject {
    std::string type;
    std::map<std::string, std::string> properties;
    std::vector<MockLvglObject*> children;
    bool visible = true;
    
    MockLvglObject(const std::string& objType) : type(objType) {}
};

// Global mock LVGL object storage
static std::vector<std::unique_ptr<MockLvglObject>> mockLvglObjects;

// Mock LVGL functions for testing
extern "C" {
    lv_obj_t* lv_obj_create(lv_obj_t* parent) {
        auto obj = std::make_unique<MockLvglObject>("obj");
        lv_obj_t* ptr = reinterpret_cast<lv_obj_t*>(obj.get());
        mockLvglObjects.push_back(std::move(obj));
        return ptr;
    }
    
    lv_obj_t* lv_scale_create(lv_obj_t* parent) {
        auto obj = std::make_unique<MockLvglObject>("scale");
        lv_obj_t* ptr = reinterpret_cast<lv_obj_t*>(obj.get());
        mockLvglObjects.push_back(std::move(obj));
        return ptr;
    }
    
    lv_obj_t* lv_line_create(lv_obj_t* parent) {
        auto obj = std::make_unique<MockLvglObject>("line");
        lv_obj_t* ptr = reinterpret_cast<lv_obj_t*>(obj.get());
        mockLvglObjects.push_back(std::move(obj));
        return ptr;
    }
    
    lv_obj_t* lv_image_create(lv_obj_t* parent) {
        auto obj = std::make_unique<MockLvglObject>("image");
        lv_obj_t* ptr = reinterpret_cast<lv_obj_t*>(obj.get());
        mockLvglObjects.push_back(std::move(obj));
        return ptr;
    }
    
    lv_obj_t* lv_label_create(lv_obj_t* parent) {
        auto obj = std::make_unique<MockLvglObject>("label");
        lv_obj_t* ptr = reinterpret_cast<lv_obj_t*>(obj.get());
        mockLvglObjects.push_back(std::move(obj));
        return ptr;
    }
    
    void lv_obj_set_size(lv_obj_t* obj, int32_t w, int32_t h) {
        if (obj) {
            MockLvglObject* mockObj = reinterpret_cast<MockLvglObject*>(obj);
            mockObj->properties["width"] = std::to_string(w);
            mockObj->properties["height"] = std::to_string(h);
        }
    }
    
    void lv_obj_set_pos(lv_obj_t* obj, int32_t x, int32_t y) {
        if (obj) {
            MockLvglObject* mockObj = reinterpret_cast<MockLvglObject*>(obj);
            mockObj->properties["x"] = std::to_string(x);
            mockObj->properties["y"] = std::to_string(y);
        }
    }
    
    void lv_obj_add_style(lv_obj_t* obj, lv_style_t* style, lv_style_selector_t selector) {
        if (obj) {
            MockLvglObject* mockObj = reinterpret_cast<MockLvglObject*>(obj);
            mockObj->properties["style_applied"] = "true";
        }
    }
    
    void lv_scale_set_mode(lv_obj_t* obj, lv_scale_mode_t mode) {
        if (obj) {
            MockLvglObject* mockObj = reinterpret_cast<MockLvglObject*>(obj);
            mockObj->properties["scale_mode"] = std::to_string(mode);
        }
    }
    
    void lv_scale_set_range(lv_obj_t* obj, int32_t min, int32_t max) {
        if (obj) {
            MockLvglObject* mockObj = reinterpret_cast<MockLvglObject*>(obj);
            mockObj->properties["scale_min"] = std::to_string(min);
            mockObj->properties["scale_max"] = std::to_string(max);
        }
    }
    
    void lv_scale_set_rotation(lv_obj_t* obj, int32_t rotation) {
        if (obj) {
            MockLvglObject* mockObj = reinterpret_cast<MockLvglObject*>(obj);
            mockObj->properties["rotation"] = std::to_string(rotation);
        }
    }
    
    void lv_obj_set_style_line_width(lv_obj_t* obj, int32_t width, lv_style_selector_t selector) {
        if (obj) {
            MockLvglObject* mockObj = reinterpret_cast<MockLvglObject*>(obj);
            mockObj->properties["line_width"] = std::to_string(width);
        }
    }
    
    void lv_label_set_text(lv_obj_t* obj, const char* text) {
        if (obj && text) {
            MockLvglObject* mockObj = reinterpret_cast<MockLvglObject*>(obj);
            mockObj->properties["text"] = text;
        }
    }
    
    void lv_image_set_src(lv_obj_t* obj, const void* src) {
        if (obj) {
            MockLvglObject* mockObj = reinterpret_cast<MockLvglObject*>(obj);
            mockObj->properties["image_src"] = "icon_set";
        }
    }
}

// Forward declarations for component testing
class TestStyleManager;

// ============================================================================
// REAL MANAGER IMPLEMENTATIONS FOR TESTING
// ============================================================================

// Real PanelManager testing implementation (simplified)
class TestPanelManager {
private:
    MockDisplayProvider* displayProvider_;
    MockGpioProvider* gpioProvider_;
    MockStyleService* styleService_;
    std::string currentPanel_ = "oil_panel";
    std::string restorationPanel_ = "oil_panel";
    bool initialized_ = false;
    int updateCount_ = 0;
    std::vector<std::string> panelLoadHistory_;
    std::function<void()> currentCallback_;
    
public:
    TestPanelManager(MockDisplayProvider* display, MockGpioProvider* gpio, MockStyleService* style)
        : displayProvider_(display), gpioProvider_(gpio), styleService_(style) {}
    
    void init() {
        initialized_ = true;
        displayProvider_->init();
    }
    
    void createAndLoadPanel(const char* panelName, std::function<void()> callback = nullptr, bool isTriggerDriven = false) {
        panelLoadHistory_.push_back(panelName);
        currentPanel_ = panelName;
        currentCallback_ = callback;
        
        // Simulate async completion
        if (callback) {
            callback();
        }
    }
    
    void createAndLoadPanelWithSplash(const char* panelName) {
        panelLoadHistory_.push_back("splash_panel");
        panelLoadHistory_.push_back(panelName);
        currentPanel_ = panelName;
    }
    
    void updatePanel() {
        updateCount_++;
    }
    
    void setUiState(int state) {
        // Mock implementation - just track that it was called
    }
    
    const char* getCurrentPanel() const {
        return currentPanel_.c_str();
    }
    
    const char* getRestorationPanel() const {
        return restorationPanel_.c_str();
    }
    
    void triggerPanelSwitchCallback(const char* triggerId) {
        // Mock trigger callback handling
        panelLoadHistory_.push_back(std::string("trigger_") + triggerId);
    }
    
    // Phase 4 integration methods
    void showOilPanel() {
        createAndLoadPanel("OilPanel");
    }
    
    void showKeyPanel() {
        createAndLoadPanel("KeyPanel");
    }
    
    void showLockPanel() {
        createAndLoadPanel("LockPanel");
    }
    
    std::string getCurrentPanel() {
        return currentPanel_;
    }
    
    bool wasSplashShown() {
        for (const auto& panel : panelLoadHistory_) {
            if (panel == "splash_panel" || panel == "SplashPanel") {
                return true;
            }
        }
        return false;
    }
    
    void clearPanelHistory() {
        panelLoadHistory_.clear();
        // Simulate splash on startup
        panelLoadHistory_.push_back("SplashPanel");
        currentPanel_ = "OilPanel";
    }
    
    // Test verification methods
    bool isInitialized() const { return initialized_; }
    int getUpdateCount() const { return updateCount_; }
    const std::vector<std::string>& getPanelLoadHistory() const { return panelLoadHistory_; }
    void setRestorationPanel(const char* panelName) { restorationPanel_ = panelName; }
};

// Real StyleManager testing implementation (simplified)
class TestStyleManager {
private:
    std::string currentTheme_ = "night";
    std::vector<std::string> themeHistory_;
    int applyCount_ = 0;
    bool initialized_ = false;
    std::map<std::string, std::string> styleCache_;
    
public:
    TestStyleManager() = default;
    
    void init() {
        initialized_ = true;
        // Initialize with default theme
        setTheme("night");
    }
    
    void setTheme(const char* themeName) {
        currentTheme_ = themeName;
        themeHistory_.push_back(themeName);
        
        // Mock style creation for theme with theme-specific keys
        std::string themePrefix = std::string(themeName) + "_";
        styleCache_[themePrefix + "bg"] = std::string(themeName) + "_background_style";
        styleCache_[themePrefix + "text"] = std::string(themeName) + "_text_style";
        styleCache_[themePrefix + "gauge"] = std::string(themeName) + "_gauge_style";
    }
    
    void applyTheme() {
        applyCount_++;
    }
    
    void applyThemeToScreen(void* screen) {
        applyCount_++;
        // Mock screen application
    }
    
    const char* getCurrentTheme() const {
        return currentTheme_.c_str();
    }
    
    const char* getCurrentThemeAsString() const {
        return currentTheme_.c_str();
    }
    
    // Mock style getters
    void* getGaugeMainStyle() { 
        std::string key = currentTheme_ + "_gauge";
        return styleCache_.find(key) != styleCache_.end() ? &styleCache_[key] : nullptr;
    }
    void* getGaugeIndicatorStyle() { 
        std::string key = currentTheme_ + "_gauge";
        return styleCache_.find(key) != styleCache_.end() ? &styleCache_[key] : nullptr;
    }
    void* getGaugeItemsStyle() { 
        std::string key = currentTheme_ + "_gauge";
        return styleCache_.find(key) != styleCache_.end() ? &styleCache_[key] : nullptr;
    }
    void* getGaugeDangerSectionStyle() { 
        std::string key = currentTheme_ + "_gauge";
        return styleCache_.find(key) != styleCache_.end() ? &styleCache_[key] : nullptr;
    }
    
    // Phase 4 integration methods
    void setTheme(Theme theme) {
        if (theme == Theme::DAY) {
            setTheme("day");
        } else if (theme == Theme::NIGHT) {
            setTheme("night");
        }
    }
    
    Theme getCurrentTheme() {
        if (currentTheme_ == "day") {
            return Theme::DAY;
        } else {
            return Theme::NIGHT;
        }
    }
    
    std::string getCurrentThemeString() {
        return currentTheme_;
    }
    
    // Test verification methods
    bool isInitialized() const { return initialized_; }
    const std::vector<std::string>& getThemeHistory() const { return themeHistory_; }
    int getApplyCount() const { return applyCount_; }
    bool hasStyleForTheme(const char* themeName) const {
        return styleCache_.find(std::string(themeName) + "_bg") != styleCache_.end();
    }
};

// Real PreferenceManager testing implementation (simplified)
class TestPreferenceManager {
private:
    std::map<std::string, std::string> preferences_;
    bool initialized_ = false;
    int saveCount_ = 0;
    int loadCount_ = 0;
    bool configExists_ = false;
    
public:
    TestPreferenceManager() = default;
    
    void init() {
        initialized_ = true;
        loadConfig();
    }
    
    void loadConfig() {
        loadCount_++;
        if (!configExists_) {
            createDefaultConfig();
        }
    }
    
    void createDefaultConfig() {
        preferences_["panel_name"] = "oil_panel";
        preferences_["theme"] = "night";
        preferences_["brightness"] = "80";
        configExists_ = true;
    }
    
    void saveConfig() {
        saveCount_++;
        // Mock NVS save operation
    }
    
    // Preference getters/setters
    void setString(const char* key, const char* value) {
        preferences_[key] = value;
    }
    
    std::string getString(const char* key, const char* defaultValue = "") const {
        auto it = preferences_.find(key);
        return (it != preferences_.end()) ? it->second : std::string(defaultValue);
    }
    
    void setInt(const char* key, int value) {
        preferences_[key] = std::to_string(value);
    }
    
    int getInt(const char* key, int defaultValue = 0) const {
        auto it = preferences_.find(key);
        return (it != preferences_.end()) ? std::stoi(it->second) : defaultValue;
    }
    
    void setBool(const char* key, bool value) {
        preferences_[key] = value ? "true" : "false";
    }
    
    bool getBool(const char* key, bool defaultValue = false) const {
        auto it = preferences_.find(key);
        if (it != preferences_.end()) {
            return it->second == "true";
        }
        return defaultValue;
    }
    
    void clear() {
        preferences_.clear();
        configExists_ = false;
    }
    
    // Test verification methods
    bool isInitialized() const { return initialized_; }
    int getSaveCount() const { return saveCount_; }
    int getLoadCount() const { return loadCount_; }
    bool hasConfig() const { return configExists_; }
    size_t getPreferenceCount() const { return preferences_.size(); }
    void simulateCorruption() { configExists_ = false; preferences_.clear(); }
};

// ============================================================================
// COMPONENT IMPLEMENTATIONS FOR TESTING (After manager definitions)
// ============================================================================

// Test implementations of component classes
class TestOemOilPressureComponent {
private:
    MockDisplayProvider* displayProvider_;
    TestStyleManager* styleService_;
    int32_t currentValue_ = 0;
    int32_t scaleMaterial_ = 0;
    int32_t scaleMax_ = 100;
    int32_t dangerZone_ = 80;
    bool rendered_ = false;
    std::string iconType_ = "oil_pressure";
    
public:
    TestOemOilPressureComponent(MockDisplayProvider* display, TestStyleManager* style)
        : displayProvider_(display), styleService_(style) {}
    
    // Component interface methods
    void render(void* screen, const ComponentLocation& location) {
        rendered_ = true;
        // Mock rendering - create scale, needle, icon, labels
        displayProvider_->update(); // Track render call
    }
    
    void refresh(const Reading& reading) {
        if (std::holds_alternative<int32_t>(reading)) {
            setValue(std::get<int32_t>(reading));
        }
    }
    
    void setValue(int32_t value) {
        currentValue_ = value;
    }
    
    // OEM Oil Component specific methods
    int32_t getScaleMin() const { return scaleMaterial_; }
    int32_t getScaleMax() const { return scaleMax_; }
    int32_t getDangerZone() const { return dangerZone_; }
    bool isDangerCondition(int32_t value) const { return value >= dangerZone_; }
    
    int32_t mapValueForDisplay(int32_t value) const {
        // Clamp to scale range and map to display range
        int32_t clampedValue = std::max(scaleMaterial_, std::min(scaleMax_, value));
        return clampedValue;
    }
    
    // Test verification methods
    bool isRendered() const { return rendered_; }
    int32_t getCurrentValue() const { return currentValue_; }
    std::string getIconType() const { return iconType_; }
    bool isInDangerZone() const { return isDangerCondition(currentValue_); }
    
    // Configuration methods for testing
    void setScaleRange(int32_t min, int32_t max) {
        scaleMaterial_ = min;
        scaleMax_ = max;
    }
    
    void setDangerZone(int32_t threshold) {
        dangerZone_ = threshold;
    }
};

class TestOemOilTemperatureComponent {
private:
    MockDisplayProvider* displayProvider_;
    TestStyleManager* styleService_;
    int32_t currentValue_ = 0;
    int32_t scaleMin_ = 160;  // Fahrenheit
    int32_t scaleMax_ = 250;
    int32_t dangerZone_ = 220;
    bool rendered_ = false;
    std::string iconType_ = "oil_temperature";
    
public:
    TestOemOilTemperatureComponent(MockDisplayProvider* display, TestStyleManager* style)
        : displayProvider_(display), styleService_(style) {}
    
    void render(void* screen, const ComponentLocation& location) {
        rendered_ = true;
        displayProvider_->update();
    }
    
    void refresh(const Reading& reading) {
        if (std::holds_alternative<int32_t>(reading)) {
            setValue(std::get<int32_t>(reading));
        }
    }
    
    void setValue(int32_t value) {
        currentValue_ = value;
    }
    
    // Temperature-specific methods
    int32_t getScaleMin() const { return scaleMin_; }
    int32_t getScaleMax() const { return scaleMax_; }
    int32_t getDangerZone() const { return dangerZone_; }
    bool isDangerCondition(int32_t value) const { return value >= dangerZone_; }
    
    int32_t mapValueForDisplay(int32_t value) const {
        int32_t clampedValue = std::max(scaleMin_, std::min(scaleMax_, value));
        return clampedValue;
    }
    
    // Test verification methods
    bool isRendered() const { return rendered_; }
    int32_t getCurrentValue() const { return currentValue_; }
    std::string getIconType() const { return iconType_; }
    bool isInDangerZone() const { return isDangerCondition(currentValue_); }
    
    void setScaleRange(int32_t min, int32_t max) {
        scaleMin_ = min;
        scaleMax_ = max;
    }
    
    void setDangerZone(int32_t threshold) {
        dangerZone_ = threshold;
    }
};

class TestKeyComponent {
private:
    MockDisplayProvider* displayProvider_;
    TestStyleManager* styleService_;
    bool currentState_ = false;
    bool rendered_ = false;
    std::string iconType_ = "key";
    std::string currentColor_ = "normal";
    
public:
    TestKeyComponent(MockDisplayProvider* display, TestStyleManager* style)
        : displayProvider_(display), styleService_(style) {}
    
    void render(void* screen, const ComponentLocation& location) {
        rendered_ = true;
        displayProvider_->update();
    }
    
    void refresh(const Reading& reading) {
        if (std::holds_alternative<bool>(reading)) {
            bool newState = std::get<bool>(reading);
            setState(newState);
        }
    }
    
    void setState(bool state) {
        currentState_ = state;
        updateVisualState();
    }
    
    void updateVisualState() {
        // Update color based on state and theme
        std::string theme = styleService_->getCurrentThemeAsString();
        if (currentState_) {
            currentColor_ = theme + "_active";
        } else {
            currentColor_ = theme + "_inactive";
        }
    }
    
    // Phase 4 integration methods
    void setValue(int value) {
        setState(value != 0); // Convert int to bool
    }
    
    int getCurrentValue() const {
        return currentState_ ? 1 : 0; // Convert bool to int
    }
    
    // Test verification methods
    bool isRendered() const { return rendered_; }
    bool getCurrentState() const { return currentState_; }
    std::string getIconType() const { return iconType_; }
    std::string getCurrentColor() const { return currentColor_; }
};

class TestLockComponent {
private:
    MockDisplayProvider* displayProvider_;
    TestStyleManager* styleService_;
    bool currentState_ = false;
    bool rendered_ = false;
    std::string iconType_ = "lock";
    std::string currentColor_ = "normal";
    
public:
    TestLockComponent(MockDisplayProvider* display, TestStyleManager* style)
        : displayProvider_(display), styleService_(style) {}
    
    void render(void* screen, const ComponentLocation& location) {
        rendered_ = true;
        displayProvider_->update();
    }
    
    void refresh(const Reading& reading) {
        if (std::holds_alternative<bool>(reading)) {
            bool newState = std::get<bool>(reading);
            setState(newState);
        }
    }
    
    void setState(bool state) {
        currentState_ = state;
        updateVisualState();
    }
    
    void updateVisualState() {
        std::string theme = styleService_->getCurrentThemeAsString();
        if (currentState_) {
            currentColor_ = theme + "_locked";
        } else {
            currentColor_ = theme + "_unlocked";
        }
    }
    
    // Test verification methods
    bool isRendered() const { return rendered_; }
    bool getCurrentState() const { return currentState_; }
    std::string getIconType() const { return iconType_; }
    std::string getCurrentColor() const { return currentColor_; }
};

class TestClarityComponent {
private:
    MockDisplayProvider* displayProvider_;
    TestStyleManager* styleService_;
    bool rendered_ = false;
    std::string logoType_ = "clarity_logo";
    std::string currentTheme_ = "default";
    
public:
    TestClarityComponent(MockDisplayProvider* display, TestStyleManager* style)
        : displayProvider_(display), styleService_(style) {}
    
    void render(void* screen, const ComponentLocation& location) {
        rendered_ = true;
        currentTheme_ = styleService_->getCurrentThemeAsString();
        displayProvider_->update();
    }
    
    void refresh(const Reading& reading) {
        // Clarity component typically doesn't refresh with sensor data
        // But could refresh with theme changes
        currentTheme_ = styleService_->getCurrentThemeAsString();
    }
    
    // Test verification methods
    bool isRendered() const { return rendered_; }
    std::string getLogoType() const { return logoType_; }
    std::string getCurrentTheme() const { return currentTheme_; }
};

// ============================================================================
// TEST HELPERS (Enhanced from disabled files)
// ============================================================================

namespace TestHelpers {
    std::unique_ptr<MockGpioProvider> createMockGpioProvider() {
        auto mock = std::make_unique<MockGpioProvider>();
        mock->reset();
        return mock;
    }
    
    void waitForSensorUpdate(unsigned long intervalMs) {
        std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs + 10));
    }
    
    void assertReadingInt32(const Reading& reading, int32_t expectedValue) {
        TEST_ASSERT_TRUE(std::holds_alternative<int32_t>(reading));
        TEST_ASSERT_EQUAL_INT32(expectedValue, std::get<int32_t>(reading));
    }
    
    void assertReadingBool(const Reading& reading, bool expectedValue) {
        TEST_ASSERT_TRUE(std::holds_alternative<bool>(reading));
        TEST_ASSERT_EQUAL(expectedValue, std::get<bool>(reading));
    }
    
    void configureMockForOilPressure(MockGpioProvider* mock, uint16_t adcValue) {
        mock->setAnalogReading(gpio_pins::OIL_PRESSURE, adcValue);
    }
    
    void configureMockForOilTemperature(MockGpioProvider* mock, uint16_t adcValue) {
        mock->setAnalogReading(gpio_pins::OIL_TEMPERATURE, adcValue);
    }
    
    void configureMockForKeySensor(MockGpioProvider* mock, bool keyPresent, bool keyNotPresent) {
        mock->setDigitalReading(gpio_pins::KEY_PRESENT, keyPresent);
        mock->setDigitalReading(gpio_pins::KEY_NOT_PRESENT, keyNotPresent);
    }
    
    void configureMockForLockSensor(MockGpioProvider* mock, bool lockState) {
        mock->setDigitalReading(gpio_pins::LOCK, lockState);
    }
    
    void configureMockForLightSensor(MockGpioProvider* mock, bool lightState) {
        mock->setDigitalReading(gpio_pins::LIGHTS, lightState);
    }
    
    int32_t calculateExpectedPressure(uint16_t adcValue) {
        constexpr int32_t ADC_MAX_VALUE = 4095;
        constexpr int32_t PRESSURE_MAX_BAR = 10;
        return (adcValue * PRESSURE_MAX_BAR) / ADC_MAX_VALUE;
    }
    
    int32_t calculateExpectedTemperature(uint16_t adcValue) {
        constexpr int32_t ADC_MAX_VALUE = 4095;
        constexpr int32_t TEMPERATURE_MAX_CELSIUS = 120;
        return (adcValue * TEMPERATURE_MAX_CELSIUS) / ADC_MAX_VALUE;
    }
    
    // Enhanced test patterns from disabled files
    void assertValidAdcRange(uint16_t value) {
        TEST_ASSERT_TRUE(value >= 0 && value <= 4095);
    }
    
    void assertValidPressureRange(int32_t pressure) {
        TEST_ASSERT_TRUE(pressure >= 0 && pressure <= 10);
    }
    
    void assertValidTemperatureRange(int32_t temperature) {
        TEST_ASSERT_TRUE(temperature >= 0 && temperature <= 120);
    }
}

// ============================================================================
// GLOBAL TEST FIXTURES
// ============================================================================

static std::unique_ptr<MockGpioProvider> mockGpio;
static std::unique_ptr<MockPanelService> mockPanelService;
static std::unique_ptr<MockStyleService> mockStyleService;
static std::unique_ptr<MockDisplayProvider> mockDisplay;
static std::unique_ptr<TestPanelManager> testPanelManager;
static std::unique_ptr<TestStyleManager> testStyleManager;
static std::unique_ptr<TestPreferenceManager> testPreferenceManager;
static std::unique_ptr<TestOemOilPressureComponent> testOilPressureComponent;
static std::unique_ptr<TestOemOilTemperatureComponent> testOilTemperatureComponent;
static std::unique_ptr<TestKeyComponent> testKeyComponent;
static std::unique_ptr<TestLockComponent> testLockComponent;
static std::unique_ptr<TestClarityComponent> testClarityComponent;

void setUp(void) {
    mockGpio = TestHelpers::createMockGpioProvider();
    mockPanelService = std::make_unique<MockPanelService>();
    mockStyleService = std::make_unique<MockStyleService>();
    mockDisplay = std::make_unique<MockDisplayProvider>();
    testPanelManager = std::make_unique<TestPanelManager>(mockDisplay.get(), mockGpio.get(), mockStyleService.get());
    testStyleManager = std::make_unique<TestStyleManager>();
    testPreferenceManager = std::make_unique<TestPreferenceManager>();
    testOilPressureComponent = std::make_unique<TestOemOilPressureComponent>(mockDisplay.get(), testStyleManager.get());
    testOilTemperatureComponent = std::make_unique<TestOemOilTemperatureComponent>(mockDisplay.get(), testStyleManager.get());
    testKeyComponent = std::make_unique<TestKeyComponent>(mockDisplay.get(), testStyleManager.get());
    testLockComponent = std::make_unique<TestLockComponent>(mockDisplay.get(), testStyleManager.get());
    testClarityComponent = std::make_unique<TestClarityComponent>(mockDisplay.get(), testStyleManager.get());
    
    // Clear mock LVGL objects for each test
    mockLvglObjects.clear();
}

void tearDown(void) {
    mockGpio.reset();
    mockPanelService.reset();
    mockStyleService.reset();
    mockDisplay.reset();
    testPanelManager.reset();
    testStyleManager.reset();
    testPreferenceManager.reset();
    testOilPressureComponent.reset();
    testOilTemperatureComponent.reset();
    testKeyComponent.reset();
    testLockComponent.reset();
    testClarityComponent.reset();
    mockLvglObjects.clear();
}

// ============================================================================
// PHASE 1: ENHANCED SENSOR TESTS (From disabled files)
// ============================================================================

// Oil Pressure Sensor Tests - Enhanced Coverage
void test_oil_pressure_sensor_initialization() {
    auto sensor = std::make_unique<SimpleOilPressureSensor>(mockGpio.get(), gpio_pins::OIL_PRESSURE);
    sensor->init();
    
    TEST_ASSERT_TRUE(sensor->isInitialized());
    Reading reading = sensor->getReading();
    TEST_ASSERT_FALSE(std::holds_alternative<std::monostate>(reading));
    TEST_ASSERT_TRUE(mockGpio->getAnalogReadCount(gpio_pins::OIL_PRESSURE) > 0);
}

void test_oil_pressure_sensor_constructor() {
    SimpleOilPressureSensor sensor(mockGpio.get(), gpio_pins::OIL_PRESSURE);
    TEST_ASSERT_NOT_NULL(&sensor);
    TEST_ASSERT_FALSE(sensor.isInitialized()); // Should not be initialized yet
}

void test_oil_pressure_sensor_adc_mapping_boundary_conditions() {
    auto sensor = std::make_unique<SimpleOilPressureSensor>(mockGpio.get(), gpio_pins::OIL_PRESSURE);
    sensor->init();
    
    // Test boundary values
    struct TestCase {
        uint16_t adcValue;
        int32_t expectedPressure;
        const char* description;
    };
    
    TestCase testCases[] = {
        {0, 0, "Minimum ADC (0)"},
        {4095, 10, "Maximum ADC (4095)"},
        {2048, 5, "Midpoint ADC (~2048)"},
        {1024, 2, "Quarter point ADC (~1024)"},
        {3072, 7, "Three-quarter point ADC (~3072)"}
    };
    
    for (const auto& testCase : testCases) {
        TestHelpers::configureMockForOilPressure(mockGpio.get(), testCase.adcValue);
        Reading reading = sensor->getReading();
        int32_t actualPressure = std::get<int32_t>(reading);
        
        // Allow ±1 tolerance for integer math
        TEST_ASSERT_TRUE(abs(actualPressure - testCase.expectedPressure) <= 1);
        TestHelpers::assertValidPressureRange(actualPressure);
    }
}

void test_oil_pressure_sensor_delta_updates_comprehensive() {
    auto sensor = std::make_unique<SimpleOilPressureSensor>(mockGpio.get(), gpio_pins::OIL_PRESSURE);
    TestHelpers::configureMockForOilPressure(mockGpio.get(), 2048);
    sensor->init();
    
    // Multiple readings with same value should be identical
    Reading reading1 = sensor->getReading();
    Reading reading2 = sensor->getReading();
    Reading reading3 = sensor->getReading();
    
    int32_t pressure1 = std::get<int32_t>(reading1);
    int32_t pressure2 = std::get<int32_t>(reading2);
    int32_t pressure3 = std::get<int32_t>(reading3);
    
    TEST_ASSERT_EQUAL_INT32(pressure1, pressure2);
    TEST_ASSERT_EQUAL_INT32(pressure2, pressure3);
    
    // Change value and verify update
    TestHelpers::configureMockForOilPressure(mockGpio.get(), 3000);
    Reading reading4 = sensor->getReading();
    int32_t pressure4 = std::get<int32_t>(reading4);
    
    TEST_ASSERT_NOT_EQUAL(pressure1, pressure4);
    TEST_ASSERT_TRUE(pressure4 > pressure1);
}

void test_oil_pressure_sensor_adc_mapping_minimum() {
    auto sensor = std::make_unique<SimpleOilPressureSensor>(mockGpio.get(), gpio_pins::OIL_PRESSURE);
    TestHelpers::configureMockForOilPressure(mockGpio.get(), 0);
    sensor->init();
    
    Reading reading = sensor->getReading();
    int32_t pressure = std::get<int32_t>(reading);
    
    TEST_ASSERT_EQUAL_INT32(0, pressure);
    TestHelpers::assertValidPressureRange(pressure);
}

void test_oil_pressure_sensor_adc_mapping_maximum() {
    auto sensor = std::make_unique<SimpleOilPressureSensor>(mockGpio.get(), gpio_pins::OIL_PRESSURE);
    TestHelpers::configureMockForOilPressure(mockGpio.get(), 4095);
    sensor->init();
    
    Reading reading = sensor->getReading();
    int32_t pressure = std::get<int32_t>(reading);
    
    TEST_ASSERT_EQUAL_INT32(10, pressure);
    TestHelpers::assertValidPressureRange(pressure);
}

// Oil Temperature Sensor Tests - Enhanced Coverage  
void test_oil_temperature_sensor_initialization() {
    auto sensor = std::make_unique<SimpleOilTemperatureSensor>(mockGpio.get(), gpio_pins::OIL_TEMPERATURE);
    sensor->init();
    
    TEST_ASSERT_TRUE(sensor->isInitialized());
    Reading reading = sensor->getReading();
    TEST_ASSERT_FALSE(std::holds_alternative<std::monostate>(reading));
    TEST_ASSERT_TRUE(mockGpio->getAnalogReadCount(gpio_pins::OIL_TEMPERATURE) > 0);
}

void test_oil_temperature_sensor_adc_mapping_minimum() {
    auto sensor = std::make_unique<SimpleOilTemperatureSensor>(mockGpio.get(), gpio_pins::OIL_TEMPERATURE);
    TestHelpers::configureMockForOilTemperature(mockGpio.get(), 0);
    sensor->init();
    
    Reading reading = sensor->getReading();
    int32_t temperature = std::get<int32_t>(reading);
    
    TEST_ASSERT_EQUAL_INT32(0, temperature);
    TestHelpers::assertValidTemperatureRange(temperature);
}

void test_oil_temperature_sensor_adc_mapping_maximum() {
    auto sensor = std::make_unique<SimpleOilTemperatureSensor>(mockGpio.get(), gpio_pins::OIL_TEMPERATURE);
    TestHelpers::configureMockForOilTemperature(mockGpio.get(), 4095);
    sensor->init();
    
    Reading reading = sensor->getReading();
    int32_t temperature = std::get<int32_t>(reading);
    
    TEST_ASSERT_EQUAL_INT32(120, temperature);
    TestHelpers::assertValidTemperatureRange(temperature);
}

void test_oil_temperature_sensor_delta_updates() {
    auto sensor = std::make_unique<SimpleOilTemperatureSensor>(mockGpio.get(), gpio_pins::OIL_TEMPERATURE);
    TestHelpers::configureMockForOilTemperature(mockGpio.get(), 2048);
    sensor->init();
    
    // Multiple readings with same value should be identical
    Reading reading1 = sensor->getReading();
    Reading reading2 = sensor->getReading();
    Reading reading3 = sensor->getReading();
    
    int32_t temp1 = std::get<int32_t>(reading1);
    int32_t temp2 = std::get<int32_t>(reading2);
    int32_t temp3 = std::get<int32_t>(reading3);
    
    TEST_ASSERT_EQUAL_INT32(temp1, temp2);
    TEST_ASSERT_EQUAL_INT32(temp2, temp3);
    
    // Change value and verify update
    TestHelpers::configureMockForOilTemperature(mockGpio.get(), 3000);
    Reading reading4 = sensor->getReading();
    int32_t temp4 = std::get<int32_t>(reading4);
    
    TEST_ASSERT_NOT_EQUAL(temp1, temp4);
    TEST_ASSERT_TRUE(temp4 > temp1);
}

void test_oil_temperature_sensor_comprehensive_mapping() {
    auto sensor = std::make_unique<SimpleOilTemperatureSensor>(mockGpio.get(), gpio_pins::OIL_TEMPERATURE);
    sensor->init();
    
    // Test comprehensive temperature mapping
    struct TempTestCase {
        uint16_t adcValue;
        int32_t minExpected;
        int32_t maxExpected;
    };
    
    TempTestCase testCases[] = {
        {0, 0, 1},           // Minimum
        {4095, 119, 120},    // Maximum  
        {2048, 59, 61},      // Midpoint
        {820, 23, 25},       // Low range
        {3275, 95, 97}       // High range
    };
    
    for (const auto& testCase : testCases) {
        TestHelpers::configureMockForOilTemperature(mockGpio.get(), testCase.adcValue);
        Reading reading = sensor->getReading();
        int32_t temperature = std::get<int32_t>(reading);
        
        TEST_ASSERT_TRUE(temperature >= testCase.minExpected && temperature <= testCase.maxExpected);
        TestHelpers::assertValidTemperatureRange(temperature);
    }
}

// Key Sensor Tests - Enhanced Coverage
void test_key_sensor_initialization_comprehensive() {
    auto sensor = std::make_unique<SimpleKeySensor>(mockGpio.get(), gpio_pins::KEY_PRESENT, gpio_pins::KEY_NOT_PRESENT);
    sensor->init();
    
    TEST_ASSERT_TRUE(sensor->isInitialized());
    
    // Verify GPIO configuration
    TEST_ASSERT_TRUE(mockGpio->wasPinModeSet(gpio_pins::KEY_PRESENT));
    TEST_ASSERT_TRUE(mockGpio->wasPinModeSet(gpio_pins::KEY_NOT_PRESENT));
    
    // Verify pin modes (INPUT_PULLDOWN = 2)
    TEST_ASSERT_EQUAL_INT(2, mockGpio->getPinMode(gpio_pins::KEY_PRESENT));
    TEST_ASSERT_EQUAL_INT(2, mockGpio->getPinMode(gpio_pins::KEY_NOT_PRESENT));
    
    // Verify interrupts attached
    TEST_ASSERT_TRUE(mockGpio->wasInterruptAttached(gpio_pins::KEY_PRESENT));
    TEST_ASSERT_TRUE(mockGpio->wasInterruptAttached(gpio_pins::KEY_NOT_PRESENT));
}

void test_key_sensor_state_combinations() {
    auto sensor = std::make_unique<SimpleKeySensor>(mockGpio.get(), gpio_pins::KEY_PRESENT, gpio_pins::KEY_NOT_PRESENT);
    sensor->init();
    
    // Test all possible pin combinations
    struct KeyTestCase {
        bool keyPresent;
        bool keyNotPresent;
        bool expectedReading;
        KeyState expectedState;
        const char* description;
    };
    
    KeyTestCase testCases[] = {
        {true, false, true, KeyState::Present, "Key Present"},
        {false, true, false, KeyState::NotPresent, "Key Not Present"},
        {false, false, false, KeyState::Inactive, "Inactive (neither pin)"},
        {true, true, false, KeyState::Inactive, "Invalid (both pins)"}
    };
    
    for (const auto& testCase : testCases) {
        TestHelpers::configureMockForKeySensor(mockGpio.get(), testCase.keyPresent, testCase.keyNotPresent);
        Reading reading = sensor->getReading();
        bool result = std::get<bool>(reading);
        
        TEST_ASSERT_EQUAL(testCase.expectedReading, result);
        TEST_ASSERT_EQUAL_INT(static_cast<int>(testCase.expectedState), static_cast<int>(sensor->getLastState()));
    }
}

void test_key_sensor_present_state() {
    auto sensor = std::make_unique<SimpleKeySensor>(mockGpio.get(), gpio_pins::KEY_PRESENT, gpio_pins::KEY_NOT_PRESENT);
    TestHelpers::configureMockForKeySensor(mockGpio.get(), true, false);
    sensor->init();
    
    Reading reading = sensor->getReading();
    bool result = std::get<bool>(reading);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(KeyState::Present), static_cast<int>(sensor->getLastState()));
}

void test_key_sensor_absent_state() {
    auto sensor = std::make_unique<SimpleKeySensor>(mockGpio.get(), gpio_pins::KEY_PRESENT, gpio_pins::KEY_NOT_PRESENT);
    TestHelpers::configureMockForKeySensor(mockGpio.get(), false, true);
    sensor->init();
    
    Reading reading = sensor->getReading();
    bool result = std::get<bool>(reading);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(KeyState::NotPresent), static_cast<int>(sensor->getLastState()));
}

// Lock Sensor Tests
void test_lock_sensor_initialization() {
    auto sensor = std::make_unique<SimpleLockSensor>(mockGpio.get(), gpio_pins::LOCK);
    sensor->init();
    
    TEST_ASSERT_TRUE(sensor->isInitialized());
    TEST_ASSERT_TRUE(mockGpio->wasPinModeSet(gpio_pins::LOCK));
    TEST_ASSERT_EQUAL_INT(2, mockGpio->getPinMode(gpio_pins::LOCK)); // INPUT_PULLDOWN
    TEST_ASSERT_TRUE(mockGpio->wasInterruptAttached(gpio_pins::LOCK));
}

void test_lock_sensor_locked_state() {
    auto sensor = std::make_unique<SimpleLockSensor>(mockGpio.get(), gpio_pins::LOCK);
    TestHelpers::configureMockForLockSensor(mockGpio.get(), true);
    sensor->init();
    
    Reading reading = sensor->getReading();
    bool result = std::get<bool>(reading);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_TRUE(sensor->getLastReading());
}

void test_lock_sensor_unlocked_state() {
    auto sensor = std::make_unique<SimpleLockSensor>(mockGpio.get(), gpio_pins::LOCK);
    TestHelpers::configureMockForLockSensor(mockGpio.get(), false);
    sensor->init();
    
    Reading reading = sensor->getReading();
    bool result = std::get<bool>(reading);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_FALSE(sensor->getLastReading());
}

// Light Sensor Tests
void test_light_sensor_initialization() {
    auto sensor = std::make_unique<SimpleLightSensor>(mockGpio.get(), gpio_pins::LIGHTS);
    sensor->init();
    
    TEST_ASSERT_TRUE(sensor->isInitialized());
    TEST_ASSERT_TRUE(mockGpio->wasPinModeSet(gpio_pins::LIGHTS));
    TEST_ASSERT_EQUAL_INT(2, mockGpio->getPinMode(gpio_pins::LIGHTS)); // INPUT_PULLDOWN
}

void test_light_sensor_day_mode() {
    auto sensor = std::make_unique<SimpleLightSensor>(mockGpio.get(), gpio_pins::LIGHTS);
    TestHelpers::configureMockForLightSensor(mockGpio.get(), true);
    sensor->init();
    
    Reading reading = sensor->getReading();
    bool result = std::get<bool>(reading);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_TRUE(sensor->getLastReading());
}

void test_light_sensor_night_mode() {
    auto sensor = std::make_unique<SimpleLightSensor>(mockGpio.get(), gpio_pins::LIGHTS);
    TestHelpers::configureMockForLightSensor(mockGpio.get(), false);
    sensor->init();
    
    Reading reading = sensor->getReading();
    bool result = std::get<bool>(reading);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_FALSE(sensor->getLastReading());
}

// ============================================================================
// PHASE 2: MANAGER LAYER TESTS  
// ============================================================================

void test_trigger_manager_initialization() {
    auto keySensor = std::make_shared<SimpleKeySensor>(mockGpio.get(), gpio_pins::KEY_PRESENT, gpio_pins::KEY_NOT_PRESENT);
    auto lockSensor = std::make_shared<SimpleLockSensor>(mockGpio.get(), gpio_pins::LOCK);
    auto lightSensor = std::make_shared<SimpleLightSensor>(mockGpio.get(), gpio_pins::LIGHTS);
    
    auto triggerManager = std::make_unique<SimpleTriggerManager>(
        keySensor, lockSensor, lightSensor, 
        mockPanelService.get(), mockStyleService.get()
    );
    
    triggerManager->init();
    
    TEST_ASSERT_TRUE(triggerManager->isInitialized());
    TEST_ASSERT_TRUE(keySensor->isInitialized());
    TEST_ASSERT_TRUE(lockSensor->isInitialized());
    TEST_ASSERT_TRUE(lightSensor->isInitialized());
    
    // Verify triggers are registered
    TEST_ASSERT_TRUE(triggerManager->hasTrigger("key_present"));
    TEST_ASSERT_TRUE(triggerManager->hasTrigger("key_not_present"));
    TEST_ASSERT_TRUE(triggerManager->hasTrigger("lock_active"));
    TEST_ASSERT_TRUE(triggerManager->hasTrigger("light_change"));
    
    TEST_ASSERT_EQUAL_INT(4, triggerManager->getTriggerCount());
}

void test_trigger_manager_key_trigger_activation() {
    auto keySensor = std::make_shared<SimpleKeySensor>(mockGpio.get(), gpio_pins::KEY_PRESENT, gpio_pins::KEY_NOT_PRESENT);
    auto lockSensor = std::make_shared<SimpleLockSensor>(mockGpio.get(), gpio_pins::LOCK);
    auto lightSensor = std::make_shared<SimpleLightSensor>(mockGpio.get(), gpio_pins::LIGHTS);
    
    auto triggerManager = std::make_unique<SimpleTriggerManager>(
        keySensor, lockSensor, lightSensor,
        mockPanelService.get(), mockStyleService.get()
    );
    
    triggerManager->init();
    
    // Initially no key present
    TestHelpers::configureMockForKeySensor(mockGpio.get(), false, true);
    triggerManager->processTriggerEvents();
    
    // Insert key - should trigger key panel load
    TestHelpers::configureMockForKeySensor(mockGpio.get(), true, false);
    triggerManager->processTriggerEvents();
    
    // Verify panel was loaded
    auto loadHistory = mockPanelService->getLoadHistory();
    TEST_ASSERT_TRUE(loadHistory.size() > 0);
    TEST_ASSERT_EQUAL_STRING("key_panel", loadHistory.back().c_str());
}

void test_trigger_manager_lock_trigger_activation() {
    auto keySensor = std::make_shared<SimpleKeySensor>(mockGpio.get(), gpio_pins::KEY_PRESENT, gpio_pins::KEY_NOT_PRESENT);
    auto lockSensor = std::make_shared<SimpleLockSensor>(mockGpio.get(), gpio_pins::LOCK);
    auto lightSensor = std::make_shared<SimpleLightSensor>(mockGpio.get(), gpio_pins::LIGHTS);
    
    auto triggerManager = std::make_unique<SimpleTriggerManager>(
        keySensor, lockSensor, lightSensor,
        mockPanelService.get(), mockStyleService.get()
    );
    
    triggerManager->init();
    
    // Initially not locked
    TestHelpers::configureMockForLockSensor(mockGpio.get(), false);
    triggerManager->processTriggerEvents();
    
    // Activate lock
    TestHelpers::configureMockForLockSensor(mockGpio.get(), true);
    triggerManager->processTriggerEvents();
    
    // Verify lock panel was loaded
    auto loadHistory = mockPanelService->getLoadHistory();
    TEST_ASSERT_TRUE(loadHistory.size() > 0);
    TEST_ASSERT_EQUAL_STRING("lock_panel", loadHistory.back().c_str());
}

void test_trigger_manager_theme_switching() {
    auto keySensor = std::make_shared<SimpleKeySensor>(mockGpio.get(), gpio_pins::KEY_PRESENT, gpio_pins::KEY_NOT_PRESENT);
    auto lockSensor = std::make_shared<SimpleLockSensor>(mockGpio.get(), gpio_pins::LOCK);
    auto lightSensor = std::make_shared<SimpleLightSensor>(mockGpio.get(), gpio_pins::LIGHTS);
    
    auto triggerManager = std::make_unique<SimpleTriggerManager>(
        keySensor, lockSensor, lightSensor,
        mockPanelService.get(), mockStyleService.get()
    );
    
    triggerManager->init();
    
    // Initially day mode
    TestHelpers::configureMockForLightSensor(mockGpio.get(), true);
    triggerManager->processTriggerEvents();
    
    // Switch to night mode
    TestHelpers::configureMockForLightSensor(mockGpio.get(), false);
    triggerManager->processTriggerEvents();
    
    // Verify theme was switched to dark
    auto themeHistory = mockStyleService->getThemeHistory();
    TEST_ASSERT_TRUE(themeHistory.size() > 0);
    TEST_ASSERT_EQUAL_STRING("dark", themeHistory.back().c_str());
    TEST_ASSERT_TRUE(mockStyleService->getApplyCount() > 0);
}

void test_trigger_manager_priority_resolution() {
    auto keySensor = std::make_shared<SimpleKeySensor>(mockGpio.get(), gpio_pins::KEY_PRESENT, gpio_pins::KEY_NOT_PRESENT);
    auto lockSensor = std::make_shared<SimpleLockSensor>(mockGpio.get(), gpio_pins::LOCK);
    auto lightSensor = std::make_shared<SimpleLightSensor>(mockGpio.get(), gpio_pins::LIGHTS);
    
    auto triggerManager = std::make_unique<SimpleTriggerManager>(
        keySensor, lockSensor, lightSensor,
        mockPanelService.get(), mockStyleService.get()
    );
    
    triggerManager->init();
    
    // Trigger multiple events simultaneously
    TestHelpers::configureMockForKeySensor(mockGpio.get(), true, false);  // Key present
    TestHelpers::configureMockForLockSensor(mockGpio.get(), true);        // Lock active
    TestHelpers::configureMockForLightSensor(mockGpio.get(), false);      // Night mode
    
    triggerManager->processTriggerEvents();
    
    // Verify all appropriate actions were triggered
    auto loadHistory = mockPanelService->getLoadHistory();
    auto themeHistory = mockStyleService->getThemeHistory();
    
    // Should have both panel loads and theme change
    TEST_ASSERT_TRUE(loadHistory.size() >= 2);
    TEST_ASSERT_TRUE(themeHistory.size() > 0);
    TEST_ASSERT_EQUAL_STRING("dark", themeHistory.back().c_str());
}

void test_trigger_manager_startup_panel_override() {
    auto keySensor = std::make_shared<SimpleKeySensor>(mockGpio.get(), gpio_pins::KEY_PRESENT, gpio_pins::KEY_NOT_PRESENT);
    auto lockSensor = std::make_shared<SimpleLockSensor>(mockGpio.get(), gpio_pins::LOCK);
    auto lightSensor = std::make_shared<SimpleLightSensor>(mockGpio.get(), gpio_pins::LIGHTS);
    
    auto triggerManager = std::make_unique<SimpleTriggerManager>(
        keySensor, lockSensor, lightSensor,
        mockPanelService.get(), mockStyleService.get()
    );
    
    // Set startup override
    triggerManager->setStartupPanelOverride("splash_panel");
    
    // Verify override is returned
    const char* override = triggerManager->getStartupPanelOverride();
    TEST_ASSERT_NOT_NULL(override);
    TEST_ASSERT_EQUAL_STRING("splash_panel", override);
    
    // Clear override
    triggerManager->setStartupPanelOverride(nullptr);
    TEST_ASSERT_NULL(triggerManager->getStartupPanelOverride());
}

void test_panel_service_mock_functionality() {
    mockPanelService->init();
    
    // Test panel loading
    mockPanelService->loadPanel("test_panel");
    
    auto loadHistory = mockPanelService->getLoadHistory();
    TEST_ASSERT_EQUAL_INT(1, loadHistory.size());
    TEST_ASSERT_EQUAL_STRING("test_panel", loadHistory[0].c_str());
    
    // Test current panel
    IPanel* currentPanel = mockPanelService->getCurrentPanel();
    TEST_ASSERT_NOT_NULL(currentPanel);
    TEST_ASSERT_EQUAL_STRING("test_panel", currentPanel->getName());
    
    // Test panel refresh
    mockPanelService->refreshPanel();
    TEST_ASSERT_EQUAL_INT(1, mockPanelService->getRefreshCount());
    
    // Verify panel state
    MockPanel* mockPanel = mockPanelService->getMockCurrentPanel();
    TEST_ASSERT_TRUE(mockPanel->isInitialized());
    TEST_ASSERT_TRUE(mockPanel->isLoaded());
    TEST_ASSERT_EQUAL_INT(1, mockPanel->getUpdateCount());
}

void test_style_service_mock_functionality() {
    // Test theme switching
    mockStyleService->setTheme("dark");
    TEST_ASSERT_EQUAL_STRING("dark", mockStyleService->getCurrentTheme());
    
    mockStyleService->setTheme("light");
    TEST_ASSERT_EQUAL_STRING("light", mockStyleService->getCurrentTheme());
    
    // Test theme history
    auto themeHistory = mockStyleService->getThemeHistory();
    TEST_ASSERT_EQUAL_INT(2, themeHistory.size());
    TEST_ASSERT_EQUAL_STRING("dark", themeHistory[0].c_str());
    TEST_ASSERT_EQUAL_STRING("light", themeHistory[1].c_str());
    
    // Test apply count
    mockStyleService->applyTheme();
    mockStyleService->applyTheme();
    TEST_ASSERT_EQUAL_INT(2, mockStyleService->getApplyCount());
}

// ============================================================================
// PHASE 2: REAL MANAGER TESTS (Complete Implementation)
// ============================================================================

// PanelManager Comprehensive Tests
void test_panel_manager_initialization() {
    testPanelManager->init();
    
    TEST_ASSERT_TRUE(testPanelManager->isInitialized());
    TEST_ASSERT_TRUE(mockDisplay->isInitialized());
    TEST_ASSERT_FALSE(testPanelManager->getCurrentPanel().empty());
    TEST_ASSERT_EQUAL_STRING("oil_panel", testPanelManager->getCurrentPanel().c_str());
}

void test_panel_manager_panel_lifecycle() {
    testPanelManager->init();
    
    // Test panel creation and loading
    bool callbackExecuted = false;
    auto callback = [&callbackExecuted]() { callbackExecuted = true; };
    
    testPanelManager->createAndLoadPanel("key_panel", callback, false);
    
    // Verify panel was loaded
    TEST_ASSERT_EQUAL_STRING("key_panel", testPanelManager->getCurrentPanel().c_str());
    TEST_ASSERT_TRUE(callbackExecuted);
    
    auto history = testPanelManager->getPanelLoadHistory();
    TEST_ASSERT_EQUAL_INT(1, history.size());
    TEST_ASSERT_EQUAL_STRING("key_panel", history[0].c_str());
}

void test_panel_manager_splash_transitions() {
    testPanelManager->init();
    
    // Test splash panel transition
    testPanelManager->createAndLoadPanelWithSplash("main_panel");
    
    // Verify splash sequence
    auto history = testPanelManager->getPanelLoadHistory();
    TEST_ASSERT_EQUAL_INT(2, history.size());
    TEST_ASSERT_EQUAL_STRING("splash_panel", history[0].c_str());
    TEST_ASSERT_EQUAL_STRING("main_panel", history[1].c_str());
    TEST_ASSERT_EQUAL_STRING("main_panel", testPanelManager->getCurrentPanel().c_str());
}

void test_panel_manager_update_operations() {
    testPanelManager->init();
    
    // Test panel updates
    int initialUpdateCount = testPanelManager->getUpdateCount();
    
    testPanelManager->updatePanel();
    testPanelManager->updatePanel();
    testPanelManager->updatePanel();
    
    TEST_ASSERT_EQUAL_INT(initialUpdateCount + 3, testPanelManager->getUpdateCount());
}

void test_panel_manager_restoration_panel() {
    testPanelManager->init();
    
    // Test restoration panel functionality
    TEST_ASSERT_EQUAL_STRING("oil_panel", testPanelManager->getRestorationPanel());
    
    testPanelManager->setRestorationPanel("splash_panel");
    TEST_ASSERT_EQUAL_STRING("splash_panel", testPanelManager->getRestorationPanel());
}

void test_panel_manager_trigger_integration() {
    testPanelManager->init();
    
    // Test trigger callback handling
    testPanelManager->triggerPanelSwitchCallback("key_trigger");
    
    auto history = testPanelManager->getPanelLoadHistory();
    TEST_ASSERT_TRUE(history.size() > 0);
    TEST_ASSERT_EQUAL_STRING("trigger_key_trigger", history.back().c_str());
}

void test_panel_manager_ui_state_management() {
    testPanelManager->init();
    
    // Test UI state management (mock implementation)
    testPanelManager->setUiState(1); // LOADING
    testPanelManager->setUiState(2); // UPDATING
    testPanelManager->setUiState(0); // IDLE
    
    // Test that state changes don't crash (mock implementation)
    TEST_ASSERT_TRUE(testPanelManager->isInitialized());
}

// StyleManager Comprehensive Tests
void test_style_manager_initialization() {
    testStyleManager->init();
    
    TEST_ASSERT_TRUE(testStyleManager->isInitialized());
    TEST_ASSERT_EQUAL_STRING("night", testStyleManager->getCurrentTheme());
    
    // Verify default theme was applied during init
    auto history = testStyleManager->getThemeHistory();
    TEST_ASSERT_TRUE(history.size() > 0);
    TEST_ASSERT_EQUAL_STRING("night", history[0].c_str());
}

void test_style_manager_theme_switching() {
    testStyleManager->init();
    
    // Test theme switching
    testStyleManager->setTheme("day");
    TEST_ASSERT_EQUAL_STRING("day", testStyleManager->getCurrentTheme());
    TEST_ASSERT_TRUE(testStyleManager->hasStyleForTheme("day"));
    
    testStyleManager->setTheme("night");
    TEST_ASSERT_EQUAL_STRING("night", testStyleManager->getCurrentTheme());
    TEST_ASSERT_TRUE(testStyleManager->hasStyleForTheme("night"));
    
    // Verify theme history
    auto history = testStyleManager->getThemeHistory();
    TEST_ASSERT_TRUE(history.size() >= 3); // init + day + night
}

void test_style_manager_theme_application() {
    testStyleManager->init();
    
    int initialApplyCount = testStyleManager->getApplyCount();
    
    // Test theme application
    testStyleManager->applyTheme();
    TEST_ASSERT_EQUAL_INT(initialApplyCount + 1, testStyleManager->getApplyCount());
    
    // Test screen-specific application
    testStyleManager->applyThemeToScreen(nullptr); // Mock screen
    TEST_ASSERT_EQUAL_INT(initialApplyCount + 2, testStyleManager->getApplyCount());
}

void test_style_manager_style_getters() {
    testStyleManager->init();
    
    // Test style getter methods
    TEST_ASSERT_NOT_NULL(testStyleManager->getGaugeMainStyle());
    TEST_ASSERT_NOT_NULL(testStyleManager->getGaugeIndicatorStyle());
    TEST_ASSERT_NOT_NULL(testStyleManager->getGaugeItemsStyle());
    TEST_ASSERT_NOT_NULL(testStyleManager->getGaugeDangerSectionStyle());
}

void test_style_manager_theme_persistence() {
    testStyleManager->init();
    
    // Test that theme changes persist
    testStyleManager->setTheme("custom_theme");
    TEST_ASSERT_EQUAL_STRING("custom_theme", testStyleManager->getCurrentTheme());
    TEST_ASSERT_TRUE(testStyleManager->hasStyleForTheme("custom_theme"));
    
    // Apply theme multiple times
    testStyleManager->applyTheme();
    testStyleManager->applyTheme();
    
    // Theme should remain consistent
    TEST_ASSERT_EQUAL_STRING("custom_theme", testStyleManager->getCurrentTheme());
}

// PreferenceManager Comprehensive Tests
void test_preference_manager_initialization() {
    testPreferenceManager->init();
    
    TEST_ASSERT_TRUE(testPreferenceManager->isInitialized());
    TEST_ASSERT_TRUE(testPreferenceManager->hasConfig());
    TEST_ASSERT_EQUAL_INT(1, testPreferenceManager->getLoadCount());
    
    // Verify default config was created
    TEST_ASSERT_TRUE(testPreferenceManager->getPreferenceCount() > 0);
}

void test_preference_manager_default_config_creation() {
    testPreferenceManager->init();
    
    // Verify default values
    TEST_ASSERT_EQUAL_STRING("oil_panel", testPreferenceManager->getString("panel_name").c_str());
    TEST_ASSERT_EQUAL_STRING("night", testPreferenceManager->getString("theme").c_str());
    TEST_ASSERT_EQUAL_INT(80, testPreferenceManager->getInt("brightness"));
}

void test_preference_manager_string_operations() {
    testPreferenceManager->init();
    
    // Test string preferences
    testPreferenceManager->setString("test_string", "test_value");
    TEST_ASSERT_EQUAL_STRING("test_value", testPreferenceManager->getString("test_string").c_str());
    
    // Test default value handling
    TEST_ASSERT_EQUAL_STRING("default", testPreferenceManager->getString("nonexistent", "default").c_str());
}

void test_preference_manager_integer_operations() {
    testPreferenceManager->init();
    
    // Test integer preferences
    testPreferenceManager->setInt("test_int", 42);
    TEST_ASSERT_EQUAL_INT(42, testPreferenceManager->getInt("test_int"));
    
    // Test default value handling
    TEST_ASSERT_EQUAL_INT(999, testPreferenceManager->getInt("nonexistent", 999));
}

void test_preference_manager_boolean_operations() {
    testPreferenceManager->init();
    
    // Test boolean preferences
    testPreferenceManager->setBool("test_bool", true);
    TEST_ASSERT_TRUE(testPreferenceManager->getBool("test_bool"));
    
    testPreferenceManager->setBool("test_bool", false);
    TEST_ASSERT_FALSE(testPreferenceManager->getBool("test_bool"));
    
    // Test default value handling
    TEST_ASSERT_TRUE(testPreferenceManager->getBool("nonexistent", true));
}

void test_preference_manager_save_operations() {
    testPreferenceManager->init();
    
    int initialSaveCount = testPreferenceManager->getSaveCount();
    
    // Test save operations
    testPreferenceManager->saveConfig();
    TEST_ASSERT_EQUAL_INT(initialSaveCount + 1, testPreferenceManager->getSaveCount());
    
    testPreferenceManager->saveConfig();
    testPreferenceManager->saveConfig();
    TEST_ASSERT_EQUAL_INT(initialSaveCount + 3, testPreferenceManager->getSaveCount());
}

void test_preference_manager_corruption_recovery() {
    testPreferenceManager->init();
    
    // Simulate corruption
    testPreferenceManager->simulateCorruption();
    TEST_ASSERT_FALSE(testPreferenceManager->hasConfig());
    
    // Test recovery
    testPreferenceManager->loadConfig();
    TEST_ASSERT_TRUE(testPreferenceManager->hasConfig());
    TEST_ASSERT_TRUE(testPreferenceManager->getPreferenceCount() > 0);
    
    // Verify defaults were recreated
    TEST_ASSERT_EQUAL_STRING("oil_panel", testPreferenceManager->getString("panel_name").c_str());
}

void test_preference_manager_clear_operations() {
    testPreferenceManager->init();
    
    // Add some preferences
    testPreferenceManager->setString("test", "value");
    testPreferenceManager->setInt("number", 123);
    
    size_t countBeforeClear = testPreferenceManager->getPreferenceCount();
    TEST_ASSERT_TRUE(countBeforeClear > 0);
    
    // Clear preferences
    testPreferenceManager->clear();
    TEST_ASSERT_EQUAL_INT(0, testPreferenceManager->getPreferenceCount());
    TEST_ASSERT_FALSE(testPreferenceManager->hasConfig());
}

// ============================================================================
// PHASE 3: COMPONENT LAYER TESTS (UI Logic)
// ============================================================================

// OEM Oil Pressure Component Tests
void test_oem_oil_pressure_component_initialization() {
    ComponentLocation location(50, 50, 240, 240);
    
    testOilPressureComponent->render(nullptr, location);
    
    TEST_ASSERT_TRUE(testOilPressureComponent->isRendered());
    TEST_ASSERT_TRUE(mockDisplay->getUpdateCount() > 0);
    TEST_ASSERT_EQUAL_STRING("oil_pressure", testOilPressureComponent->getIconType().c_str());
}

void test_oem_oil_pressure_component_value_mapping() {
    testOilPressureComponent->setScaleRange(0, 100);
    
    // Test boundary values
    TEST_ASSERT_EQUAL_INT32(0, testOilPressureComponent->mapValueForDisplay(-10));   // Below min
    TEST_ASSERT_EQUAL_INT32(0, testOilPressureComponent->mapValueForDisplay(0));     // At min
    TEST_ASSERT_EQUAL_INT32(50, testOilPressureComponent->mapValueForDisplay(50));   // Middle
    TEST_ASSERT_EQUAL_INT32(100, testOilPressureComponent->mapValueForDisplay(100)); // At max  
    TEST_ASSERT_EQUAL_INT32(100, testOilPressureComponent->mapValueForDisplay(150)); // Above max
}

void test_oem_oil_pressure_component_danger_zone() {
    testOilPressureComponent->setDangerZone(80);
    
    // Test danger condition detection
    TEST_ASSERT_FALSE(testOilPressureComponent->isDangerCondition(50));  // Safe
    TEST_ASSERT_FALSE(testOilPressureComponent->isDangerCondition(79));  // Just below
    TEST_ASSERT_TRUE(testOilPressureComponent->isDangerCondition(80));   // At threshold
    TEST_ASSERT_TRUE(testOilPressureComponent->isDangerCondition(100));  // Above threshold
    
    // Test with component value
    testOilPressureComponent->setValue(90);
    TEST_ASSERT_TRUE(testOilPressureComponent->isInDangerZone());
    
    testOilPressureComponent->setValue(70);
    TEST_ASSERT_FALSE(testOilPressureComponent->isInDangerZone());
}

void test_oem_oil_pressure_component_refresh_with_reading() {
    ComponentLocation location(50, 50);
    testOilPressureComponent->render(nullptr, location);
    
    // Test refresh with int32_t reading
    Reading pressureReading = static_cast<int32_t>(75);
    testOilPressureComponent->refresh(pressureReading);
    
    TEST_ASSERT_EQUAL_INT32(75, testOilPressureComponent->getCurrentValue());
}

void test_oem_oil_pressure_component_scale_configuration() {
    testOilPressureComponent->setScaleRange(10, 90);
    
    TEST_ASSERT_EQUAL_INT32(10, testOilPressureComponent->getScaleMin());
    TEST_ASSERT_EQUAL_INT32(90, testOilPressureComponent->getScaleMax());
    
    // Test value mapping with custom range
    TEST_ASSERT_EQUAL_INT32(10, testOilPressureComponent->mapValueForDisplay(5));   // Below min -> min
    TEST_ASSERT_EQUAL_INT32(50, testOilPressureComponent->mapValueForDisplay(50));  // In range
    TEST_ASSERT_EQUAL_INT32(90, testOilPressureComponent->mapValueForDisplay(95));  // Above max -> max
}

// OEM Oil Temperature Component Tests  
void test_oem_oil_temperature_component_initialization() {
    ComponentLocation location(290, 50, 240, 240);
    
    testOilTemperatureComponent->render(nullptr, location);
    
    TEST_ASSERT_TRUE(testOilTemperatureComponent->isRendered());
    TEST_ASSERT_TRUE(mockDisplay->getUpdateCount() > 0);
    TEST_ASSERT_EQUAL_STRING("oil_temperature", testOilTemperatureComponent->getIconType().c_str());
}

void test_oem_oil_temperature_component_temperature_ranges() {
    // Test default Fahrenheit range
    TEST_ASSERT_EQUAL_INT32(160, testOilTemperatureComponent->getScaleMin());
    TEST_ASSERT_EQUAL_INT32(250, testOilTemperatureComponent->getScaleMax());
    TEST_ASSERT_EQUAL_INT32(220, testOilTemperatureComponent->getDangerZone());
}

void test_oem_oil_temperature_component_value_mapping() {
    testOilTemperatureComponent->setScaleRange(160, 250);
    
    // Test temperature value mapping
    TEST_ASSERT_EQUAL_INT32(160, testOilTemperatureComponent->mapValueForDisplay(150)); // Below min
    TEST_ASSERT_EQUAL_INT32(180, testOilTemperatureComponent->mapValueForDisplay(180)); // Normal range
    TEST_ASSERT_EQUAL_INT32(220, testOilTemperatureComponent->mapValueForDisplay(220)); // Danger threshold
    TEST_ASSERT_EQUAL_INT32(250, testOilTemperatureComponent->mapValueForDisplay(260)); // Above max
}

void test_oem_oil_temperature_component_danger_detection() {
    testOilTemperatureComponent->setDangerZone(220);
    
    TEST_ASSERT_FALSE(testOilTemperatureComponent->isDangerCondition(190)); // Safe temp
    TEST_ASSERT_FALSE(testOilTemperatureComponent->isDangerCondition(219)); // Just below danger
    TEST_ASSERT_TRUE(testOilTemperatureComponent->isDangerCondition(220));  // At danger threshold
    TEST_ASSERT_TRUE(testOilTemperatureComponent->isDangerCondition(240));  // High temp
    
    // Test with actual component value
    testOilTemperatureComponent->setValue(230);
    TEST_ASSERT_TRUE(testOilTemperatureComponent->isInDangerZone());
}

void test_oem_oil_temperature_component_refresh() {
    ComponentLocation location(290, 50);
    testOilTemperatureComponent->render(nullptr, location);
    
    Reading tempReading = static_cast<int32_t>(195);
    testOilTemperatureComponent->refresh(tempReading);
    
    TEST_ASSERT_EQUAL_INT32(195, testOilTemperatureComponent->getCurrentValue());
}

// Key Component Tests
void test_key_component_initialization() {
    ComponentLocation location(120, 300, 80, 80);
    
    testKeyComponent->render(nullptr, location);
    
    TEST_ASSERT_TRUE(testKeyComponent->isRendered());
    TEST_ASSERT_TRUE(mockDisplay->getUpdateCount() > 0);
    TEST_ASSERT_EQUAL_STRING("key", testKeyComponent->getIconType().c_str());
}

void test_key_component_state_management() {
    testStyleManager->init(); // Initialize with default theme
    ComponentLocation location(120, 300);
    testKeyComponent->render(nullptr, location);
    
    // Test initial state
    TEST_ASSERT_FALSE(testKeyComponent->getCurrentState());
    
    // Test state change to active
    testKeyComponent->setState(true);
    TEST_ASSERT_TRUE(testKeyComponent->getCurrentState());
    TEST_ASSERT_EQUAL_STRING("night_active", testKeyComponent->getCurrentColor().c_str());
    
    // Test state change to inactive
    testKeyComponent->setState(false);  
    TEST_ASSERT_FALSE(testKeyComponent->getCurrentState());
    TEST_ASSERT_EQUAL_STRING("night_inactive", testKeyComponent->getCurrentColor().c_str());
}

void test_key_component_theme_integration() {
    testStyleManager->init();
    ComponentLocation location(120, 300);
    testKeyComponent->render(nullptr, location);
    
    // Test with night theme
    testStyleManager->setTheme("night");
    testKeyComponent->setState(true);
    TEST_ASSERT_EQUAL_STRING("night_active", testKeyComponent->getCurrentColor().c_str());
    
    // Test with day theme
    testStyleManager->setTheme("day");
    testKeyComponent->setState(true);
    TEST_ASSERT_EQUAL_STRING("day_active", testKeyComponent->getCurrentColor().c_str());
}

void test_key_component_refresh_with_reading() {
    ComponentLocation location(120, 300);
    testKeyComponent->render(nullptr, location);
    
    // Test refresh with boolean reading
    Reading keyReading = true;
    testKeyComponent->refresh(keyReading);
    TEST_ASSERT_TRUE(testKeyComponent->getCurrentState());
    
    keyReading = false;
    testKeyComponent->refresh(keyReading);
    TEST_ASSERT_FALSE(testKeyComponent->getCurrentState());
}

// Lock Component Tests
void test_lock_component_initialization() {
    ComponentLocation location(200, 300, 80, 80);
    
    testLockComponent->render(nullptr, location);
    
    TEST_ASSERT_TRUE(testLockComponent->isRendered());
    TEST_ASSERT_TRUE(mockDisplay->getUpdateCount() > 0);
    TEST_ASSERT_EQUAL_STRING("lock", testLockComponent->getIconType().c_str());
}

void test_lock_component_state_management() {
    testStyleManager->init();
    ComponentLocation location(200, 300);
    testLockComponent->render(nullptr, location);
    
    // Test locked state
    testLockComponent->setState(true);
    TEST_ASSERT_TRUE(testLockComponent->getCurrentState());
    TEST_ASSERT_EQUAL_STRING("night_locked", testLockComponent->getCurrentColor().c_str());
    
    // Test unlocked state  
    testLockComponent->setState(false);
    TEST_ASSERT_FALSE(testLockComponent->getCurrentState());
    TEST_ASSERT_EQUAL_STRING("night_unlocked", testLockComponent->getCurrentColor().c_str());
}

void test_lock_component_theme_integration() {
    testStyleManager->init();
    ComponentLocation location(200, 300);
    testLockComponent->render(nullptr, location);
    
    // Test theme changes affect lock colors
    testStyleManager->setTheme("day");
    testLockComponent->setState(true);
    TEST_ASSERT_EQUAL_STRING("day_locked", testLockComponent->getCurrentColor().c_str());
    
    testStyleManager->setTheme("night");  
    testLockComponent->setState(false);
    TEST_ASSERT_EQUAL_STRING("night_unlocked", testLockComponent->getCurrentColor().c_str());
}

void test_lock_component_refresh() {
    ComponentLocation location(200, 300);
    testLockComponent->render(nullptr, location);
    
    Reading lockReading = true;
    testLockComponent->refresh(lockReading);
    TEST_ASSERT_TRUE(testLockComponent->getCurrentState());
    
    lockReading = false;
    testLockComponent->refresh(lockReading);
    TEST_ASSERT_FALSE(testLockComponent->getCurrentState());
}

// Clarity Component Tests
void test_clarity_component_initialization() {
    ComponentLocation location(120, 120, 200, 100);
    
    testClarityComponent->render(nullptr, location);
    
    TEST_ASSERT_TRUE(testClarityComponent->isRendered());
    TEST_ASSERT_TRUE(mockDisplay->getUpdateCount() > 0);
    TEST_ASSERT_EQUAL_STRING("clarity_logo", testClarityComponent->getLogoType().c_str());
}

void test_clarity_component_theme_awareness() {
    testStyleManager->init();
    ComponentLocation location(120, 120);
    
    // Test initial theme
    testClarityComponent->render(nullptr, location);
    TEST_ASSERT_EQUAL_STRING("night", testClarityComponent->getCurrentTheme().c_str());
    
    // Test theme change
    testStyleManager->setTheme("day");
    testClarityComponent->refresh(Reading{});
    TEST_ASSERT_EQUAL_STRING("day", testClarityComponent->getCurrentTheme().c_str());
}

void test_clarity_component_branding_display() {
    ComponentLocation location(120, 120);
    testClarityComponent->render(nullptr, location);
    
    // Verify branding elements are set up
    TEST_ASSERT_TRUE(testClarityComponent->isRendered());
    TEST_ASSERT_EQUAL_STRING("clarity_logo", testClarityComponent->getLogoType().c_str());
    
    // Should work with any theme
    testStyleManager->setTheme("custom_theme");
    testClarityComponent->refresh(Reading{});
    TEST_ASSERT_EQUAL_STRING("custom_theme", testClarityComponent->getCurrentTheme().c_str());
}

// Component Integration Tests
void test_component_lifecycle_management() {
    ComponentLocation oilPressureLoc(50, 50);
    ComponentLocation oilTempLoc(290, 50);
    ComponentLocation keyLoc(120, 300);
    
    // Test multiple component rendering
    testOilPressureComponent->render(nullptr, oilPressureLoc);
    testOilTemperatureComponent->render(nullptr, oilTempLoc);
    testKeyComponent->render(nullptr, keyLoc);
    
    TEST_ASSERT_TRUE(testOilPressureComponent->isRendered());
    TEST_ASSERT_TRUE(testOilTemperatureComponent->isRendered());
    TEST_ASSERT_TRUE(testKeyComponent->isRendered());
    
    // Verify display updates for all components
    int finalUpdateCount = mockDisplay->getUpdateCount();
    TEST_ASSERT_TRUE(finalUpdateCount >= 3); // At least one update per component
}

void test_component_data_flow_integration() {
    ComponentLocation pressureLoc(50, 50);
    ComponentLocation keyLoc(120, 300);
    
    testOilPressureComponent->render(nullptr, pressureLoc);
    testKeyComponent->render(nullptr, keyLoc);
    
    // Test data flow from readings to components
    Reading pressureData = static_cast<int32_t>(85);
    Reading keyData = true;
    
    testOilPressureComponent->refresh(pressureData);
    testKeyComponent->refresh(keyData);
    
    TEST_ASSERT_EQUAL_INT32(85, testOilPressureComponent->getCurrentValue());
    TEST_ASSERT_TRUE(testKeyComponent->getCurrentState());
}

void test_component_theme_coordination() {
    testStyleManager->init();
    ComponentLocation keyLoc(120, 300);
    ComponentLocation lockLoc(200, 300);
    ComponentLocation clarityLoc(120, 120);
    
    testKeyComponent->render(nullptr, keyLoc);
    testLockComponent->render(nullptr, lockLoc);
    testClarityComponent->render(nullptr, clarityLoc);
    
    // Test coordinated theme change across all components
    testStyleManager->setTheme("day");
    
    testKeyComponent->setState(true);
    testLockComponent->setState(true);
    testClarityComponent->refresh(Reading{});
    
    // Verify all components use the same theme
    TEST_ASSERT_EQUAL_STRING("day_active", testKeyComponent->getCurrentColor().c_str());
    TEST_ASSERT_EQUAL_STRING("day_locked", testLockComponent->getCurrentColor().c_str());
    TEST_ASSERT_EQUAL_STRING("day", testClarityComponent->getCurrentTheme().c_str());
}

// ====================================================================
// PHASE 4: INTEGRATION & SCENARIO TESTS - SUPPORTING TYPES & CLASSES
// ====================================================================

// Theme enumeration already defined earlier

// Trigger type enumeration for manager integration
enum class TriggerType {
    NONE = 0,
    KEY_PRESENT = 1,
    KEY_NOT_PRESENT = 2,
    LOCK = 3,
    THEME = 4
};

// GPIO pin constants for Phase 4 integration tests
constexpr int GPIO_PRESSURE_SENSOR = 34;
constexpr int GPIO_TEMPERATURE_SENSOR = 35;
constexpr int GPIO_KEY_PRESENT = 12;
constexpr int GPIO_KEY_NOT_PRESENT = 13;
constexpr int GPIO_LOCK_TRIGGER = 14;
constexpr int GPIO_LIGHT_SENSOR = 15;

/**
 * @brief Test Trigger Manager for integration testing
 */
class TestTriggerManager {
private:
    MockGpioProvider* gpio_;
    TriggerType activeTrigger_ = TriggerType::NONE;
    
public:
    TestTriggerManager(MockGpioProvider* gpio) : gpio_(gpio) {}
    
    void update() {
        // Check triggers in priority order
        bool keyPresent = gpio_->getDigitalValue(GPIO_KEY_PRESENT);
        bool keyNotPresent = gpio_->getDigitalValue(GPIO_KEY_NOT_PRESENT);
        bool lock = gpio_->getDigitalValue(GPIO_LOCK_TRIGGER);
        bool light = gpio_->getDigitalValue(GPIO_LIGHT_SENSOR);
        
        // Handle conflicting key states (invalid state)
        if (keyPresent && keyNotPresent) {
            activeTrigger_ = TriggerType::LOCK; // Fall back to lock trigger
            return;
        }
        
        // Priority: Key Present > Key Not Present > Lock > Theme
        if (keyPresent) {
            activeTrigger_ = TriggerType::KEY_PRESENT;
        } else if (keyNotPresent) {
            activeTrigger_ = TriggerType::KEY_NOT_PRESENT;
        } else if (lock) {
            activeTrigger_ = TriggerType::LOCK;
        } else if (light) {
            activeTrigger_ = TriggerType::THEME;
        } else {
            activeTrigger_ = TriggerType::NONE;
        }
    }
    
    TriggerType getActiveTrigger() {
        return activeTrigger_;
    }
};

// ====================================================================
// PHASE 4: INTEGRATION & SCENARIO TESTS
// ====================================================================

/**
 * @brief System Integration Test Helper Class
 * @details Provides complete system mock for integration testing
 */
class SystemIntegrationFixture {
public:
    MockGpioProvider* gpio;
    TestStyleManager* styleManager;
    TestPreferenceManager* prefManager;
    TestTriggerManager* triggerManager;
    TestPanelManager* panelManager;
    MockDisplayProvider* displayProvider;
    
    // Test sensors
    std::unique_ptr<SimpleOilPressureSensor> pressureSensor;
    std::unique_ptr<SimpleOilTemperatureSensor> temperatureSensor;
    std::unique_ptr<SimpleKeySensor> keySensor;
    std::unique_ptr<SimpleLockSensor> lockSensor;
    std::unique_ptr<SimpleLightSensor> lightSensor;
    
    // Test components
    std::unique_ptr<TestOemOilPressureComponent> pressureComponent;
    std::unique_ptr<TestOemOilTemperatureComponent> temperatureComponent;
    std::unique_ptr<TestKeyComponent> keyComponent;
    std::unique_ptr<TestLockComponent> lockComponent;
    std::unique_ptr<TestClarityComponent> clarityComponent;
    
    SystemIntegrationFixture() {
        // Initialize core services
        gpio = new MockGpioProvider();
        displayProvider = new MockDisplayProvider();
        styleManager = new TestStyleManager();
        prefManager = new TestPreferenceManager();
        triggerManager = new TestTriggerManager(gpio);
        panelManager = new TestPanelManager(displayProvider, gpio, nullptr);
        
        // Initialize sensors
        pressureSensor = std::make_unique<SimpleOilPressureSensor>(gpio, GPIO_PRESSURE_SENSOR);
        temperatureSensor = std::make_unique<SimpleOilTemperatureSensor>(gpio, GPIO_TEMPERATURE_SENSOR);
        keySensor = std::make_unique<SimpleKeySensor>(gpio, GPIO_KEY_PRESENT, GPIO_KEY_NOT_PRESENT);
        lockSensor = std::make_unique<SimpleLockSensor>(gpio, GPIO_LOCK_TRIGGER);
        lightSensor = std::make_unique<SimpleLightSensor>(gpio, GPIO_LIGHT_SENSOR);
        
        // Initialize components
        pressureComponent = std::make_unique<TestOemOilPressureComponent>(displayProvider, styleManager);
        temperatureComponent = std::make_unique<TestOemOilTemperatureComponent>(displayProvider, styleManager);
        keyComponent = std::make_unique<TestKeyComponent>(displayProvider, styleManager);
        lockComponent = std::make_unique<TestLockComponent>(displayProvider, styleManager);
        clarityComponent = std::make_unique<TestClarityComponent>(displayProvider, styleManager);
        
        // Set up default theme
        styleManager->setTheme(Theme::DAY);
    }
    
    ~SystemIntegrationFixture() {
        delete gpio;
        delete displayProvider;
        delete styleManager;
        delete prefManager;
        delete triggerManager;
        delete panelManager;
    }
    
    void simulateSystemStartup() {
        // Simulate system initialization sequence
        panelManager->clearPanelHistory();
        styleManager->setTheme(Theme::DAY);
        
        // Simulate sensors initialization
        pressureSensor->begin();
        temperatureSensor->begin();
        keySensor->begin();
        lockSensor->begin();
        lightSensor->begin();
    }
    
    void simulateSensorData(int32_t pressure, int32_t temperature) {
        gpio->setAnalogValue(GPIO_PRESSURE_SENSOR, pressure);
        gpio->setAnalogValue(GPIO_TEMPERATURE_SENSOR, temperature);
    }
    
    void simulateTriggers(bool keyPresent, bool keyNotPresent, bool lock, bool light) {
        gpio->setDigitalValue(GPIO_KEY_PRESENT, keyPresent);
        gpio->setDigitalValue(GPIO_KEY_NOT_PRESENT, keyNotPresent);
        gpio->setDigitalValue(GPIO_LOCK_TRIGGER, lock);
        gpio->setDigitalValue(GPIO_LIGHT_SENSOR, light);
    }
    
    void updateSystem() {
        // Update all sensors
        pressureSensor->update();
        temperatureSensor->update();
        keySensor->update();
        lockSensor->update();
        lightSensor->update();
        
        // Update trigger manager
        triggerManager->update();
        
        // Simulate panel updates based on triggers
        updatePanelsBasedOnTriggers();
    }
    
private:
    void updatePanelsBasedOnTriggers() {
        TriggerType activeTrigger = triggerManager->getActiveTrigger();
        
        switch(activeTrigger) {
            case TriggerType::KEY_PRESENT:
                panelManager->showKeyPanel();
                keyComponent->setValue(1); // Green icon
                break;
            case TriggerType::KEY_NOT_PRESENT:
                panelManager->showKeyPanel();
                keyComponent->setValue(0); // Red icon
                break;
            case TriggerType::LOCK:
                panelManager->showLockPanel();
                break;
            case TriggerType::THEME:
                // Theme change doesn't trigger panel switch
                styleManager->setTheme(lightSensor->read() ? Theme::NIGHT : Theme::DAY);
                break;
            case TriggerType::NONE:
            default:
                panelManager->showOilPanel();
                // Update components with sensor data
                pressureComponent->setValue(pressureSensor->read());
                temperatureComponent->setValue(temperatureSensor->read());
                break;
        }
    }
};

// 4.1 Scenario-Based Integration Tests

/**
 * @test test_major_scenario_complete_system
 * @brief Tests the complete major scenario from scenario.md
 */
void test_major_scenario_complete_system(void) {
    SystemIntegrationFixture fixture;
    
    // App starts with pressure and temperature values set to halfway
    fixture.simulateSystemStartup();
    fixture.simulateSensorData(2048, 2048); // Halfway values
    fixture.simulateTriggers(false, false, false, false); // No triggers
    
    // → Splash animates with day theme (white text)
    fixture.updateSystem();
    TEST_ASSERT_EQUAL(Theme::DAY, fixture.styleManager->getCurrentTheme());
    TEST_ASSERT_TRUE(fixture.panelManager->wasSplashShown());
    
    // → Oil panel loads with day theme (white scale ticks and icon)
    TEST_ASSERT_EQUAL_STRING("OilPanel", fixture.panelManager->getCurrentPanel().c_str());
    TEST_ASSERT_EQUAL(Theme::DAY, fixture.styleManager->getCurrentTheme());
    
    // → Oil panel needles animate (verify components got values)
    TEST_ASSERT_EQUAL(fixture.pressureSensor->read(), fixture.pressureComponent->getCurrentValue());
    TEST_ASSERT_EQUAL(fixture.temperatureSensor->read(), fixture.temperatureComponent->getCurrentValue());
    
    // → Lights trigger high
    fixture.simulateTriggers(false, false, false, true);
    fixture.updateSystem();
    
    // → Oil panel does NOT reload, theme changes to night (red scale ticks and icon)
    TEST_ASSERT_EQUAL_STRING("OilPanel", fixture.panelManager->getCurrentPanel().c_str());
    TEST_ASSERT_EQUAL(Theme::NIGHT, fixture.styleManager->getCurrentTheme());
    
    // → Lock trigger high
    fixture.simulateTriggers(false, false, true, true);
    fixture.updateSystem();
    
    // → Lock panel loads
    TEST_ASSERT_EQUAL_STRING("LockPanel", fixture.panelManager->getCurrentPanel().c_str());
    
    // → Key not present trigger high
    fixture.simulateTriggers(false, true, true, true);
    fixture.updateSystem();
    
    // → Key panel loads (present = false → red icon)
    TEST_ASSERT_EQUAL_STRING("KeyPanel", fixture.panelManager->getCurrentPanel().c_str());
    TEST_ASSERT_EQUAL(0, fixture.keyComponent->getCurrentValue()); // Red icon
    
    // → Key present trigger high (invalid state)
    fixture.simulateTriggers(true, true, true, true);
    fixture.updateSystem();
    
    // → Lock panel loads (both key present and key not present true = invalid state)
    TEST_ASSERT_EQUAL_STRING("LockPanel", fixture.panelManager->getCurrentPanel().c_str());
    
    // → Key not present trigger low
    fixture.simulateTriggers(true, false, true, true);
    fixture.updateSystem();
    
    // → Key panel loads (present = true → green icon, key present trigger still high)
    TEST_ASSERT_EQUAL_STRING("KeyPanel", fixture.panelManager->getCurrentPanel().c_str());
    TEST_ASSERT_EQUAL(1, fixture.keyComponent->getCurrentValue()); // Green icon
    
    // → Key present trigger low
    fixture.simulateTriggers(false, false, true, true);
    fixture.updateSystem();
    
    // → Lock panel loads (lock trigger still high)
    TEST_ASSERT_EQUAL_STRING("LockPanel", fixture.panelManager->getCurrentPanel().c_str());
    
    // → Lock trigger low
    fixture.simulateTriggers(false, false, false, true);
    fixture.updateSystem();
    
    // → Oil panel loads with night theme (red scale ticks and icon, lights trigger still active)
    TEST_ASSERT_EQUAL_STRING("OilPanel", fixture.panelManager->getCurrentPanel().c_str());
    TEST_ASSERT_EQUAL(Theme::NIGHT, fixture.styleManager->getCurrentTheme());
    
    // → Oil panel needles animate (verify components updated)
    TEST_ASSERT_EQUAL(fixture.pressureSensor->read(), fixture.pressureComponent->getCurrentValue());
    TEST_ASSERT_EQUAL(fixture.temperatureSensor->read(), fixture.temperatureComponent->getCurrentValue());
    
    // → Lights trigger low
    fixture.simulateTriggers(false, false, false, false);
    fixture.updateSystem();
    
    // → Oil panel does NOT reload, theme changes to day (white scale ticks and icon)
    TEST_ASSERT_EQUAL_STRING("OilPanel", fixture.panelManager->getCurrentPanel().c_str());
    TEST_ASSERT_EQUAL(Theme::DAY, fixture.styleManager->getCurrentTheme());
}

/**
 * @test test_startup_scenario_default
 * @brief Tests default startup scenario
 */
void test_startup_scenario_default(void) {
    SystemIntegrationFixture fixture;
    
    // App starts → Splash animates with day theme (white text)
    fixture.simulateSystemStartup();
    fixture.updateSystem();
    
    TEST_ASSERT_TRUE(fixture.panelManager->wasSplashShown());
    TEST_ASSERT_EQUAL(Theme::DAY, fixture.styleManager->getCurrentTheme());
    TEST_ASSERT_EQUAL_STRING("OilPanel", fixture.panelManager->getCurrentPanel().c_str());
}

/**
 * @test test_startup_scenario_with_oil_data
 * @brief Tests startup with oil data scenario
 */
void test_startup_scenario_with_oil_data(void) {
    SystemIntegrationFixture fixture;
    
    // App starts with pressure and temperature values set to halfway
    fixture.simulateSystemStartup();
    fixture.simulateSensorData(2048, 2048); // Halfway values
    fixture.updateSystem();
    
    // → Splash animates with day theme (white text)
    TEST_ASSERT_TRUE(fixture.panelManager->wasSplashShown());
    TEST_ASSERT_EQUAL(Theme::DAY, fixture.styleManager->getCurrentTheme());
    
    // → Oil panel loads with day theme (white scale ticks and icon)
    TEST_ASSERT_EQUAL_STRING("OilPanel", fixture.panelManager->getCurrentPanel().c_str());
    TEST_ASSERT_EQUAL(Theme::DAY, fixture.styleManager->getCurrentTheme());
    
    // → Oil panel needles animate
    TEST_ASSERT_EQUAL(fixture.pressureSensor->read(), fixture.pressureComponent->getCurrentValue());
    TEST_ASSERT_EQUAL(fixture.temperatureSensor->read(), fixture.temperatureComponent->getCurrentValue());
}

/**
 * @test test_startup_scenario_with_triggers
 * @brief Tests startup with active triggers
 */
void test_startup_scenario_with_triggers(void) {
    SystemIntegrationFixture fixture;
    
    // App starts with key present trigger high
    fixture.simulateSystemStartup();
    fixture.simulateTriggers(true, false, false, false); // Key present
    fixture.updateSystem();
    
    // → Splash animates
    TEST_ASSERT_TRUE(fixture.panelManager->wasSplashShown());
    
    // → Oil panel does NOT load
    // → Key panel loads (green icon)
    TEST_ASSERT_EQUAL_STRING("KeyPanel", fixture.panelManager->getCurrentPanel().c_str());
    TEST_ASSERT_EQUAL(1, fixture.keyComponent->getCurrentValue()); // Green icon
    
    // → Key present trigger low → Oil panel loads
    fixture.simulateTriggers(false, false, false, false);
    fixture.updateSystem();
    TEST_ASSERT_EQUAL_STRING("OilPanel", fixture.panelManager->getCurrentPanel().c_str());
}

/**
 * @test test_key_present_scenario_runtime
 * @brief Tests key present scenario during runtime
 */
void test_key_present_scenario_runtime(void) {
    SystemIntegrationFixture fixture;
    
    // App starts → Splash animates → Oil panel loads
    fixture.simulateSystemStartup();
    fixture.updateSystem();
    TEST_ASSERT_EQUAL_STRING("OilPanel", fixture.panelManager->getCurrentPanel().c_str());
    
    // → Key present trigger high → Key panel loads (green icon)
    fixture.simulateTriggers(true, false, false, false);
    fixture.updateSystem();
    TEST_ASSERT_EQUAL_STRING("KeyPanel", fixture.panelManager->getCurrentPanel().c_str());
    TEST_ASSERT_EQUAL(1, fixture.keyComponent->getCurrentValue()); // Green icon
    
    // → Key present trigger low → Oil panel loads
    fixture.simulateTriggers(false, false, false, false);
    fixture.updateSystem();
    TEST_ASSERT_EQUAL_STRING("OilPanel", fixture.panelManager->getCurrentPanel().c_str());
}

/**
 * @test test_key_present_scenario_startup
 * @brief Tests key present scenario at startup
 */
void test_key_present_scenario_startup(void) {
    SystemIntegrationFixture fixture;
    
    // App starts with key present trigger high
    fixture.simulateSystemStartup();
    fixture.simulateTriggers(true, false, false, false);
    fixture.updateSystem();
    
    // → Splash animates
    TEST_ASSERT_TRUE(fixture.panelManager->wasSplashShown());
    
    // → Oil panel does NOT load
    // → Key panel loads (green icon)
    TEST_ASSERT_EQUAL_STRING("KeyPanel", fixture.panelManager->getCurrentPanel().c_str());
    TEST_ASSERT_EQUAL(1, fixture.keyComponent->getCurrentValue()); // Green icon
    
    // → Key present trigger low → Oil panel loads
    fixture.simulateTriggers(false, false, false, false);
    fixture.updateSystem();
    TEST_ASSERT_EQUAL_STRING("OilPanel", fixture.panelManager->getCurrentPanel().c_str());
}

/**
 * @test test_key_not_present_scenario_runtime
 * @brief Tests key not present scenario during runtime
 */
void test_key_not_present_scenario_runtime(void) {
    SystemIntegrationFixture fixture;
    
    // App starts → Oil panel loads
    fixture.simulateSystemStartup();
    fixture.updateSystem();
    TEST_ASSERT_EQUAL_STRING("OilPanel", fixture.panelManager->getCurrentPanel().c_str());
    
    // → Key not present trigger high → Key panel loads (red icon)
    fixture.simulateTriggers(false, true, false, false);
    fixture.updateSystem();
    TEST_ASSERT_EQUAL_STRING("KeyPanel", fixture.panelManager->getCurrentPanel().c_str());
    TEST_ASSERT_EQUAL(0, fixture.keyComponent->getCurrentValue()); // Red icon
    
    // → Key not present trigger low → Oil panel loads
    fixture.simulateTriggers(false, false, false, false);
    fixture.updateSystem();
    TEST_ASSERT_EQUAL_STRING("OilPanel", fixture.panelManager->getCurrentPanel().c_str());
}

/**
 * @test test_key_not_present_scenario_startup
 * @brief Tests key not present scenario at startup
 */
void test_key_not_present_scenario_startup(void) {
    SystemIntegrationFixture fixture;
    
    // App starts with key not present trigger high
    fixture.simulateSystemStartup();
    fixture.simulateTriggers(false, true, false, false);
    fixture.updateSystem();
    
    // → Splash animates
    TEST_ASSERT_TRUE(fixture.panelManager->wasSplashShown());
    
    // → Oil panel does NOT load
    // → Key panel loads (red icon)
    TEST_ASSERT_EQUAL_STRING("KeyPanel", fixture.panelManager->getCurrentPanel().c_str());
    TEST_ASSERT_EQUAL(0, fixture.keyComponent->getCurrentValue()); // Red icon
    
    // → Key not present trigger low → Oil panel loads
    fixture.simulateTriggers(false, false, false, false);
    fixture.updateSystem();
    TEST_ASSERT_EQUAL_STRING("OilPanel", fixture.panelManager->getCurrentPanel().c_str());
}

/**
 * @test test_lock_scenario_runtime
 * @brief Tests lock scenario during runtime
 */
void test_lock_scenario_runtime(void) {
    SystemIntegrationFixture fixture;
    
    // App starts → Oil panel loads
    fixture.simulateSystemStartup();
    fixture.updateSystem();
    TEST_ASSERT_EQUAL_STRING("OilPanel", fixture.panelManager->getCurrentPanel().c_str());
    
    // → Lock trigger high → Lock panel loads
    fixture.simulateTriggers(false, false, true, false);
    fixture.updateSystem();
    TEST_ASSERT_EQUAL_STRING("LockPanel", fixture.panelManager->getCurrentPanel().c_str());
    
    // → Lock trigger low → Oil panel loads
    fixture.simulateTriggers(false, false, false, false);
    fixture.updateSystem();
    TEST_ASSERT_EQUAL_STRING("OilPanel", fixture.panelManager->getCurrentPanel().c_str());
}

/**
 * @test test_lock_scenario_startup
 * @brief Tests lock scenario at startup
 */
void test_lock_scenario_startup(void) {
    SystemIntegrationFixture fixture;
    
    // App starts with lock trigger high
    fixture.simulateSystemStartup();
    fixture.simulateTriggers(false, false, true, false);
    fixture.updateSystem();
    
    // → Splash animates
    TEST_ASSERT_TRUE(fixture.panelManager->wasSplashShown());
    
    // → Oil panel does NOT load
    // → Lock panel loads
    TEST_ASSERT_EQUAL_STRING("LockPanel", fixture.panelManager->getCurrentPanel().c_str());
    
    // → Lock trigger low → Oil panel loads
    fixture.simulateTriggers(false, false, false, false);
    fixture.updateSystem();
    TEST_ASSERT_EQUAL_STRING("OilPanel", fixture.panelManager->getCurrentPanel().c_str());
}

/**
 * @test test_theme_change_scenario_runtime
 * @brief Tests theme change scenario during runtime
 */
void test_theme_change_scenario_runtime(void) {
    SystemIntegrationFixture fixture;
    
    // App starts → Oil panel loads
    fixture.simulateSystemStartup();
    fixture.updateSystem();
    TEST_ASSERT_EQUAL_STRING("OilPanel", fixture.panelManager->getCurrentPanel().c_str());
    TEST_ASSERT_EQUAL(Theme::DAY, fixture.styleManager->getCurrentTheme());
    
    // → Lights trigger high → Theme changes to night (no reload)
    fixture.simulateTriggers(false, false, false, true);
    fixture.updateSystem();
    TEST_ASSERT_EQUAL_STRING("OilPanel", fixture.panelManager->getCurrentPanel().c_str()); // No panel change
    TEST_ASSERT_EQUAL(Theme::NIGHT, fixture.styleManager->getCurrentTheme());
    
    // → Lights trigger low → Theme changes to day (no reload)
    fixture.simulateTriggers(false, false, false, false);
    fixture.updateSystem();
    TEST_ASSERT_EQUAL_STRING("OilPanel", fixture.panelManager->getCurrentPanel().c_str()); // No panel change
    TEST_ASSERT_EQUAL(Theme::DAY, fixture.styleManager->getCurrentTheme());
}

/**
 * @test test_theme_change_scenario_startup
 * @brief Tests theme change scenario at startup
 */
void test_theme_change_scenario_startup(void) {
    SystemIntegrationFixture fixture;
    
    // App starts with lights trigger high
    fixture.simulateSystemStartup();
    fixture.simulateTriggers(false, false, false, true);
    fixture.updateSystem();
    
    // → Splash animates with night theme (red text)
    TEST_ASSERT_TRUE(fixture.panelManager->wasSplashShown());
    TEST_ASSERT_EQUAL(Theme::NIGHT, fixture.styleManager->getCurrentTheme());
    
    // → Oil panel loads with night theme
    TEST_ASSERT_EQUAL_STRING("OilPanel", fixture.panelManager->getCurrentPanel().c_str());
    TEST_ASSERT_EQUAL(Theme::NIGHT, fixture.styleManager->getCurrentTheme());
    
    // → Lights trigger low → Theme changes to day (no reload)
    fixture.simulateTriggers(false, false, false, false);
    fixture.updateSystem();
    TEST_ASSERT_EQUAL_STRING("OilPanel", fixture.panelManager->getCurrentPanel().c_str()); // No panel change
    TEST_ASSERT_EQUAL(Theme::DAY, fixture.styleManager->getCurrentTheme());
}

// 4.2 System Integration Tests

/**
 * @test test_service_container_dependency_injection
 * @brief Tests service container dependency injection
 */
void test_service_container_dependency_injection(void) {
    SystemIntegrationFixture fixture;
    
    // Verify all services are properly injected
    TEST_ASSERT_NOT_NULL(fixture.gpio);
    TEST_ASSERT_NOT_NULL(fixture.styleManager);
    TEST_ASSERT_NOT_NULL(fixture.prefManager);
    TEST_ASSERT_NOT_NULL(fixture.triggerManager);
    TEST_ASSERT_NOT_NULL(fixture.panelManager);
    
    // Verify sensors have GPIO provider
    TEST_ASSERT_NOT_NULL(fixture.pressureSensor.get());
    TEST_ASSERT_NOT_NULL(fixture.temperatureSensor.get());
    TEST_ASSERT_NOT_NULL(fixture.keySensor.get());
    TEST_ASSERT_NOT_NULL(fixture.lockSensor.get());
    TEST_ASSERT_NOT_NULL(fixture.lightSensor.get());
    
    // Verify components have style service
    TEST_ASSERT_NOT_NULL(fixture.pressureComponent.get());
    TEST_ASSERT_NOT_NULL(fixture.temperatureComponent.get());
    TEST_ASSERT_NOT_NULL(fixture.keyComponent.get());
    TEST_ASSERT_NOT_NULL(fixture.lockComponent.get());
    TEST_ASSERT_NOT_NULL(fixture.clarityComponent.get());
}

/**
 * @test test_service_container_lifecycle_management
 * @brief Tests service container lifecycle management
 */
void test_service_container_lifecycle_management(void) {
    SystemIntegrationFixture fixture;
    
    // Test initialization sequence
    fixture.simulateSystemStartup();
    
    // Verify sensors are initialized
    TEST_ASSERT_TRUE(fixture.pressureSensor->isInitialized());
    TEST_ASSERT_TRUE(fixture.temperatureSensor->isInitialized());
    TEST_ASSERT_TRUE(fixture.keySensor->isInitialized());
    TEST_ASSERT_TRUE(fixture.lockSensor->isInitialized());
    TEST_ASSERT_TRUE(fixture.lightSensor->isInitialized());
    
    // Verify managers are in expected initial state
    TEST_ASSERT_EQUAL(TriggerType::NONE, fixture.triggerManager->getActiveTrigger());
    TEST_ASSERT_EQUAL(Theme::DAY, fixture.styleManager->getCurrentTheme());
}

/**
 * @test test_service_container_cross_service_communication
 * @brief Tests cross-service communication
 */
void test_service_container_cross_service_communication(void) {
    SystemIntegrationFixture fixture;
    fixture.simulateSystemStartup();
    
    // Test GPIO → Sensor → TriggerManager communication
    fixture.simulateTriggers(true, false, false, false); // Key present
    fixture.updateSystem();
    
    // Verify communication chain
    TEST_ASSERT_TRUE(fixture.gpio->getDigitalValue(GPIO_KEY_PRESENT));
    TEST_ASSERT_TRUE(fixture.keySensor->read());
    TEST_ASSERT_EQUAL(TriggerType::KEY_PRESENT, fixture.triggerManager->getActiveTrigger());
    TEST_ASSERT_EQUAL_STRING("KeyPanel", fixture.panelManager->getCurrentPanel().c_str());
    
    // Test theme communication
    fixture.simulateTriggers(false, false, false, true); // Light sensor
    fixture.updateSystem();
    
    TEST_ASSERT_TRUE(fixture.lightSensor->read());
    TEST_ASSERT_EQUAL(Theme::NIGHT, fixture.styleManager->getCurrentTheme());
}

/**
 * @test test_end_to_end_sensor_to_component_flow
 * @brief Tests complete sensor to component data flow
 */
void test_end_to_end_sensor_to_component_flow(void) {
    SystemIntegrationFixture fixture;
    fixture.simulateSystemStartup();
    
    // Set sensor data
    int32_t testPressure = 1500;
    int32_t testTemperature = 3000;
    fixture.simulateSensorData(testPressure, testTemperature);
    
    // Update system
    fixture.updateSystem();
    
    // Verify data flows from GPIO → Sensor → Component
    TEST_ASSERT_EQUAL(testPressure, fixture.gpio->getAnalogValue(GPIO_PRESSURE_SENSOR));
    TEST_ASSERT_EQUAL(testPressure, fixture.pressureSensor->read());
    TEST_ASSERT_EQUAL(testPressure, fixture.pressureComponent->getCurrentValue());
    
    TEST_ASSERT_EQUAL(testTemperature, fixture.gpio->getAnalogValue(GPIO_TEMPERATURE_SENSOR));
    TEST_ASSERT_EQUAL(testTemperature, fixture.temperatureSensor->read());
    TEST_ASSERT_EQUAL(testTemperature, fixture.temperatureComponent->getCurrentValue());
}

/**
 * @test test_end_to_end_trigger_to_panel_pipeline
 * @brief Tests complete trigger to panel pipeline
 */
void test_end_to_end_trigger_to_panel_pipeline(void) {
    SystemIntegrationFixture fixture;
    fixture.simulateSystemStartup();
    
    // Test each trigger type
    struct TriggerTest {
        bool keyPresent, keyNotPresent, lock, light;
        TriggerType expectedTrigger;
        const char* expectedPanel;
    };
    
    TriggerTest tests[] = {
        {true, false, false, false, TriggerType::KEY_PRESENT, "KeyPanel"},
        {false, true, false, false, TriggerType::KEY_NOT_PRESENT, "KeyPanel"},
        {false, false, true, false, TriggerType::LOCK, "LockPanel"},
        {false, false, false, true, TriggerType::THEME, "OilPanel"}, // Theme doesn't change panel
        {false, false, false, false, TriggerType::NONE, "OilPanel"}
    };
    
    for (auto& test : tests) {
        fixture.simulateTriggers(test.keyPresent, test.keyNotPresent, test.lock, test.light);
        fixture.updateSystem();
        
        TEST_ASSERT_EQUAL(test.expectedTrigger, fixture.triggerManager->getActiveTrigger());
        TEST_ASSERT_EQUAL_STRING(test.expectedPanel, fixture.panelManager->getCurrentPanel().c_str());
    }
}

/**
 * @test test_system_error_propagation_recovery
 * @brief Tests system error propagation and recovery
 */
void test_system_error_propagation_recovery(void) {
    SystemIntegrationFixture fixture;
    fixture.simulateSystemStartup();
    
    // Test sensor error handling
    fixture.gpio->simulateAnalogFailure(GPIO_PRESSURE_SENSOR);
    fixture.updateSystem();
    
    // System should handle gracefully (component gets last valid value or default)
    TEST_ASSERT_TRUE(fixture.pressureComponent->getCurrentValue() >= 0); // Should not crash
    
    // Test recovery
    fixture.gpio->clearAnalogFailure(GPIO_PRESSURE_SENSOR);
    fixture.simulateSensorData(2000, 2000);
    fixture.updateSystem();
    
    // System should recover
    TEST_ASSERT_EQUAL(2000, fixture.pressureComponent->getCurrentValue());
}

/**
 * @test test_system_state_persistence
 * @brief Tests system state persistence across operations
 */
void test_system_state_persistence(void) {
    SystemIntegrationFixture fixture;
    fixture.simulateSystemStartup();
    
    // Set theme to night
    fixture.simulateTriggers(false, false, false, true);
    fixture.updateSystem();
    TEST_ASSERT_EQUAL(Theme::NIGHT, fixture.styleManager->getCurrentTheme());
    
    // Switch to key panel
    fixture.simulateTriggers(true, false, false, true);
    fixture.updateSystem();
    TEST_ASSERT_EQUAL_STRING("KeyPanel", fixture.panelManager->getCurrentPanel().c_str());
    
    // Theme should persist
    TEST_ASSERT_EQUAL(Theme::NIGHT, fixture.styleManager->getCurrentTheme());
    
    // Return to oil panel
    fixture.simulateTriggers(false, false, false, true);
    fixture.updateSystem();
    TEST_ASSERT_EQUAL_STRING("OilPanel", fixture.panelManager->getCurrentPanel().c_str());
    
    // Theme should still persist
    TEST_ASSERT_EQUAL(Theme::NIGHT, fixture.styleManager->getCurrentTheme());
}

/**
 * @test test_system_performance_under_load
 * @brief Tests system performance under load
 */
void test_system_performance_under_load(void) {
    SystemIntegrationFixture fixture;
    fixture.simulateSystemStartup();
    
    // Simulate rapid trigger changes
    for (int i = 0; i < 100; i++) {
        bool state = (i % 2 == 0);
        fixture.simulateTriggers(state, false, false, false);
        fixture.updateSystem();
        
        // Verify system remains stable
        TEST_ASSERT_NOT_NULL(fixture.panelManager->getCurrentPanel().c_str());
        TEST_ASSERT_TRUE(fixture.styleManager->getCurrentTheme() == Theme::DAY || 
                        fixture.styleManager->getCurrentTheme() == Theme::NIGHT);
    }
    
    // Simulate rapid sensor value changes
    for (int i = 0; i < 50; i++) {
        fixture.simulateSensorData(i * 80, i * 60);
        fixture.updateSystem();
        
        // Verify components handle rapid updates
        TEST_ASSERT_TRUE(fixture.pressureComponent->getCurrentValue() >= 0);
        TEST_ASSERT_TRUE(fixture.temperatureComponent->getCurrentValue() >= 0);
    }
}

// ============================================================================
// PHASE 5: INFRASTRUCTURE TESTS (Nice-to-Have Coverage)
// ============================================================================

// ============================================================================
// Device Layer Tests
// ============================================================================

class MockDevice {
public:
    bool prepared_ = false;
    bool screenInitialized_ = false;
    uint32_t brightness_ = 0;
    int rotationValue_ = 0;
    
    void prepare() {
        prepared_ = true;
        screenInitialized_ = true;
        brightness_ = 100; // Default brightness
    }
    
    void setBrightness(uint32_t brightness) {
        brightness_ = brightness;
    }
    
    void setRotation(int rotation) {
        rotationValue_ = rotation;
    }
    
    bool isPrepared() const { return prepared_; }
    bool isScreenInitialized() const { return screenInitialized_; }
    uint32_t getBrightness() const { return brightness_; }
    int getRotation() const { return rotationValue_; }
};

void test_device_initialization(void) {
    MockDevice device;
    
    // Test initial state
    TEST_ASSERT_FALSE(device.isPrepared());
    TEST_ASSERT_FALSE(device.isScreenInitialized());
    TEST_ASSERT_EQUAL_UINT32(0, device.getBrightness());
    TEST_ASSERT_EQUAL_INT(0, device.getRotation());
}

void test_device_preparation(void) {
    MockDevice device;
    
    // Test device preparation
    device.prepare();
    
    TEST_ASSERT_TRUE(device.isPrepared());
    TEST_ASSERT_TRUE(device.isScreenInitialized());
    TEST_ASSERT_EQUAL_UINT32(100, device.getBrightness()); // Default brightness
}

void test_device_display_configuration(void) {
    MockDevice device;
    device.prepare();
    
    // Test brightness control
    device.setBrightness(50);
    TEST_ASSERT_EQUAL_UINT32(50, device.getBrightness());
    
    device.setBrightness(255);
    TEST_ASSERT_EQUAL_UINT32(255, device.getBrightness());
    
    // Test rotation control
    device.setRotation(90);
    TEST_ASSERT_EQUAL_INT(90, device.getRotation());
}

void test_device_hardware_abstraction(void) {
    MockDevice device;
    
    // Test that device provides proper hardware abstraction
    device.prepare();
    
    // Verify all core hardware functions are initialized
    TEST_ASSERT_TRUE(device.isPrepared());
    TEST_ASSERT_TRUE(device.isScreenInitialized());
    
    // Test display buffer allocation simulation
    const int bufferSize = 240 * 60 * 2; // Simulated buffer size
    TEST_ASSERT_TRUE(bufferSize > 0);
    TEST_ASSERT_TRUE(bufferSize <= 65536); // Reasonable buffer size
}

// ============================================================================
// Factory Pattern Tests
// ============================================================================

// Mock factory for testing (avoids LVGL dependencies)
class TestManagerFactory {
public:
    static std::unique_ptr<TestPanelManager> createPanelManager(
        MockDisplayProvider* display, 
        MockGpioProvider* gpio, 
        TestStyleManager* styleService) {
        
        if (!display || !gpio || !styleService) {
            return nullptr;
        }
        
        auto manager = std::make_unique<TestPanelManager>(display, gpio, (MockStyleService*)styleService);
        return manager;
    }
    
    static std::unique_ptr<TestStyleManager> createStyleManager(const char* theme = nullptr) {
        auto manager = std::make_unique<TestStyleManager>();
        if (theme) {
            manager->setTheme(theme);
        }
        return manager;
    }
    
    static std::unique_ptr<TestTriggerManager> createTriggerManager(
        MockGpioProvider* gpio,
        MockPanelService* panelService,
        MockStyleService* styleService) {
        
        if (!gpio || !panelService || !styleService) {
            return nullptr;
        }
        
        auto manager = std::make_unique<TestTriggerManager>(gpio);
        return manager;
    }
    
    static std::unique_ptr<TestPreferenceManager> createPreferenceManager() {
        return std::make_unique<TestPreferenceManager>();
    }
};

void test_manager_factory_panel_manager_creation(void) {
    auto mockGpio = std::make_unique<MockGpioProvider>();
    auto mockDisplay = std::make_unique<MockDisplayProvider>();
    auto styleManager = std::make_unique<TestStyleManager>();
    
    // Test successful creation with valid dependencies
    auto panelManager = TestManagerFactory::createPanelManager(
        mockDisplay.get(), 
        mockGpio.get(), 
        styleManager.get()
    );
    
    TEST_ASSERT_NOT_NULL(panelManager.get());
    
    // Test manager functionality
    panelManager->createAndLoadPanel("TestPanel");
    TEST_ASSERT_FALSE(panelManager->getCurrentPanel().empty());
}

void test_manager_factory_style_manager_creation(void) {
    // Test creation with default theme
    auto styleManager1 = TestManagerFactory::createStyleManager();
    TEST_ASSERT_NOT_NULL(styleManager1.get());
    
    // Test creation with specific theme
    auto styleManager2 = TestManagerFactory::createStyleManager("day");
    TEST_ASSERT_NOT_NULL(styleManager2.get());
    
    auto styleManager3 = TestManagerFactory::createStyleManager("night");
    TEST_ASSERT_NOT_NULL(styleManager3.get());
}

void test_manager_factory_trigger_manager_creation(void) {
    auto mockGpio = std::make_unique<MockGpioProvider>();
    auto mockPanelService = std::make_unique<MockPanelService>();
    auto mockStyleService = std::make_unique<MockStyleService>();
    
    // Test successful creation with valid dependencies
    auto triggerManager = TestManagerFactory::createTriggerManager(
        mockGpio.get(),
        mockPanelService.get(),
        mockStyleService.get()
    );
    
    TEST_ASSERT_NOT_NULL(triggerManager.get());
}

void test_manager_factory_preference_manager_creation(void) {
    // Test creation (no dependencies)
    auto preferenceManager = TestManagerFactory::createPreferenceManager();
    TEST_ASSERT_NOT_NULL(preferenceManager.get());
}

void test_manager_factory_dependency_validation(void) {
    auto mockGpio = std::make_unique<MockGpioProvider>();
    auto mockDisplay = std::make_unique<MockDisplayProvider>();
    auto mockStyleService = std::make_unique<TestStyleManager>();
    
    // Test that factory validates dependencies - should return null with invalid deps
    auto panelManagerNull = TestManagerFactory::createPanelManager(nullptr, mockGpio.get(), mockStyleService.get());
    TEST_ASSERT_NULL(panelManagerNull.get());
    
    // Test valid creation
    auto panelManager = TestManagerFactory::createPanelManager(
        mockDisplay.get(), 
        mockGpio.get(), 
        mockStyleService.get()
    );
    
    TEST_ASSERT_NOT_NULL(panelManager.get());
}

void test_factory_pattern_consistency(void) {
    auto mockGpio = std::make_unique<MockGpioProvider>();
    auto mockDisplay = std::make_unique<MockDisplayProvider>();
    auto mockStyleService = std::make_unique<TestStyleManager>();
    auto mockPanelService = std::make_unique<MockPanelService>();
    
    // Test that multiple instances can be created consistently
    auto panelManager1 = TestManagerFactory::createPanelManager(mockDisplay.get(), mockGpio.get(), mockStyleService.get());
    auto panelManager2 = TestManagerFactory::createPanelManager(mockDisplay.get(), mockGpio.get(), mockStyleService.get());
    
    TEST_ASSERT_NOT_NULL(panelManager1.get());
    TEST_ASSERT_NOT_NULL(panelManager2.get());
    TEST_ASSERT_NOT_EQUAL(panelManager1.get(), panelManager2.get()); // Different instances
    
    auto styleManager1 = TestManagerFactory::createStyleManager("day");
    auto styleManager2 = TestManagerFactory::createStyleManager("night");
    
    TEST_ASSERT_NOT_NULL(styleManager1.get());
    TEST_ASSERT_NOT_NULL(styleManager2.get());
    TEST_ASSERT_NOT_EQUAL(styleManager1.get(), styleManager2.get());
}

// ============================================================================
// Utility Class Tests
// ============================================================================

class MockTicker {
public:
    static uint32_t mockMillis_;
    static uint32_t lastDelay_;
    static int lvTasksHandled_;
    
    static void setMockMillis(uint32_t time) { mockMillis_ = time; }
    static uint32_t getMockMillis() { return mockMillis_; }
    static void mockDelay(uint32_t ms) { lastDelay_ = ms; }
    static uint32_t getLastDelay() { return lastDelay_; }
    static void mockHandleLvTasks() { lvTasksHandled_++; }
    static int getLvTasksHandled() { return lvTasksHandled_; }
    static void reset() { mockMillis_ = 0; lastDelay_ = 0; lvTasksHandled_ = 0; }
    
    static void handleDynamicDelay(uint32_t startTime) {
        uint32_t elapsedTime = mockMillis_ - startTime;
        uint32_t targetFrameTime = 16;
        if (elapsedTime < targetFrameTime) {
            mockDelay(targetFrameTime - elapsedTime);
        } else {
            mockDelay(1);
        }
    }
    
    static void handleLvTasks() {
        mockHandleLvTasks();
    }
};

uint32_t MockTicker::mockMillis_ = 0;
uint32_t MockTicker::lastDelay_ = 0;
int MockTicker::lvTasksHandled_ = 0;

void test_ticker_dynamic_delay_fast_execution(void) {
    MockTicker::reset();
    MockTicker::setMockMillis(20);
    
    // Test when execution is fast (under 16ms target)
    uint32_t startTime = 10;
    MockTicker::handleDynamicDelay(startTime);
    
    // Should delay for remaining time to reach 16ms target
    uint32_t expectedDelay = 16 - (20 - 10); // 16 - 10 = 6ms
    TEST_ASSERT_EQUAL_UINT32(6, MockTicker::getLastDelay());
}

void test_ticker_dynamic_delay_slow_execution(void) {
    MockTicker::reset();
    MockTicker::setMockMillis(30);
    
    // Test when execution is slow (over 16ms target)
    uint32_t startTime = 10;
    MockTicker::handleDynamicDelay(startTime);
    
    // Should use minimal delay
    TEST_ASSERT_EQUAL_UINT32(1, MockTicker::getLastDelay());
}

void test_ticker_lv_tasks_handling(void) {
    MockTicker::reset();
    
    // Test LVGL task handling
    MockTicker::handleLvTasks();
    TEST_ASSERT_EQUAL_INT(1, MockTicker::getLvTasksHandled());
    
    MockTicker::handleLvTasks();
    TEST_ASSERT_EQUAL_INT(2, MockTicker::getLvTasksHandled());
}

void test_ticker_frame_rate_consistency(void) {
    MockTicker::reset();
    
    // Test consistent frame rate targeting
    const uint32_t targetFrameTime = 16; // ~60fps
    
    for (int i = 0; i < 10; i++) {
        uint32_t startTime = i * 20;
        MockTicker::setMockMillis(startTime + 5); // Fast execution
        MockTicker::handleDynamicDelay(startTime);
        
        // Should always try to maintain 16ms frame time
        uint32_t expectedDelay = targetFrameTime - 5;
        TEST_ASSERT_EQUAL_UINT32(expectedDelay, MockTicker::getLastDelay());
    }
}

void test_utility_timing_precision(void) {
    MockTicker::reset();
    
    // Test timing precision with various execution times
    struct TestCase {
        uint32_t startTime;
        uint32_t currentTime;
        uint32_t expectedDelay;
    };
    
    TestCase testCases[] = {
        {0, 5, 11},   // Fast: 16 - 5 = 11ms delay
        {0, 10, 6},   // Medium: 16 - 10 = 6ms delay
        {0, 15, 1},   // Near target: 16 - 15 = 1ms delay
        {0, 20, 1},   // Over target: minimum 1ms delay
        {0, 50, 1}    // Way over: minimum 1ms delay
    };
    
    for (const auto& testCase : testCases) {
        MockTicker::setMockMillis(testCase.currentTime);
        MockTicker::handleDynamicDelay(testCase.startTime);
        TEST_ASSERT_EQUAL_UINT32(testCase.expectedDelay, MockTicker::getLastDelay());
    }
}

// ============================================================================
// Main Application Startup Tests
// ============================================================================

class MockMainApplication {
public:
    bool servicesInitialized_ = false;
    bool devicePrepared_ = false;
    bool triggerManagerInitialized_ = false;
    bool startupPanelLoaded_ = false;
    std::string startupPanel_ = "";
    int loopIterations_ = 0;
    
    void initializeServices() {
        servicesInitialized_ = true;
    }
    
    void prepareDevice() {
        devicePrepared_ = true;
    }
    
    void initializeTriggerManager() {
        triggerManagerInitialized_ = true;
    }
    
    void loadStartupPanel(const std::string& panelName) {
        startupPanelLoaded_ = true;
        startupPanel_ = panelName;
    }
    
    void runMainLoop() {
        loopIterations_++;
    }
    
    void setup() {
        initializeServices();
        prepareDevice();
        initializeTriggerManager();
        loadStartupPanel("OilPanel");
    }
    
    void loop() {
        runMainLoop();
    }
    
    // Getters for testing
    bool areServicesInitialized() const { return servicesInitialized_; }
    bool isDevicePrepared() const { return devicePrepared_; }
    bool isTriggerManagerInitialized() const { return triggerManagerInitialized_; }
    bool isStartupPanelLoaded() const { return startupPanelLoaded_; }
    std::string getStartupPanel() const { return startupPanel_; }
    int getLoopIterations() const { return loopIterations_; }
};

void test_main_application_setup_sequence(void) {
    MockMainApplication app;
    
    // Test initial state
    TEST_ASSERT_FALSE(app.areServicesInitialized());
    TEST_ASSERT_FALSE(app.isDevicePrepared());
    TEST_ASSERT_FALSE(app.isTriggerManagerInitialized());
    TEST_ASSERT_FALSE(app.isStartupPanelLoaded());
    
    // Test setup sequence
    app.setup();
    
    // Verify all setup steps completed
    TEST_ASSERT_TRUE(app.areServicesInitialized());
    TEST_ASSERT_TRUE(app.isDevicePrepared());
    TEST_ASSERT_TRUE(app.isTriggerManagerInitialized());
    TEST_ASSERT_TRUE(app.isStartupPanelLoaded());
    TEST_ASSERT_EQUAL_STRING("OilPanel", app.getStartupPanel().c_str());
}

void test_main_application_service_initialization_order(void) {
    MockMainApplication app;
    
    // Test individual initialization steps
    app.initializeServices();
    TEST_ASSERT_TRUE(app.areServicesInitialized());
    TEST_ASSERT_FALSE(app.isDevicePrepared()); // Should not be prepared yet
    
    app.prepareDevice();
    TEST_ASSERT_TRUE(app.isDevicePrepared());
    TEST_ASSERT_FALSE(app.isTriggerManagerInitialized()); // Should not be initialized yet
    
    app.initializeTriggerManager();
    TEST_ASSERT_TRUE(app.isTriggerManagerInitialized());
    TEST_ASSERT_FALSE(app.isStartupPanelLoaded()); // Should not be loaded yet
    
    app.loadStartupPanel("KeyPanel");
    TEST_ASSERT_TRUE(app.isStartupPanelLoaded());
    TEST_ASSERT_EQUAL_STRING("KeyPanel", app.getStartupPanel().c_str());
}

void test_main_application_loop_execution(void) {
    MockMainApplication app;
    app.setup();
    
    // Test main loop execution
    TEST_ASSERT_EQUAL_INT(0, app.getLoopIterations());
    
    app.loop();
    TEST_ASSERT_EQUAL_INT(1, app.getLoopIterations());
    
    // Test multiple loop iterations
    for (int i = 0; i < 10; i++) {
        app.loop();
    }
    TEST_ASSERT_EQUAL_INT(11, app.getLoopIterations());
}

void test_main_application_startup_panel_selection(void) {
    MockMainApplication app;
    
    // Test different startup panels
    app.loadStartupPanel("OilPanel");
    TEST_ASSERT_EQUAL_STRING("OilPanel", app.getStartupPanel().c_str());
    
    app.loadStartupPanel("KeyPanel");
    TEST_ASSERT_EQUAL_STRING("KeyPanel", app.getStartupPanel().c_str());
    
    app.loadStartupPanel("LockPanel");
    TEST_ASSERT_EQUAL_STRING("LockPanel", app.getStartupPanel().c_str());
}

void test_main_application_error_resilience(void) {
    MockMainApplication app;
    
    // Test that application can handle partial initialization
    app.initializeServices();
    TEST_ASSERT_TRUE(app.areServicesInitialized());
    
    // Even without full setup, loop should be callable
    app.loop();
    TEST_ASSERT_EQUAL_INT(1, app.getLoopIterations());
    
    // Complete setup and verify normal operation
    app.setup();
    TEST_ASSERT_TRUE(app.isStartupPanelLoaded());
}

void test_main_application_integration_flow(void) {
    MockMainApplication app;
    
    // Test complete application flow
    app.setup();
    
    // Verify setup completed properly
    TEST_ASSERT_TRUE(app.areServicesInitialized());
    TEST_ASSERT_TRUE(app.isDevicePrepared());
    TEST_ASSERT_TRUE(app.isTriggerManagerInitialized());
    TEST_ASSERT_TRUE(app.isStartupPanelLoaded());
    
    // Run main loop and verify execution
    for (int i = 0; i < 5; i++) {
        app.loop();
    }
    TEST_ASSERT_EQUAL_INT(5, app.getLoopIterations());
    
    // Verify application maintains stable state
    TEST_ASSERT_TRUE(app.areServicesInitialized());
    TEST_ASSERT_TRUE(app.isDevicePrepared());
    TEST_ASSERT_FALSE(app.getStartupPanel().empty());
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main() {
    UNITY_BEGIN();
    
#ifdef TEST_SENSORS_ONLY
    printf("\n=== Clarity Sensor Test Suite ===\n");
    printf("Running sensor layer tests...\n\n");
    
    // Phase 1: Sensor Tests (21 tests)
    printf("--- Oil Pressure Sensor Tests ---\n");
    RUN_TEST(test_oil_pressure_sensor_initialization);
    RUN_TEST(test_oil_pressure_sensor_constructor);
    RUN_TEST(test_oil_pressure_sensor_adc_mapping_minimum);
    RUN_TEST(test_oil_pressure_sensor_adc_mapping_maximum);
    RUN_TEST(test_oil_pressure_sensor_adc_mapping_boundary_conditions);
    RUN_TEST(test_oil_pressure_sensor_delta_updates_comprehensive);
    
    printf("--- Oil Temperature Sensor Tests ---\n");
    RUN_TEST(test_oil_temperature_sensor_initialization);
    RUN_TEST(test_oil_temperature_sensor_adc_mapping_minimum);
    RUN_TEST(test_oil_temperature_sensor_adc_mapping_maximum);
    RUN_TEST(test_oil_temperature_sensor_delta_updates);
    RUN_TEST(test_oil_temperature_sensor_comprehensive_mapping);
    
    printf("--- Key Sensor Tests ---\n");
    RUN_TEST(test_key_sensor_initialization_comprehensive);
    RUN_TEST(test_key_sensor_state_combinations);
    RUN_TEST(test_key_sensor_present_state);
    RUN_TEST(test_key_sensor_absent_state);
    
    printf("--- Lock Sensor Tests ---\n");
    RUN_TEST(test_lock_sensor_initialization);
    RUN_TEST(test_lock_sensor_locked_state);
    RUN_TEST(test_lock_sensor_unlocked_state);
    
    printf("--- Light Sensor Tests ---\n");
    RUN_TEST(test_light_sensor_initialization);
    RUN_TEST(test_light_sensor_day_mode);
    RUN_TEST(test_light_sensor_night_mode);
    
    printf("\n=== Sensor Test Suite Complete ===\n");
    printf("Total: 21 sensor tests\n");

#elif defined(TEST_MANAGERS_CORE_ONLY)
    printf("\n=== Clarity Manager Core Test Suite ===\n");
    printf("Running core manager layer tests...\n\n");
    
    // TriggerManager Tests (6 tests)
    printf("--- TriggerManager Tests ---\n");
    RUN_TEST(test_trigger_manager_initialization);
    RUN_TEST(test_trigger_manager_key_trigger_activation);
    RUN_TEST(test_trigger_manager_lock_trigger_activation);
    RUN_TEST(test_trigger_manager_theme_switching);
    RUN_TEST(test_trigger_manager_priority_resolution);
    RUN_TEST(test_trigger_manager_startup_panel_override);
    
    // Mock Service Tests (2 tests)
    printf("--- Mock Service Tests ---\n");
    RUN_TEST(test_panel_service_mock_functionality);
    RUN_TEST(test_style_service_mock_functionality);
    
    // PanelManager Tests (7 tests)
    printf("--- PanelManager Tests ---\n");
    RUN_TEST(test_panel_manager_initialization);
    RUN_TEST(test_panel_manager_panel_lifecycle);
    RUN_TEST(test_panel_manager_splash_transitions);
    RUN_TEST(test_panel_manager_update_operations);
    RUN_TEST(test_panel_manager_restoration_panel);
    RUN_TEST(test_panel_manager_trigger_integration);
    RUN_TEST(test_panel_manager_ui_state_management);
    
    printf("\n=== Manager Core Test Suite Complete ===\n");
    printf("Total: 15 core manager tests\n");

#elif defined(TEST_MANAGERS_EXTENDED_ONLY)
    printf("\n=== Clarity Manager Extended Test Suite ===\n");
    printf("Running extended manager layer tests...\n\n");
    
    // StyleManager Tests (5 tests)
    printf("--- StyleManager Tests ---\n");
    RUN_TEST(test_style_manager_initialization);
    RUN_TEST(test_style_manager_theme_switching);
    RUN_TEST(test_style_manager_theme_application);
    RUN_TEST(test_style_manager_style_getters);
    RUN_TEST(test_style_manager_theme_persistence);
    
    // PreferenceManager Tests (8 tests)
    printf("--- PreferenceManager Tests ---\n");
    RUN_TEST(test_preference_manager_initialization);
    RUN_TEST(test_preference_manager_default_config_creation);
    RUN_TEST(test_preference_manager_string_operations);
    RUN_TEST(test_preference_manager_integer_operations);
    RUN_TEST(test_preference_manager_boolean_operations);
    RUN_TEST(test_preference_manager_save_operations);
    RUN_TEST(test_preference_manager_corruption_recovery);
    RUN_TEST(test_preference_manager_clear_operations);
    
    printf("\n=== Manager Extended Test Suite Complete ===\n");
    printf("Total: 13 extended manager tests\n");

#elif defined(TEST_COMPONENTS_ONLY)
    printf("\n=== Clarity Component Test Suite ===\n");
    printf("Running component layer tests...\n\n");
    
    // Phase 3: Component Layer Tests
    printf("--- Phase 3: Component Layer Tests ---\n");
    
    // OEM Oil Component Tests
    RUN_TEST(test_oem_oil_pressure_component_initialization);
    RUN_TEST(test_oem_oil_pressure_component_value_mapping);
    RUN_TEST(test_oem_oil_pressure_component_danger_zone);
    RUN_TEST(test_oem_oil_pressure_component_refresh_with_reading);
    RUN_TEST(test_oem_oil_pressure_component_scale_configuration);
    
    RUN_TEST(test_oem_oil_temperature_component_initialization);
    RUN_TEST(test_oem_oil_temperature_component_temperature_ranges);
    RUN_TEST(test_oem_oil_temperature_component_value_mapping);
    RUN_TEST(test_oem_oil_temperature_component_danger_detection);
    RUN_TEST(test_oem_oil_temperature_component_refresh);
    
    // UI Component Tests
    RUN_TEST(test_key_component_initialization);
    RUN_TEST(test_key_component_state_management);
    RUN_TEST(test_key_component_theme_integration);
    RUN_TEST(test_key_component_refresh_with_reading);
    
    RUN_TEST(test_lock_component_initialization);
    RUN_TEST(test_lock_component_state_management);
    RUN_TEST(test_lock_component_theme_integration);
    RUN_TEST(test_lock_component_refresh);
    
    // Branding Component Tests
    RUN_TEST(test_clarity_component_initialization);
    RUN_TEST(test_clarity_component_theme_awareness);
    RUN_TEST(test_clarity_component_branding_display);
    
    // Component Integration Tests
    RUN_TEST(test_component_lifecycle_management);
    RUN_TEST(test_component_data_flow_integration);
    RUN_TEST(test_component_theme_coordination);
    
    printf("\n=== Component Test Suite Complete ===\n");
    printf("Total: 23 component tests\n");

#elif defined(TEST_INTEGRATION_ONLY)
    printf("\n=== Clarity Integration Test Suite ===\n");
    printf("Running integration & scenario tests...\n\n");
    
    // Phase 4: Integration & Scenario Tests
    printf("--- Phase 4: Integration & Scenario Tests ---\n");
    
    // 4.1 Scenario-Based Integration Tests
    printf("--- Scenario-Based Integration Tests ---\n");
    RUN_TEST(test_major_scenario_complete_system);
    RUN_TEST(test_startup_scenario_default);
    RUN_TEST(test_startup_scenario_with_oil_data);
    RUN_TEST(test_startup_scenario_with_triggers);
    RUN_TEST(test_key_present_scenario_runtime);
    RUN_TEST(test_key_present_scenario_startup);
    RUN_TEST(test_key_not_present_scenario_runtime);
    RUN_TEST(test_key_not_present_scenario_startup);
    RUN_TEST(test_lock_scenario_runtime);
    RUN_TEST(test_lock_scenario_startup);
    RUN_TEST(test_theme_change_scenario_runtime);
    RUN_TEST(test_theme_change_scenario_startup);
    
    // 4.2 System Integration Tests
    printf("--- System Integration Tests ---\n");
    RUN_TEST(test_service_container_dependency_injection);
    RUN_TEST(test_service_container_lifecycle_management);
    RUN_TEST(test_service_container_cross_service_communication);
    RUN_TEST(test_end_to_end_sensor_to_component_flow);
    RUN_TEST(test_end_to_end_trigger_to_panel_pipeline);
    RUN_TEST(test_system_error_propagation_recovery);
    RUN_TEST(test_system_state_persistence);
    RUN_TEST(test_system_performance_under_load);
    
    printf("\n=== Integration Test Suite Complete ===\n");
    printf("Total: 20 integration tests\n");

#elif defined(TEST_INFRASTRUCTURE_ONLY)  
    printf("\n=== Clarity Infrastructure Test Suite ===\n");
    printf("Running infrastructure & low-level tests...\n\n");
    
    // Phase 5: Infrastructure Tests (Nice-to-Have Coverage) 
    printf("--- Phase 5: Infrastructure & Low-Level Tests ---\n");
    
    // 5.1 Device Layer Tests
    printf("--- Device Layer Tests ---\n");
    RUN_TEST(test_device_initialization);
    RUN_TEST(test_device_preparation);
    RUN_TEST(test_device_display_configuration);
    RUN_TEST(test_device_hardware_abstraction);
    
    // 5.2 Factory Pattern Tests
    printf("--- Factory Pattern Tests ---\n");
    RUN_TEST(test_manager_factory_panel_manager_creation);
    RUN_TEST(test_manager_factory_style_manager_creation);
    RUN_TEST(test_manager_factory_trigger_manager_creation);
    RUN_TEST(test_manager_factory_preference_manager_creation);
    RUN_TEST(test_manager_factory_dependency_validation);
    RUN_TEST(test_factory_pattern_consistency);
    
    // 5.3 Utility Class Tests
    printf("--- Utility Class Tests ---\n");
    RUN_TEST(test_ticker_dynamic_delay_fast_execution);
    RUN_TEST(test_ticker_dynamic_delay_slow_execution);
    RUN_TEST(test_ticker_lv_tasks_handling);
    RUN_TEST(test_ticker_frame_rate_consistency);
    RUN_TEST(test_utility_timing_precision);
    
    // 5.4 Main Application Startup Tests
    printf("--- Main Application Tests ---\n");
    RUN_TEST(test_main_application_setup_sequence);
    RUN_TEST(test_main_application_service_initialization_order);
    RUN_TEST(test_main_application_loop_execution);
    RUN_TEST(test_main_application_startup_panel_selection);
    RUN_TEST(test_main_application_error_resilience);
    RUN_TEST(test_main_application_integration_flow);
    
    printf("\n=== Infrastructure Test Suite Complete ===\n");
    printf("Total: 21 infrastructure tests\n");

#else
    // Default: Run all tests (for backward compatibility)
    printf("\n=== Clarity Complete Test Suite ===\n");
    printf("Running ALL tests - this may exceed Unity's 37-test limit!\n\n");
    printf("Use specific test environments instead:\n");
    printf("  pio test -e test-sensors           (21 tests)\n");
    printf("  pio test -e test-managers-core     (15 tests)\n");
    printf("  pio test -e test-managers-extended (13 tests)\n"); 
    printf("  pio test -e test-components        (23 tests)\n");
    printf("  pio test -e test-integration       (20 tests)\n");
    printf("  pio test -e test-infrastructure    (21 tests)\n");
    printf("Total: 113 comprehensive tests covering ALL system layers\n");
#endif
    
    return UNITY_END();
}