#include "main.h"
#include "factories/manager_factory.h"
#include "managers/action_manager.h"
#include "managers/error_manager.h"
#include "managers/interrupt_manager.h"
#include "managers/panel_manager.h"
#include "managers/preference_manager.h"
#include "managers/style_manager.h"
#include "managers/trigger_manager.h"
#include "providers/device_provider.h"
#include "providers/gpio_provider.h"
#include "providers/lvgl_display_provider.h"
#include "utilities/ticker.h"
#include "utilities/types.h"

// Global services - factory-created instances
std::unique_ptr<DeviceProvider> deviceProvider;
std::unique_ptr<GpioProvider> gpioProvider;
std::unique_ptr<LvglDisplayProvider> displayProvider;
std::unique_ptr<StyleManager> styleManager;
std::unique_ptr<PreferenceManager> preferenceManager;
std::unique_ptr<ActionManager> actionManager;
std::unique_ptr<PanelManager> panelManager;
std::unique_ptr<TriggerManager> triggerManager;
InterruptManager *interruptManager;
ErrorManager *errorManager;

bool initializeServices()
{
    log_v("initializeServices() called");
    log_i("Starting Clarity service initialization...");

    // Create providers directly
    log_d("Creating DeviceProvider...");
    deviceProvider = std::make_unique<DeviceProvider>();
    if (!deviceProvider) {
        log_e("Failed to create DeviceProvider - allocation failed");
        ErrorManager::Instance().ReportCriticalError("main", "DeviceProvider allocation failed");
        return false;
    }
    deviceProvider->prepare();
    if (!deviceProvider->screen) {
        log_e("DeviceProvider screen initialization failed");
        ErrorManager::Instance().ReportCriticalError("main", "DeviceProvider screen initialization failed");
        return false;
    }
    log_d("DeviceProvider created successfully");

    log_d("Creating GpioProvider...");
    gpioProvider = std::make_unique<GpioProvider>();
    if (!gpioProvider) {
        log_e("Failed to create GpioProvider - allocation failed");
        ErrorManager::Instance().ReportCriticalError("main", "GpioProvider allocation failed");
        return false;
    }
    log_d("GpioProvider created successfully");

    log_d("Creating LvglDisplayProvider...");
    if (!deviceProvider->screen) {
        log_e("Cannot create LvglDisplayProvider - DeviceProvider screen is null");
        ErrorManager::Instance().ReportCriticalError("main", "Cannot create LvglDisplayProvider - screen is null");
        return false;
    }
    displayProvider = std::make_unique<LvglDisplayProvider>(deviceProvider->screen);
    if (!displayProvider) {
        log_e("Failed to create LvglDisplayProvider - allocation failed");
        ErrorManager::Instance().ReportCriticalError("main", "LvglDisplayProvider allocation failed");
        return false;
    }
    log_d("LvglDisplayProvider created successfully");

    // Create managers - factories handle all error checking and logging
    preferenceManager = ManagerFactory::createPreferenceManager();
    // Initialize StyleManager with user's theme preference
    const char *userTheme = preferenceManager->GetConfig().theme.c_str();
    styleManager = ManagerFactory::createStyleManager(userTheme);
    // Create ActionManager with nullptr first, then update after PanelManager is created
    actionManager = ManagerFactory::createActionManager(gpioProvider.get(), nullptr);
    panelManager = ManagerFactory::createPanelManager(displayProvider.get(), gpioProvider.get(), styleManager.get(),
                                                      actionManager.get(), preferenceManager.get());
    // Resolve circular dependency: ActionManager needs PanelService for UIState checking
    actionManager->SetPanelService(panelManager.get());
    triggerManager = ManagerFactory::createTriggerManager(gpioProvider.get(), panelManager.get(), styleManager.get());
    interruptManager = ManagerFactory::createInterruptManager();
    errorManager = ManagerFactory::createErrorManager();

    // Verify all critical services were created
    bool allServicesCreated = deviceProvider && gpioProvider && displayProvider && styleManager && preferenceManager &&
                              actionManager && panelManager && triggerManager && interruptManager && errorManager;

    if (!allServicesCreated)
    {
        log_e("Critical service creation failed - check factory logs above");
        // Report critical errors for null services
        if (!deviceProvider)
            ErrorManager::Instance().ReportCriticalError("Main", "DeviceProvider creation failed");
        if (!gpioProvider)
            ErrorManager::Instance().ReportCriticalError("Main", "GpioProvider creation failed");
        if (!displayProvider)
            ErrorManager::Instance().ReportCriticalError("Main", "DisplayProvider creation failed");
        if (!styleManager)
            ErrorManager::Instance().ReportCriticalError("Main", "StyleManager creation failed");
        if (!preferenceManager)
            ErrorManager::Instance().ReportCriticalError("Main", "PreferenceManager creation failed");
        if (!actionManager)
            ErrorManager::Instance().ReportCriticalError("Main", "ActionManager creation failed");
        if (!panelManager)
            ErrorManager::Instance().ReportCriticalError("Main", "PanelManager creation failed");
        if (!triggerManager)
            ErrorManager::Instance().ReportCriticalError("Main", "TriggerManager creation failed");
        if (!interruptManager)
            ErrorManager::Instance().ReportCriticalError("Main", "InterruptManager creation failed");
        return false;
    }

    log_i("All services initialized successfully");
    return true;
}

void setup()
{
    log_v("setup() called");
    log_i("Starting Clarity application...");

    if (!initializeServices())
    {
        log_e("Service initialization failed - cannot continue");
        return;
    }

    Ticker::handleLvTasks();
    styleManager->InitializeStyles();
    // InterruptManager is initialized by factory, just register legacy sources
    // Phase 4: TriggerManager now uses new interrupt system, no longer needs registration
    // Phase 5: ActionManager now uses new interrupt system, no longer needs registration
    Ticker::handleLvTasks();

    // Load startup panel
    const char *startupPanel = triggerManager->GetStartupPanelOverride();
    if (startupPanel)
    {
        panelManager->CreateAndLoadPanel(startupPanel);
    }
    else
    {
        auto config = preferenceManager->GetConfig();
        panelManager->CreateAndLoadPanel(config.panelName.c_str());
    }

    Ticker::handleLvTasks();
    log_i("Clarity application started successfully");
}

void loop()
{
    log_v("loop() called");
    static unsigned long loopCount = 0;
    loopCount++;

    // Log every 1000 loops to verify main loop is running
    if (loopCount % 1000 == 0)
    {
        log_d("Main loop running - count: %lu", loopCount);
    }

    // Process interrupts using new coordinated system
    if (interruptManager && panelManager)
    {
        UIState currentState = panelManager->GetUiState();
        if (currentState == UIState::IDLE)
        {
            interruptManager->Process(); // New coordinated interrupt processing
        }
    }
    else if (!interruptManager)
    {
        log_e("interruptManager is null!");
    }

    // Update panel state only when IDLE - allows loading and animations to complete
    if (panelManager && panelManager->GetUiState() == UIState::IDLE)
    {
        panelManager->UpdatePanel();
    }

    Ticker::handleLvTasks();
    Ticker::handleDynamicDelay(millis());
}
