#include "main.h"
#include "factories/provider_factory.h"
#include "factories/manager_factory.h"
#include "managers/error_manager.h"
#include "managers/interrupt_manager.h"
#include "managers/panel_manager.h"
#include "managers/preference_manager.h"
#include "managers/style_manager.h"
#include "interfaces/i_device_provider.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_display_provider.h"
#include "utilities/constants.h"  // For PanelNames
#include "utilities/logging.h"    // For log_t()
#include "utilities/ticker.h"
#include "utilities/types.h"

// Global factories - dual factory pattern implementation
std::unique_ptr<IProviderFactory> providerFactory;
std::unique_ptr<ManagerFactory> managerFactory;

// Global providers - created by ProviderFactory
std::unique_ptr<IDeviceProvider> deviceProvider;
std::unique_ptr<IGpioProvider> gpioProvider;
std::unique_ptr<IDisplayProvider> displayProvider;

// Global managers - created by ManagerFactory
std::unique_ptr<StyleManager> styleManager;
std::unique_ptr<PreferenceManager> preferenceManager;
std::unique_ptr<PanelManager> panelManager;
InterruptManager *interruptManager;
ErrorManager *errorManager;

bool initializeServices()
{
    log_v("initializeServices() called");
    log_i("Starting Clarity service initialization with dual factory pattern...");

    // Step 1: Create ProviderFactory
    providerFactory = std::make_unique<ProviderFactory>();
    if (!providerFactory) {
        log_e("Failed to create ProviderFactory - allocation failed");
        ErrorManager::Instance().ReportCriticalError("main", "ProviderFactory allocation failed");
        return false;
    }

    // Step 2: Create providers using ProviderFactory
    deviceProvider = providerFactory->CreateDeviceProvider();
    if (!deviceProvider) {
        log_e("Failed to create DeviceProvider via factory");
        ErrorManager::Instance().ReportCriticalError("main", "DeviceProvider creation failed");
        return false;
    }

    gpioProvider = providerFactory->CreateGpioProvider();
    if (!gpioProvider) {
        log_e("Failed to create GpioProvider via factory");
        ErrorManager::Instance().ReportCriticalError("main", "GpioProvider creation failed");
        return false;
    }

    displayProvider = providerFactory->CreateDisplayProvider(deviceProvider.get());
    if (!displayProvider) {
        log_e("Failed to create DisplayProvider via factory");
        ErrorManager::Instance().ReportCriticalError("main", "DisplayProvider creation failed");
        return false;
    }

    // Step 3: Create ManagerFactory with dependency injection
    managerFactory = std::make_unique<ManagerFactory>(std::move(providerFactory));
    if (!managerFactory) {
        log_e("Failed to create ManagerFactory - allocation failed");
        ErrorManager::Instance().ReportCriticalError("main", "ManagerFactory allocation failed");
        return false;
    }

    // Step 4: Create managers using ManagerFactory
    
    // Create PreferenceManager
    preferenceManager = managerFactory->CreatePreferenceManager();
    if (!preferenceManager) {
        log_e("Failed to create PreferenceManager via factory");
        ErrorManager::Instance().ReportCriticalError("main", "PreferenceManager creation failed");
        return false;
    }
    
    // Initialize StyleManager with user's theme preference
    const char *userTheme = preferenceManager->GetConfig().theme.c_str();
    styleManager = managerFactory->CreateStyleManager(userTheme);
    if (!styleManager) {
        log_e("Failed to create StyleManager via factory");
        ErrorManager::Instance().ReportCriticalError("main", "StyleManager creation failed");
        return false;
    }
    
    // Inject PreferenceService into StyleManager for direct preference reading
    styleManager->SetPreferenceService(preferenceManager.get());
    
    // Create InterruptManager with GPIO provider dependency
    interruptManager = managerFactory->CreateInterruptManager(gpioProvider.get());
    if (!interruptManager) {
        log_e("Failed to create InterruptManager via factory");
        ErrorManager::Instance().ReportCriticalError("main", "InterruptManager creation failed");
        return false;
    }
    
    // Create PanelManager with all dependencies
    panelManager = managerFactory->CreatePanelManager(displayProvider.get(), gpioProvider.get(), 
                                                      styleManager.get(), preferenceManager.get(), 
                                                      interruptManager);
    if (!panelManager) {
        log_e("Failed to create PanelManager via factory");
        ErrorManager::Instance().ReportCriticalError("main", "PanelManager creation failed");
        return false;
    }
    
    // Create ErrorManager
    errorManager = managerFactory->CreateErrorManager();
    if (!errorManager) {
        log_e("Failed to create ErrorManager via factory");
        ErrorManager::Instance().ReportCriticalError("main", "ErrorManager creation failed");
        return false;
    }
    

    // Verify all critical services were created (redundant check as we already checked individually)
    bool allServicesCreated = deviceProvider && gpioProvider && displayProvider && 
                              styleManager && preferenceManager && panelManager && 
                              interruptManager && errorManager;

    if (!allServicesCreated)
    {
        log_e("Critical service creation failed - check logs above for specific failures");
        return false;
    }

    log_i("All services initialized successfully");
    log_t("System ready");
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
    
    // Display complete system configuration
    if (interruptManager)
    {
        interruptManager->PrintSystemStatus();
    }
    
    log_i("Clarity application started successfully");
}

