#pragma once

#ifdef UNIT_TESTING

#include "../../include/interfaces/i_panel_service.h"
#include "../../include/interfaces/i_style_service.h"
#include "../../include/interfaces/i_trigger_service.h"
#include "../../include/interfaces/i_display_provider.h"
#include "../../include/interfaces/i_preference_service.h"
#include "../../include/interfaces/i_gpio_provider.h"
#include "../../include/utilities/types.h"
#include <map>
#include <vector>
#include <functional>

// Mock Panel Service
class MockPanelService : public IPanelService {
private:
    std::string currentPanel = "SplashPanel";
    std::map<std::string, bool> panelStates;
    std::vector<std::pair<std::string, uint32_t>> panelHistory;
    UIState uiState = UIState::IDLE;
    std::string restorationPanel = "OemOilPanel";
    
public:
    void init() override {}
    
    void createAndLoadPanel(const char* panelName, 
                           std::function<void()> completionCallback = nullptr,
                           bool isTriggerDriven = false) override {
        currentPanel = panelName;
        panelHistory.push_back({panelName, 0}); // Use 0 for timestamp in mock
        panelStates[panelName] = true;
        if (completionCallback) completionCallback();
    }
    
    void createAndLoadPanelWithSplash(const char* panelName) override {
        // First load splash, then target panel
        createAndLoadPanel("SplashPanel");
        createAndLoadPanel(panelName);
    }
    
    void updatePanel() override {}
    
    void setUiState(UIState state) override {
        uiState = state;
    }
    
    const char* getCurrentPanel() const override {
        return currentPanel.c_str();
    }
    
    const char* getRestorationPanel() const override {
        return restorationPanel.c_str();
    }
    
    void triggerPanelSwitchCallback(const char* triggerId) override {}
    
    // Test utilities
    void reset() {
        currentPanel = "SplashPanel";
        panelStates.clear();
        panelHistory.clear();
        uiState = UIState::IDLE;
    }
    
    const std::vector<std::pair<std::string, uint32_t>>& getPanelHistory() const {
        return panelHistory;
    }
    
    void setPanelVisible(const std::string& panelName, bool visible) {
        panelStates[panelName] = visible;
    }
    
    UIState getUiState() const { return uiState; }
    
    size_t getPanelChangeCount() const {
        return panelHistory.size();
    }
};

// Mock Style Service
class MockStyleService : public IStyleService {
private:
    std::string currentTheme = Themes::DAY;
    bool initialized = false;
    lv_style_t mockStyles[9]; // For various style types
    ThemeColors themeColors;
    
public:
    void initializeStyles() override {
        initialized = true;
        // Initialize mock theme colors
        themeColors.background = lv_color_black();
        themeColors.text = lv_color_white();
        themeColors.primary = lv_color_white();
        themeColors.gaugeNormal = lv_color_white();
        themeColors.gaugeWarning = lv_color_hex(0xFFA500);
        themeColors.gaugeDanger = lv_color_red();
        themeColors.gaugeTicks = lv_color_hex(0x404040);
        themeColors.needleNormal = lv_color_white();
        themeColors.needleDanger = lv_color_red();
        themeColors.keyPresent = lv_color_white();
        themeColors.keyNotPresent = lv_color_red();
    }
    
    bool isInitialized() const override {
        return initialized;
    }
    
    void init(const char* theme) override {
        currentTheme = theme;
        initializeStyles();
    }
    
    void applyThemeToScreen(lv_obj_t* screen) override {
        // Mock implementation
    }
    
    void setTheme(const char* theme) override {
        currentTheme = theme;
    }
    
    lv_style_t& getBackgroundStyle() override { return mockStyles[0]; }
    lv_style_t& getTextStyle() override { return mockStyles[1]; }
    lv_style_t& getGaugeNormalStyle() override { return mockStyles[2]; }
    lv_style_t& getGaugeWarningStyle() override { return mockStyles[3]; }
    lv_style_t& getGaugeDangerStyle() override { return mockStyles[4]; }
    lv_style_t& getGaugeIndicatorStyle() override { return mockStyles[5]; }
    lv_style_t& getGaugeItemsStyle() override { return mockStyles[6]; }
    lv_style_t& getGaugeMainStyle() override { return mockStyles[7]; }
    lv_style_t& getGaugeDangerSectionStyle() override { return mockStyles[8]; }
    
    const char* getCurrentTheme() const override {
        return currentTheme.c_str();
    }
    
    const ThemeColors& getThemeColors() const override {
        return themeColors;
    }
    
    // Test utilities
    void reset() {
        currentTheme = Themes::DAY;
        initialized = false;
    }
};

// Mock Trigger Service
class MockTriggerService : public ITriggerService {
private:
    std::map<std::string, std::function<void()>> triggers;
    std::vector<std::string> triggerHistory;
    std::string startupPanelOverride;
    
public:
    void init() override {}
    
    void processTriggerEvents() override {
        // Mock implementation - process triggers if needed
    }
    
    void addTrigger(const std::string& triggerName, ISensor* sensor, std::function<void()> callback) override {
        triggers[triggerName] = callback;
    }
    
    bool hasTrigger(const std::string& triggerName) const override {
        return triggers.find(triggerName) != triggers.end();
    }
    
