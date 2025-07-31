#pragma once

#include "interfaces/i_panel_service.h"
#include "interfaces/i_trigger_service.h"
#include "interfaces/i_preference_service.h"

/**
 * Main application class that coordinates the core application loop
 * with dependency injection instead of global state
 */
class ClarityApplication {
private:
    IPanelService* panelService_;
    ITriggerService* triggerService_;
    IPreferenceService* preferenceService_;

public:
    ClarityApplication(
        IPanelService* panelService,
        ITriggerService* triggerService,
        IPreferenceService* preferenceService
    );

    /**
     * Initialize the application and load the startup panel
     */
    void initialize();

    /**
     * Execute one iteration of the application loop
     */
    void update();

    /**
     * Run the main application loop (for testing/embedded scenarios)
     */
    void run();
};