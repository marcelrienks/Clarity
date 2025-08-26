#include "main.h"
#include "factories/manager_factory.h"
// ActionManager include removed - button handling moved to handler-based system
#include "managers/error_manager.h"
#include "managers/interrupt_manager.h"
#include "managers/panel_manager.h"
#include "managers/preference_manager.h"
#include "managers/style_manager.h"
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
// ActionManager removed - button handling moved to handler-based system
std::unique_ptr<PanelManager> panelManager;
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
    // Create InterruptManager first so it can be injected into PanelManager
    interruptManager = ManagerFactory::createInterruptManager(gpioProvider.get());
    // Create PanelManager with InterruptManager for button function injection
    panelManager = ManagerFactory::createPanelManager(displayProvider.get(), gpioProvider.get(), styleManager.get(),
                                                      preferenceManager.get(), interruptManager);
    errorManager = ManagerFactory::createErrorManager();

    // Verify all critical services were created
    bool allServicesCreated = deviceProvider && gpioProvider && displayProvider && styleManager && preferenceManager &&
                              panelManager && interruptManager && errorManager;

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
        // ActionManager removed - button handling moved to handler-based system
        if (!panelManager)
            ErrorManager::Instance().ReportCriticalError("Main", "PanelManager creation failed");
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
    // Interrupt system initialized by factory with all handlers
    Ticker::handleLvTasks();

    // Load startup panel from configuration
    // Note: Key override logic handled by interrupt system after startup
    auto config = preferenceManager->GetConfig();
    panelManager->CreateAndLoadPanel(config.panelName.c_str());

    Ticker::handleLvTasks();
    
    // Display complete system configuration for debugging
    if (interruptManager)
    {
        interruptManager->PrintSystemStatus();
    }
    
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
    
    // Periodic system health monitoring for debugging (every 30 seconds)
    static unsigned long lastDiagnosticTime = 0;
    static constexpr unsigned long DIAGNOSTIC_INTERVAL_MS = 30000;
    if (interruptManager && (millis() - lastDiagnosticTime) > DIAGNOSTIC_INTERVAL_MS)
    {
        log_i("=== Periodic System Diagnostics ===");
        size_t interruptCount = interruptManager->GetRegisteredInterruptCount();
        size_t totalEvals, totalExecs;
        interruptManager->GetInterruptStatistics(totalEvals, totalExecs);
        log_i("Interrupts: %d registered, %lu evaluations, %lu executions", 
              interruptCount, totalEvals, totalExecs);
        lastDiagnosticTime = millis();
    }

    // Process interrupts when UI is not busy to avoid conflicts
    if (interruptManager && panelManager)
    {
        UIState currentState = panelManager->GetUiState();
        if (currentState == UIState::IDLE)
        {
            interruptManager->Process(); // Execute interrupt evaluation and handling
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
