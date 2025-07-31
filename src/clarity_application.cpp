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

    Ticker::handle_lv_tasks();

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
    
    Ticker::handle_lv_tasks();
}

void ClarityApplication::update() {
    // Core 0 responsibilities: process trigger events directly (simplified)
    triggerService_->processTriggerEvents();
    
    panelService_->updatePanel();
    Ticker::handle_lv_tasks();
    Ticker::handle_dynamic_delay(millis());
}

void ClarityApplication::run() {
    // For testing scenarios or embedded loops
    while (true) {
        update();
    }
}