void loop()
{
    static unsigned long loopCount = 0;
    static unsigned long lastLoopLogTime = 0;
    loopCount++;
    unsigned long currentTime = millis();

    // Log every 2 seconds to detect hangs more quickly
    if (currentTime - lastLoopLogTime > 2000)
    {
        UIState currentUiState = panelManager ? panelManager->GetUiState() : UIState::BUSY;
        const char* currentPanel = panelManager ? panelManager->GetCurrentPanel() : "UNKNOWN";
        
        log_i("MAIN LOOP alive: iteration %lu, UI state: %s, panel: %s", 
              loopCount, 
              currentUiState == UIState::IDLE ? "IDLE" : "BUSY",
              currentPanel ? currentPanel : "NULL");
        lastLoopLogTime = currentTime;
    }

    // Log every 1000 loops to verify main loop is running
    if (loopCount % 1000 == 0)
    {
    }
    
    // Periodic system health monitoring (every 30 seconds)
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

    // Process interrupts - Actions (button timing) always run, Triggers only during IDLE
    if (interruptManager && panelManager)
    {
        interruptManager->Process(); // Always process - ActionHandler runs always, TriggerHandler only on IDLE
    }
    else if (!interruptManager)
    {
        log_e("interruptManager is null!");
    }

    // Check for error panel trigger - must be processed when UI is IDLE to avoid conflicts
    static bool errorPanelTriggered = false;  // Track if error panel was already triggered
    if (errorManager && panelManager && panelManager->GetUiState() == UIState::IDLE)
    {
        bool shouldTriggerError = errorManager->ShouldTriggerErrorPanel();
        const char* currentPanel = panelManager->GetCurrentPanel();
        bool isCurrentlyErrorPanel = currentPanel && strcmp(currentPanel, PanelNames::ERROR) == 0;
        
        // Trigger error panel if needed and not already triggered
        if (shouldTriggerError && !errorPanelTriggered && !isCurrentlyErrorPanel)
        {
            log_t("ErrorOccurredActivate() - Loading ERROR panel");
            panelManager->CreateAndLoadPanel(PanelNames::ERROR, true);  // Mark as trigger-driven
            errorManager->SetErrorPanelActive(true);
            errorPanelTriggered = true;
        }
        // Reset trigger state when no longer needed
        else if (!shouldTriggerError && errorPanelTriggered)
        {
            errorManager->SetErrorPanelActive(false);
            errorPanelTriggered = false;
        }
    }

    // Update panel state only when IDLE - allows loading and animations to complete
    if (panelManager && panelManager->GetUiState() == UIState::IDLE)
    {
        panelManager->UpdatePanel();
    }

    Ticker::handleLvTasks();
    Ticker::handleDynamicDelay(millis());
}
