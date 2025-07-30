#include "panel_manager.h"
#include "test_utilities.h"
#include <cstring>

// Mock state variables (declared in test_utilities.h)
extern std::vector<const char*> panel_creation_history;
extern std::vector<const char*> panel_load_history;
extern bool panel_loaded;
extern bool panel_initialized;

PanelManager::PanelManager() 
    : currentPanel_(nullptr), registeredPanelCount_(0), initialized_(false) {
}

PanelManager::~PanelManager() {
    // Cleanup if needed
}

bool PanelManager::initialize() {
    initialized_ = true;
    panel_initialized = true;
    
    // Create default oil panel
    currentPanel_ = new OemOilPanel();
    return true;
}

bool PanelManager::registerPanel(IPanel* panel) {
    if (!panel) return false;
    
    registeredPanelCount_++;
    return true;
}

IPanel* PanelManager::createPanel(const char* panelName) {
    if (!panelName) return nullptr;
    
    panel_creation_history.push_back(panelName);
    panel_load_history.push_back(panelName);
    panel_loaded = true;
    
    IPanel* newPanel = nullptr;
    
    if (strcmp(panelName, "KeyPanel") == 0) {
        newPanel = new KeyPanel();
    } else if (strcmp(panelName, "LockPanel") == 0) {
        newPanel = new LockPanel();
    } else if (strcmp(panelName, "OemOilPanel") == 0) {
        newPanel = new OemOilPanel();
    } else if (strcmp(panelName, "SplashPanel") == 0) {
        newPanel = new SplashPanel();
    }
    
    if (newPanel) {
        currentPanel_ = newPanel;
        newPanel->init();
        newPanel->load();
    }
    
    return newPanel;
}

IPanel* PanelManager::getCurrentPanel() {
    return currentPanel_;
}

size_t PanelManager::getRegisteredPanelCount() {
    return registeredPanelCount_;
}