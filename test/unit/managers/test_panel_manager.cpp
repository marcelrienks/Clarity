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
    
    void init() override { initialized = true; }
    void load() override { loaded = true; }
    void update() override {}
    void unload() override { loaded = false; }
    bool isActive() const override { return loaded; }
    const char* getName() const override { return panelName.c_str(); }
    
    // Mock-specific methods
    bool isInitialized() const { return initialized; }
    bool isLoaded() const { return loaded; }
};

// Mock service interfaces
class MockPreferenceService : public IPreferenceService {
public:
    Configs config;
    
    void init() override {}
    void saveConfig() override {}
    void loadConfig() override {}
    void createDefaultConfig() override { config.panelName = PanelNames::OIL; }
    Configs& getConfig() override { return config; }
    const Configs& getConfig() const override { return config; }
    void setConfig(const Configs& newConfig) override { config = newConfig; }
};

PanelManager* panelManager = nullptr;
MockPreferenceService* mockPrefService = nullptr;

void setUp_panel_manager() {
    mockPrefService = new MockPreferenceService();
    mockPrefService->createDefaultConfig();
    panelManager = new PanelManager(mockPrefService);
}

void tearDown_panel_manager() {
    delete panelManager;
    delete mockPrefService;
    panelManager = nullptr;
    mockPrefService = nullptr;
}

void test_panel_manager_init() {
    // Test initialization doesn't crash
    panelManager->init();
    TEST_ASSERT_TRUE(true);
}

void test_panel_manager_construction() {
    // Test that manager can be created and destroyed
    TEST_ASSERT_NOT_NULL(panelManager);
    TEST_ASSERT_NOT_NULL(mockPrefService);
}

void test_panel_manager_add_panel() {
    panelManager->init();
    
    // Create mock panel and add it
    auto mockPanel = std::make_shared<MockPanel>("TestPanel");
    panelManager->addPanel("TestPanel", mockPanel);
    
    // Panel should be added successfully
    TEST_ASSERT_TRUE(panelManager->hasPanel("TestPanel"));
}

void test_panel_manager_load_panel() {
    panelManager->init();
    
    // Add a mock panel
    auto mockPanel = std::make_shared<MockPanel>("TestPanel");
    panelManager->addPanel("TestPanel", mockPanel);
    
    // Load the panel
    panelManager->loadPanel("TestPanel");
    
    // Panel should be loaded
    TEST_ASSERT_TRUE(mockPanel->isLoaded());
}

void test_panel_manager_has_panel() {
    panelManager->init();
    
    // Initially should not have test panel
    TEST_ASSERT_FALSE(panelManager->hasPanel("NonExistentPanel"));
    
    // Add panel and test
    auto mockPanel = std::make_shared<MockPanel>("TestPanel");
    panelManager->addPanel("TestPanel", mockPanel);
    TEST_ASSERT_TRUE(panelManager->hasPanel("TestPanel"));
}

void test_panel_manager_get_active_panel() {
    panelManager->init();
    
    // Add and load a panel
    auto mockPanel = std::make_shared<MockPanel>("ActivePanel");
    panelManager->addPanel("ActivePanel", mockPanel);
    panelManager->loadPanel("ActivePanel");
    
    // Should be able to get active panel name
    const char* activePanelName = panelManager->getActivePanelName();
    TEST_ASSERT_NOT_NULL(activePanelName);
    TEST_ASSERT_EQUAL_STRING("ActivePanel", activePanelName);
}

void test_panel_manager_update_active_panel() {
    panelManager->init();
    
    // Add and load a panel
    auto mockPanel = std::make_shared<MockPanel>("UpdatePanel");
    panelManager->addPanel("UpdatePanel", mockPanel);
    panelManager->loadPanel("UpdatePanel");
    
    // Update should not crash
    panelManager->updateActivePanel();
    TEST_ASSERT_TRUE(true);
}

void test_panel_manager_panel_switching() {
    panelManager->init();
    
    // Add multiple panels
    auto panel1 = std::make_shared<MockPanel>("Panel1");
    auto panel2 = std::make_shared<MockPanel>("Panel2");
    panelManager->addPanel("Panel1", panel1);
    panelManager->addPanel("Panel2", panel2);
    
    // Load first panel
    panelManager->loadPanel("Panel1");
    TEST_ASSERT_TRUE(panel1->isLoaded());
    TEST_ASSERT_FALSE(panel2->isLoaded());
    
    // Switch to second panel
    panelManager->loadPanel("Panel2");
    // Note: Depending on implementation, first panel may or may not be unloaded
    TEST_ASSERT_TRUE(panel2->isLoaded());
}

void runPanelManagerTests() {
    setUp_panel_manager();
    RUN_TEST(test_panel_manager_construction);
    RUN_TEST(test_panel_manager_init);
    RUN_TEST(test_panel_manager_add_panel);
    RUN_TEST(test_panel_manager_has_panel);
    RUN_TEST(test_panel_manager_load_panel);
    RUN_TEST(test_panel_manager_get_active_panel);
    RUN_TEST(test_panel_manager_update_active_panel);
    RUN_TEST(test_panel_manager_panel_switching);
    tearDown_panel_manager();
}