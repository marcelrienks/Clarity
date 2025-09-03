#include "utilities/interrupt_callbacks.h"
#include "sensors/key_present_sensor.h"
#include "sensors/key_not_present_sensor.h"
#include "sensors/lock_sensor.h" 
#include "sensors/lights_sensor.h"
#include "sensors/button_sensor.h"
#include "managers/panel_manager.h"
#include "managers/style_manager.h"
#include "managers/error_manager.h"
#include "managers/interrupt_manager.h"
#include "utilities/constants.h"
#include <esp32-hal-log.h>
#include <cstring>

// External references to global managers (defined in main.cpp)
extern std::unique_ptr<PanelManager> panelManager;
extern std::unique_ptr<StyleManager> styleManager;

// Global variable for tracking previous panel state
static std::string savedUserPanel = "";

//=============================================================================
// PANEL LOADING INTERRUPTS
//=============================================================================

void InterruptCallbacks::KeyPresentActivate(void* context) {
    log_i("KeyPresentActivate() - Loading KEY panel");
    
    // Save current user panel for restoration
    const char* currentPanel = panelManager->GetCurrentPanel();
    if (currentPanel && !panelManager->IsCurrentPanelTriggerDriven()) {
        savedUserPanel = currentPanel;
        log_d("Saved user panel '%s' for restoration", savedUserPanel.c_str());
    }
    
    // Load KEY panel as trigger-driven
    panelManager->CreateAndLoadPanel(PanelNames::KEY, true);
}

void InterruptCallbacks::KeyNotPresentActivate(void* context) {
    log_i("KeyNotPresentActivate() - Loading KEY panel");
    
    // Save current user panel for restoration  
    const char* currentPanel = panelManager->GetCurrentPanel();
    if (currentPanel && !panelManager->IsCurrentPanelTriggerDriven()) {
        savedUserPanel = currentPanel;
        log_d("Saved user panel '%s' for restoration", savedUserPanel.c_str());
    }
    
    // Load KEY panel as trigger-driven
    panelManager->CreateAndLoadPanel(PanelNames::KEY, true);
}

//=============================================================================
// LOCK STATE INTERRUPTS
//=============================================================================

void InterruptCallbacks::LockEngagedActivate(void* context) {
    log_i("LockEngagedActivate() - Loading LOCK panel");
    
    // Save current user panel for restoration
    const char* currentPanel = panelManager->GetCurrentPanel();
    if (currentPanel && !panelManager->IsCurrentPanelTriggerDriven()) {
        savedUserPanel = currentPanel;
        log_d("Saved user panel '%s' for restoration", savedUserPanel.c_str());
    }
    
    // Load LOCK panel as trigger-driven
    panelManager->CreateAndLoadPanel(PanelNames::LOCK, true);
}

void InterruptCallbacks::LockDisengagedActivate(void* context) {
    log_i("LockDisengagedActivate() - Checking for restoration");
    
    // Check if any blocking interrupts are active
    bool hasBlockingInterrupts = InterruptManager::Instance().HasActiveInterrupts();
    
    if (!hasBlockingInterrupts && !savedUserPanel.empty()) {
        log_i("No blocking interrupts - restoring to '%s'", savedUserPanel.c_str());
        panelManager->CreateAndLoadPanel(savedUserPanel.c_str(), false); // Load as user-driven
        savedUserPanel.clear(); // Clear after restoration
    } else if (hasBlockingInterrupts) {
        log_d("Cannot restore - blocking interrupts still active");
    } else {
        log_d("No saved panel to restore to");
    }
}

//=============================================================================
// LIGHTS STATE INTERRUPTS  
//=============================================================================

void InterruptCallbacks::LightsOnActivate(void* context) {
    log_i("LightsOnActivate() - Setting NIGHT theme");
    styleManager->SetTheme(Themes::NIGHT);
}

void InterruptCallbacks::LightsOffActivate(void* context) {
    log_i("LightsOffActivate() - Setting DAY theme");
    styleManager->SetTheme(Themes::DAY);
}

