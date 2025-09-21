#include "main.h"
#include "factories/provider_factory.h"
#include "factories/manager_factory.h"
#include "managers/error_manager.h"
#include "managers/interrupt_manager.h"
#include "managers/panel_manager.h"
#include "interfaces/i_preference_service.h"
#include "definitions/configs.h"
#include "managers/style_manager.h"
#include "providers/device_provider.h"
#include "interfaces/i_gpio_provider.h"
#include "interfaces/i_display_provider.h"
#include "definitions/constants.h"
#include "utilities/logging.h"
#include "utilities/ticker.h"
#include "definitions/types.h"
#include "managers/configuration_manager.h"

// Self-registration at program startup
static bool system_config_registered = []() {
    ConfigurationManager::AddSchema(SystemConfig::RegisterConfigSchema);
    return true;
}();

// ========== Global Variables ==========

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
std::unique_ptr<ConfigurationManager> configurationManager;

// ========== System Configuration ==========

/**
 * @brief Static method to register system configuration schema without instance
 * @param preferenceService Service to register schema with
 *
 * Called automatically at program startup through ConfigRegistry.
 * Registers the SystemConfig configuration schema without
 * requiring a manager instance to exist.
 */
void SystemConfig::RegisterConfigSchema(IPreferenceService* preferenceService)
{
    if (!preferenceService) return;

    // Check if already registered to prevent duplicates
    if (preferenceService->IsSchemaRegistered(ConfigConstants::Sections::SYSTEM)) {
        log_d("SystemConfig schema already registered");
        return;
    }

    Config::ConfigSection section(ConfigConstants::Sections::SYSTEM, ConfigConstants::Sections::SYSTEM, ConfigConstants::Sections::SYSTEM);

    section.AddItem(defaultPanelConfig);
    section.AddItem(updateRateConfig);
    section.AddItem(showSplashConfig);

    preferenceService->RegisterConfigSection(section);
    log_i("SystemConfig configuration schema registered (static)");
}

/**
 * @brief Legacy function for backward compatibility during migration
 *
 * This function maintains backward compatibility during migration.
 * New code path uses static RegisterConfigSchema instead.
 * Can be removed once migration is complete.
 */
void registerSystemConfiguration()
{
    // During migration, just delegate to static method
    SystemConfig::RegisterConfigSchema(preferenceManager.get());
}

// ========== Private Methods ==========

/**
 * @brief Initializes all system services and managers using factory pattern
 * @return true if all services initialized successfully, false on failure
 *
 * Creates and initializes all core system components in the correct dependency
 * order. Uses dual factory pattern with ProviderFactory for hardware abstraction
 * and ManagerFactory for system services. Reports critical errors on failure.
 */
