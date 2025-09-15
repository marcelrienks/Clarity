#include "main.h"
#include "factories/provider_factory.h"
#include "factories/manager_factory.h"
#include "managers/error_manager.h"
#include "managers/interrupt_manager.h"
#include "managers/panel_manager.h"
#include "interfaces/i_preference_service.h"
#include "managers/style_manager.h"
#include "providers/device_provider.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_display_provider.h"
#include "utilities/constants.h"
#include "utilities/logging.h"
#include "utilities/ticker.h"
#include "utilities/types.h"

// Global variable definitions (declarations are in main.h)
std::unique_ptr<IProviderFactory> providerFactory;
std::unique_ptr<ManagerFactory> managerFactory;
std::unique_ptr<DeviceProvider> deviceProvider;
std::unique_ptr<IGpioProvider> gpioProvider;
std::unique_ptr<IDisplayProvider> displayProvider;
std::unique_ptr<StyleManager> styleManager;
std::unique_ptr<IPreferenceService> preferenceManager;
std::unique_ptr<PanelManager> panelManager;
InterruptManager *interruptManager;
ErrorManager *errorManager;

bool initializeServices()
{
    log_i("Starting Clarity service initialization with dual factory pattern...");

    providerFactory = std::make_unique<ProviderFactory>();
    if (!providerFactory) {
        log_e("Failed to create ProviderFactory - allocation failed");
        ErrorManager::Instance().ReportCriticalError("main", "ProviderFactory allocation failed");
        return false;
    }

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

    managerFactory = std::make_unique<ManagerFactory>(std::move(providerFactory));
    if (!managerFactory) {
        log_e("Failed to create ManagerFactory - allocation failed");
        ErrorManager::Instance().ReportCriticalError("main", "ManagerFactory allocation failed");
        return false;
    }

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

    errorManager = managerFactory->CreateErrorManager();
    if (!errorManager) {
        log_e("Failed to create ErrorManager via factory");
        ErrorManager::Instance().ReportCriticalError("main", "ErrorManager creation failed");
        return false;
    }

    log_t("System ready");
    return true;
}

void setup()
{
    log_i("Starting Clarity application...");

    if (!initializeServices())
    {
        return;
    }

    Ticker::handleLvTasks();

    styleManager->InitializeStyles();
    Ticker::handleLvTasks();

    auto config = preferenceManager->GetConfig();
    panelManager->CreateAndLoadPanel(config.panelName.c_str());
    Ticker::handleLvTasks();
    
    log_i("Clarity application started successfully");
}

void loop()
{
    // Always process - ActionHandler runs always, TriggerHandler only on IDLE
    interruptManager->Process();

    // Check for error panel trigger - must be processed when UI is IDLE to avoid conflicts
    static bool errorPanelTriggered = false;  // Track if error panel was already triggered
    if (panelManager->GetUiState() == UIState::IDLE)
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
    if (panelManager->GetUiState() == UIState::IDLE)
    {
        panelManager->UpdatePanel();
    }

    Ticker::handleLvTasks();
    Ticker::handleDynamicDelay(millis());
}
//CLEANED