    void executeTriggerAction(Trigger* mapping, TriggerExecutionState state) override {
        if (mapping && triggers.count(mapping->triggerId)) {
            triggerHistory.push_back(mapping->triggerId);
            triggers[mapping->triggerId]();
        }
    }
    
    const char* getStartupPanelOverride() const override {
        return startupPanelOverride.empty() ? nullptr : startupPanelOverride.c_str();
    }
    
    // Test utilities
    void reset() {
        triggers.clear();
        triggerHistory.clear();
        startupPanelOverride.clear();
    }
    
    void simulateTrigger(const std::string& triggerName) {
        if (triggers.count(triggerName)) {
            triggerHistory.push_back(triggerName);
            triggers[triggerName]();
        }
    }
    
    const std::vector<std::string>& getTriggerHistory() const {
        return triggerHistory;
    }
    
    void setStartupPanelOverride(const std::string& panelName) {
        startupPanelOverride = panelName;
    }
};

// Mock Display Provider
class MockDisplayProvider : public IDisplayProvider {
private:
    bool initialized = false;
    lv_obj_t* mockScreen = nullptr;
    
public:
    void initialize() override {
        initialized = true;
        mockScreen = lv_obj_create(nullptr);
    }
    
    void init() {
        initialize();
    }
    
    bool isInitialized() const override {
        return initialized;
    }
    
    lv_obj_t* createScreen() override {
        return lv_obj_create(nullptr);
    }
    
    void loadScreen(lv_obj_t* screen) override {
        // Mock implementation - just set as active
        MockLVGLState::instance().setActiveScreen(screen);
    }
    
    lv_obj_t* createLabel(lv_obj_t* parent) override {
        return lv_label_create(parent);
    }
    
    lv_obj_t* createObject(lv_obj_t* parent) override {
        return lv_obj_create(parent);
    }
    
    lv_obj_t* createArc(lv_obj_t* parent) override {
        return lv_arc_create(parent);
    }
    
    lv_obj_t* createScale(lv_obj_t* parent) override {
        // LVGL doesn't have lv_scale_create in mock, use arc as substitute
        return lv_arc_create(parent);
    }
    
    lv_obj_t* createImage(lv_obj_t* parent) override {
        return lv_img_create(parent);
    }
    
    lv_obj_t* createLine(lv_obj_t* parent) override {
        // Create generic object for line mock
        return lv_obj_create(parent);
    }
    
    void deleteObject(lv_obj_t* obj) override {
        MockLVGLState::instance().deleteObject(obj);
    }
    
    void addEventCallback(lv_obj_t* obj, lv_event_cb_t callback, lv_event_code_t event_code, void* user_data) override {
        lv_obj_add_event_cb(obj, callback, event_code, user_data);
    }
    
    lv_obj_t* getMainScreen() override {
        if (!mockScreen) {
            mockScreen = createScreen();
        }
        return mockScreen;
    }
    
    // Test utilities
    void reset() {
        if (mockScreen) {
            MockLVGLState::instance().deleteObject(mockScreen);
            mockScreen = nullptr;
        }
        initialized = false;
    }
};

// Mock Preference Service
class MockPreferenceService : public IPreferenceService {
private:
    Configs config;
    std::map<std::string, std::string> preferences;
    bool loadCalled = false;
    bool saveCalled = false;
    bool storageFailureSimulated = false;
    
public:
    void init() override {}
    
    void saveConfig() override {
        saveCalled = true;
    }
    
    void loadConfig() override {
        loadCalled = true;
    }
    
    void createDefaultConfig() override {
        // Set default configuration values
        config.theme = Themes::DAY;
        config.panelName = PanelNames::OIL;
        config.updateRate = 500;
    }
    
    Configs& getConfig() override {
        return config;
    }
    
    const Configs& getConfig() const override {
        return config;
    }
    
    void setConfig(const Configs& newConfig) override {
        config = newConfig;
    }
    
    // Missing methods used by tests
    void load_preferences() {
        loadConfig();
    }
    
    void set_preference(const std::string& key, const std::string& value) {
        preferences[key] = value;
        // Also update config for backward compatibility
        if (key == "panelName") {
            config.panelName = value;
        } else if (key == "theme") {
            config.theme = value;
        }
    }
    
    std::string get_preference(const std::string& key, const std::string& defaultValue = "") {
        auto it = preferences.find(key);
        return (it != preferences.end()) ? it->second : defaultValue;
    }
    
    void clear_all_preferences() {
        preferences.clear();
        config = {};
    }
    
    // Missing methods for test scenarios
    void simulateCorruptedData(const std::string& corruptedJson) {
        // Simulate loading corrupted data - should trigger recovery
        preferences["corrupted"] = corruptedJson;
    }
    
    void simulateStorageFailure(bool enabled) {
        storageFailureSimulated = enabled;
    }
    
    // Test utilities
    void reset() {
        config = {};
        preferences.clear();
        loadCalled = false;
        saveCalled = false;
        storageFailureSimulated = false;
    }
    
    bool wasLoadCalled() const { return loadCalled; }
    bool wasSaveCalled() const { return saveCalled; }
};

// MockGpioProvider is defined in mock_gpio_provider.h

#endif // UNIT_TESTING