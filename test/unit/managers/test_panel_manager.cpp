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

static std::unique_ptr<ManagerTestFixture> panelManagerFixtureForTest;
static PanelManager* panelManagerForTest = nullptr;

void setUp_panel_manager() {
    panelManagerFixtureForTest = std::make_unique<ManagerTestFixture>();
    panelManagerFixtureForTest->SetUp();
    panelManagerForTest = new PanelManager(panelManagerFixtureForTest->getDisplayProvider(), panelManagerFixtureForTest->getGpioProvider(), panelManagerFixtureForTest->getStyleService());
}

void tearDown_panel_manager() {
    delete panelManagerForTest;
    panelManagerForTest = nullptr;
    panelManagerFixtureForTest->TearDown();
    panelManagerFixtureForTest.reset();
}

void test_panel_manager_init() {
    // Test initialization doesn't crash
    panelManagerForTest->init();
    TEST_ASSERT_TRUE(true);
}

void test_panel_manager_construction() {
    // Test that manager can be created and destroyed
    TEST_ASSERT_NOT_NULL(panelManagerForTest);
    TEST_ASSERT_NOT_NULL(panelManagerFixtureForTest->getDisplayProvider());
}

void test_panel_manager_create_and_load_panel() {
    panelManagerForTest->init();
    
    // Test creating and loading a panel by name
    panelManagerForTest->createAndLoadPanel(PanelNames::OIL);
    
    // Panel should be loaded successfully
    TEST_ASSERT_EQUAL_STRING(PanelNames::OIL, panelManagerForTest->getCurrentPanel());
}

void test_panel_manager_load_panel_with_splash() {
    panelManagerForTest->init();
    
    // Test loading panel with splash transition
    panelManagerForTest->createAndLoadPanelWithSplash(PanelNames::KEY);
    
    // With mock panels and synchronous callbacks, should load target panel directly
    TEST_ASSERT_EQUAL_STRING(PanelNames::KEY, panelManagerForTest->getCurrentPanel());
}

void test_panel_manager_update_panel() {
    panelManagerForTest->init();
    
    // Load a panel first
    panelManagerForTest->createAndLoadPanel("LOCK");
    
    // Update should work without crashing
    panelManagerForTest->updatePanel();
    TEST_ASSERT_TRUE(true);
}

void test_panel_manager_get_current_panel() {
    panelManagerForTest->init();
    
    // Load a panel
    panelManagerForTest->createAndLoadPanel(PanelNames::OIL);
    
    // Should be able to get current panel name
    const char* currentPanel = panelManagerForTest->getCurrentPanel();
    TEST_ASSERT_NOT_NULL(currentPanel);
    TEST_ASSERT_EQUAL_STRING(PanelNames::OIL, currentPanel);
}

void test_panel_manager_ui_state() {
    panelManagerForTest->init();
    
    // Test setting UI state
    panelManagerForTest->setUiState(UIState::IDLE);
    TEST_ASSERT_TRUE(true);
    
    panelManagerForTest->setUiState(UIState::LOADING);
    TEST_ASSERT_TRUE(true);
}

void test_panel_manager_panel_switching() {
    panelManagerForTest->init();
    
    // Test switching between different panels
    panelManagerForTest->createAndLoadPanel(PanelNames::OIL);
    TEST_ASSERT_EQUAL_STRING(PanelNames::OIL, panelManagerForTest->getCurrentPanel());
    
    // Switch to different panel
    panelManagerForTest->createAndLoadPanel(PanelNames::KEY);
    TEST_ASSERT_EQUAL_STRING(PanelNames::KEY, panelManagerForTest->getCurrentPanel());
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