#include "clarity_application.h"
#include "utilities/ticker.h"
#include "main.h" // For logging

ClarityApplication::ClarityApplication(
    IPanelService* panelService,
    ITriggerService* triggerService,
    IPreferenceService* preferenceService
) : panelService_(panelService),
    triggerService_(triggerService),
    preferenceService_(preferenceService) {
}

void ClarityApplication::initialize() {
    log_d("Initializing Clarity application");
    
    // Initialize trigger service after all dependencies are resolved
    triggerService_->init();

    Ticker::handleLvTasks();

    // Check if startup triggers require a specific panel, otherwise use config default
    const char* startupPanel = triggerService_->getStartupPanelOverride();
    if (startupPanel) {
        log_i("Using startup panel override: %s", startupPanel);
        panelService_->createAndLoadPanelWithSplash(startupPanel);
    } else {
        auto config = preferenceService_->getConfig();
        log_i("Using config default panel: %s", config.panelName.c_str());
        panelService_->createAndLoadPanelWithSplash(config.panelName.c_str());
    }
    
    Ticker::handleLvTasks();
}

void ClarityApplication::update() {
    // Core 0 responsibilities: process trigger events directly (simplified)
    triggerService_->processTriggerEvents();
    
    panelService_->updatePanel();
    Ticker::handleLvTasks();
    Ticker::handleDynamicDelay(millis());
}

void ClarityApplication::run() {
    // For testing scenarios or embedded loops
    while (true) {
        update();
    }
}