//=============================================================================
// ERROR STATE INTERRUPTS
//=============================================================================

void InterruptCallbacks::ErrorOccurredActivate(void* context) {
    log_i("ErrorOccurredActivate() - Loading ERROR panel");
    
    // Save current user panel for restoration
    const char* currentPanel = panelManager->GetCurrentPanel();
    if (currentPanel && !panelManager->IsCurrentPanelTriggerDriven()) {
        savedUserPanel = currentPanel;
        log_d("Saved user panel '%s' for restoration", savedUserPanel.c_str());
    }
    
    // Load ERROR panel as trigger-driven
    panelManager->CreateAndLoadPanel(PanelNames::ERROR, true);
}

void InterruptCallbacks::ErrorClearedActivate(void* context) {
    log_i("ErrorClearedActivate() - Checking for restoration");
    
    // Check if any blocking interrupts are active
    bool hasBlockingInterrupts = InterruptManager::Instance().HasActiveInterrupts();
    
    if (!hasBlockingInterrupts && !savedUserPanel.empty()) {
        log_i("No blocking interrupts - restoring to '%s'", savedUserPanel.c_str());
        panelManager->CreateAndLoadPanel(savedUserPanel.c_str(), false); // Load as user-driven
        savedUserPanel.clear(); // Clear after restoration
    } else if (hasBlockingInterrupts) {
        log_d("Cannot restore - blocking interrupts still active");
    } else {
        log_d("No saved panel to restore to");
    }
}

//=============================================================================
// BUTTON ACTION INTERRUPTS
//=============================================================================

void InterruptCallbacks::ShortPressActivate(void* context) {
    log_i("ShortPressActivate() - Executing short press action");
    
    ButtonSensor* sensor = static_cast<ButtonSensor*>(context);
    if (!sensor) {
        log_w("Short press context is null");
        return;
    }
    
    // Get current panel to determine action
    const char* currentPanel = panelManager->GetCurrentPanel();
    if (!currentPanel) {
        log_w("No current panel for short press action");
        return;
    }
    
    // Execute panel-specific short press action
    if (strcmp(currentPanel, PanelNames::OIL) == 0) {
        log_d("Short press on OIL panel - cycling to next panel");
        panelManager->CreateAndLoadPanel(PanelNames::KEY, false);
    } else if (strcmp(currentPanel, PanelNames::KEY) == 0) {
        log_d("Short press on WATER panel - cycling to OIL panel");
        panelManager->CreateAndLoadPanel(PanelNames::OIL, false);
    }
    // Key, Lock, and Error panels don't respond to button presses (trigger-driven only)
    
    // Clear the action from sensor
    sensor->ClearButtonAction();
}

void InterruptCallbacks::LongPressActivate(void* context) {
    log_i("LongPressActivate() - Executing long press action");
    
    ButtonSensor* sensor = static_cast<ButtonSensor*>(context);
    if (!sensor) {
        log_w("Long press context is null");
        return;
    }
    
    // Long press toggles between day/night theme
    const char* currentTheme = styleManager->GetCurrentTheme();
    if (strcmp(currentTheme, Themes::DAY) == 0) {
        log_d("Long press - switching from DAY to NIGHT theme");
        styleManager->SetTheme(Themes::NIGHT);
    } else {
        log_d("Long press - switching from NIGHT to DAY theme");
        styleManager->SetTheme(Themes::DAY);
    }
    
    // Clear the action from sensor
    sensor->ClearButtonAction();
}

//=============================================================================
// HELPER FUNCTIONS
//=============================================================================

void* InterruptCallbacks::GetPanelManager() {
    return panelManager.get();
}

void* InterruptCallbacks::GetStyleManager() {
    return styleManager.get();
}

void* InterruptCallbacks::GetCurrentPanel() {
    return const_cast<char*>(panelManager->GetCurrentPanel());
}