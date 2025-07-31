#include "mock_managers.h"
#include "test_utilities.h"
#include "utilities/types.h"

// Global mock state
static const char* current_panel = PanelNames::OIL;
static const char* current_theme = Themes::DAY;

// TriggerManager implementation
TriggerManager& TriggerManager::GetInstance() {
    static TriggerManager instance;
    return instance;
}

void TriggerManager::init() {
    // Initialize mock trigger manager
}

void TriggerManager::ProcessTriggerEvents() {
    // Process mock GPIO states and update panels accordingly
    bool keyPresent = MockHardware::getGpioState(25);
    if (keyPresent) {
        current_panel = PanelNames::KEY;
    } else {
        current_panel = PanelNames::OIL;
    }
}

// Mock PanelManager implementation
PanelManager& PanelManager::GetInstance() {
    static PanelManager instance;
    return instance;
}

void PanelManager::init() {
    // Initialize mock panel manager
    current_panel = "OemOilPanel";
}

void PanelManager::loadPanel(const char* panelName) {
    current_panel = panelName;
}

const char* PanelManager::getCurrentPanelName() const {
    return current_panel;
}

// StyleManager implementation
StyleManager& StyleManager::GetInstance() {
    static StyleManager instance;
    return instance;
}

void StyleManager::init() {
    // Initialize mock style manager
}

void StyleManager::setTheme(const char* theme) {
    current_theme = theme;
}