bool initializeServices()
{
    log_i("Starting Clarity service initialization with dual factory pattern...");

    providerFactory = std::make_unique<ProviderFactory>();
    if (!providerFactory) {
        log_e("Failed to create ProviderFactory - %s", ErrorMessages::Generic::ALLOCATION_FAILED);
        ErrorManager::Instance().ReportCriticalError("main", ErrorMessages::System::PROVIDER_FACTORY_ALLOCATION_FAILED);
        return false;
    }

    deviceProvider = providerFactory->CreateDeviceProvider();
    if (!deviceProvider) {
        log_e("Failed to create DeviceProvider via factory");
        ErrorManager::Instance().ReportCriticalError("main", ErrorMessages::System::DEVICE_PROVIDER_CREATION_FAILED);
        return false;
    }

    gpioProvider = providerFactory->CreateGpioProvider();
    if (!gpioProvider) {
        log_e("Failed to create GpioProvider via factory");
        ErrorManager::Instance().ReportCriticalError("main", ErrorMessages::System::GPIO_PROVIDER_CREATION_FAILED);
        return false;
    }

    displayProvider = providerFactory->CreateDisplayProvider(deviceProvider.get());
    if (!displayProvider) {
        log_e("Failed to create DisplayProvider via factory");
        ErrorManager::Instance().ReportCriticalError("main", ErrorMessages::System::DISPLAY_PROVIDER_CREATION_FAILED);
        return false;
    }

    managerFactory = std::make_unique<ManagerFactory>(std::move(providerFactory));
    if (!managerFactory) {
        log_e("Failed to create ManagerFactory - %s", ErrorMessages::Generic::ALLOCATION_FAILED);
        ErrorManager::Instance().ReportCriticalError("main", ErrorMessages::System::MANAGER_FACTORY_ALLOCATION_FAILED);
        return false;
    }

    preferenceManager = managerFactory->CreatePreferenceManager();
    if (!preferenceManager) {
        log_e("Failed to create PreferenceManager via factory");
        ErrorManager::Instance().ReportCriticalError("main", ErrorMessages::System::PREFERENCE_MANAGER_CREATION_FAILED);
        return false;
    }

    // Create ConfigurationManager for schema registration coordination
    configurationManager = std::make_unique<ConfigurationManager>();
    if (!configurationManager) {
        log_e("Failed to create ConfigurationManager");
        ErrorManager::Instance().ReportCriticalError("main", "Configuration manager allocation failed");
        return false;
    }

    // Initialize StyleManager with user's theme preference
    std::string userTheme = UIStrings::ThemeNames::DAY; // Default
    if (auto themeValue = preferenceManager->QueryConfig<std::string>(ConfigConstants::Keys::SYSTEM_THEME)) {
        userTheme = *themeValue;
    }
    styleManager = managerFactory->CreateStyleManager(userTheme.c_str());
    if (!styleManager) {
        log_e("Failed to create StyleManager via factory");
        ErrorManager::Instance().ReportCriticalError("main", ErrorMessages::System::STYLE_MANAGER_CREATION_FAILED);
        return false;
    }
 
    styleManager->SetPreferenceService(preferenceManager.get());
    
    // Create InterruptManager with GPIO provider dependency
    interruptManager = managerFactory->CreateInterruptManager(gpioProvider.get());
    if (!interruptManager) {
        log_e("Failed to create InterruptManager via factory");
        ErrorManager::Instance().ReportCriticalError("main", ErrorMessages::System::INTERRUPT_MANAGER_CREATION_FAILED);
        return false;
    }
    
    // Create PanelManager with all dependencies
    panelManager = managerFactory->CreatePanelManager(displayProvider.get(), gpioProvider.get(), 
                                                      styleManager.get(), preferenceManager.get(), 
                                                      interruptManager);

    if (!panelManager) {
        log_e("Failed to create PanelManager via factory");
        ErrorManager::Instance().ReportCriticalError("main", ErrorMessages::System::PANEL_MANAGER_CREATION_FAILED);
        return false;
    }

    errorManager = managerFactory->CreateErrorManager();
    if (!errorManager) {
        log_e("Failed to create ErrorManager via factory");
        ErrorManager::Instance().ReportCriticalError("main", ErrorMessages::System::ERROR_MANAGER_CREATION_FAILED);
        return false;
    }

    log_t("System ready");
    return true;
}

// ========== Arduino Functions ==========

/**
 * @brief Arduino setup function - initializes the Clarity application
 *
 * Entry point for ESP32 application initialization. Initializes all system
 * services, configures display styles, and loads the initial panel based on
 * user preferences. Called once at system startup before the main loop.
 */
void setup()
{
    log_i("Starting Clarity application...");

    if (!initializeServices())
    {
        return;
    }

    // Register all configuration schemas from components via manager
    configurationManager->RegisterAllSchemas(preferenceManager.get());

    Ticker::handleLvTasks();

    styleManager->InitializeStyles();
    Ticker::handleLvTasks();

    std::string panelName = PanelNames::OIL; // Default
    if (auto nameValue = preferenceManager->QueryConfig<std::string>(ConfigConstants::Keys::SYSTEM_DEFAULT_PANEL)) {
        panelName = *nameValue;
    }
    panelManager->CreateAndLoadPanel(panelName.c_str());
    Ticker::handleLvTasks();
    
    log_i("Clarity application started successfully");
}

/**
 * @brief Arduino main loop - processes system events and updates UI
 *
 * Continuously processes interrupt events, monitors error conditions,
 * and updates the active panel. Handles error panel triggering when
 * UI is idle to avoid conflicts with other operations. Maintains
 * responsive system behavior through dynamic delay management.
 */
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