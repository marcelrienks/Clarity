#include <unity.h>
#include "managers/panel_manager.h"
#include "utilities/types.h"

// Mock panel for testing
class MockPanel : public IPanel {
private:
    bool initialized = false;
    bool loaded = false;
    std::string panelName;

public:
    MockPanel(const std::string& name) : panelName(name) {}
    
    void init(IGpioProvider *gpio, IDisplayProvider *display) override { 
        initialized = true; 
    }
    
    void load(std::function<void()> callbackFunction, IGpioProvider *gpio, IDisplayProvider *display) override { 
        loaded = true;
        if (callbackFunction) callbackFunction();
    }
    
    void update(std::function<void()> callbackFunction, IGpioProvider *gpio, IDisplayProvider *display) override {
        if (callbackFunction) callbackFunction();
    }
    
    // Mock-specific methods  
    bool isInitialized() const { return initialized; }
    bool isLoaded() const { return loaded; }
    const char* getName() const { return panelName.c_str(); }
};

// Note: MockPreferenceService is now available via test_fixtures.h

#include "test_fixtures.h"

std::unique_ptr<ManagerTestFixture> panelManagerFixture;
PanelManager* panelManager = nullptr;

void setUp_panel_manager() {
    panelManagerFixture = std::make_unique<ManagerTestFixture>();
    panelManagerFixture->SetUp();
    panelManager = new PanelManager(panelManagerFixture->getDisplayProvider(), panelManagerFixture->getGpioProvider(), panelManagerFixture->getStyleService());
}

void tearDown_panel_manager() {
    delete panelManager;
    panelManager = nullptr;
    panelManagerFixture->TearDown();
    panelManagerFixture.reset();
}

void test_panel_manager_init() {
    // Test initialization doesn't crash
    panelManager->init();
    TEST_ASSERT_TRUE(true);
}

void test_panel_manager_construction() {
    // Test that manager can be created and destroyed
    TEST_ASSERT_NOT_NULL(panelManager);
    TEST_ASSERT_NOT_NULL(panelManagerFixture->getDisplayProvider());
}

void test_panel_manager_create_and_load_panel() {
    panelManager->init();
    
    // Test creating and loading a panel by name
    panelManager->createAndLoadPanel(PanelNames::OIL);
    
    // Panel should be loaded successfully
    TEST_ASSERT_EQUAL_STRING(PanelNames::OIL, panelManager->getCurrentPanel());
}

void test_panel_manager_load_panel_with_splash() {
    panelManager->init();
    
    // Test loading panel with splash transition
    panelManager->createAndLoadPanelWithSplash(PanelNames::KEY);
    
    // With mock panels and synchronous callbacks, should load target panel directly
    TEST_ASSERT_EQUAL_STRING(PanelNames::KEY, panelManager->getCurrentPanel());
}

void test_panel_manager_update_panel() {
    panelManager->init();
    
    // Load a panel first
    panelManager->createAndLoadPanel("LOCK");
    
    // Update should work without crashing
    panelManager->updatePanel();
    TEST_ASSERT_TRUE(true);
}

void test_panel_manager_get_current_panel() {
    panelManager->init();
    
    // Load a panel
    panelManager->createAndLoadPanel(PanelNames::OIL);
    
    // Should be able to get current panel name
    const char* currentPanel = panelManager->getCurrentPanel();
    TEST_ASSERT_NOT_NULL(currentPanel);
    TEST_ASSERT_EQUAL_STRING(PanelNames::OIL, currentPanel);
}

void test_panel_manager_ui_state() {
    panelManager->init();
    
    // Test setting UI state
    panelManager->setUiState(UIState::IDLE);
    TEST_ASSERT_TRUE(true);
    
    panelManager->setUiState(UIState::LOADING);
    TEST_ASSERT_TRUE(true);
}

void test_panel_manager_panel_switching() {
    panelManager->init();
    
    // Test switching between different panels
    panelManager->createAndLoadPanel(PanelNames::OIL);
    TEST_ASSERT_EQUAL_STRING(PanelNames::OIL, panelManager->getCurrentPanel());
    
    // Switch to different panel
    panelManager->createAndLoadPanel(PanelNames::KEY);
    TEST_ASSERT_EQUAL_STRING(PanelNames::KEY, panelManager->getCurrentPanel());
}

void runPanelManagerTests() {
    setUp_panel_manager();
    RUN_TEST(test_panel_manager_construction);
    RUN_TEST(test_panel_manager_init);
    RUN_TEST(test_panel_manager_create_and_load_panel);
    RUN_TEST(test_panel_manager_update_panel);
    RUN_TEST(test_panel_manager_load_panel_with_splash);
    RUN_TEST(test_panel_manager_get_current_panel);
    RUN_TEST(test_panel_manager_ui_state);
    RUN_TEST(test_panel_manager_panel_switching);
    tearDown_panel_manager();
}