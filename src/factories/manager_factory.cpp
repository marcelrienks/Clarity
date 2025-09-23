#include "factories/manager_factory.h"
#include "factories/provider_factory.h"
#include "managers/error_manager.h"
#include "managers/interrupt_manager.h"
#include "managers/panel_manager.h"
#include "managers/configuration_manager.h"
#include "managers/style_manager.h"
#include "providers/device_provider.h"
#include "sensors/button_sensor.h"
#ifdef CLARITY_DEBUG
#include "sensors/gpio_sensor.h"
#endif
#include "definitions/types.h"
#include <esp32-hal-log.h>

// ========== Constructors and Destructor ==========

/**
 * @brief Constructs ManagerFactory with custom provider factory
 * @param providerFactory Provider factory for creating hardware providers
 *
 * Accepts ownership of a provider factory for dependency injection.
 * Falls back to creating default ProviderFactory if nullptr is provided.
 */
ManagerFactory::ManagerFactory(std::unique_ptr<IProviderFactory> providerFactory)
    : providerFactory_(std::move(providerFactory))
{
    if (!providerFactory_) {
        log_w("ManagerFactory created without provider factory - will create default ProviderFactory");
        providerFactory_ = std::make_unique<ProviderFactory>();
    }
}

/**
 * @brief Default constructor creates standard provider factory
 *
 * Initializes ManagerFactory with default ProviderFactory for
 * standard hardware provider creation.
 */
ManagerFactory::ManagerFactory()
    : providerFactory_(std::make_unique<ProviderFactory>())
{
}

// ========== Private Methods ==========

/**
 * @brief Initializes hardware providers if not already created
 * @return true if all providers initialized successfully
 *
 * Creates device, GPIO, and display providers through the provider factory.
 * Ensures all providers are available before manager creation.
 */
bool ManagerFactory::InitializeProviders()
{
    
    if (!providerFactory_) {
        log_e("Cannot initialize providers - provider factory is null");
        return false;
    }
    
    // Create providers if not already created
    if (!deviceProvider_) {
        deviceProvider_ = providerFactory_->CreateDeviceProvider();
        if (!deviceProvider_) {
            log_e("Failed to create DeviceProvider");
            return false;
        }
    }
    
    if (!gpioProvider_) {
        gpioProvider_ = providerFactory_->CreateGpioProvider();
        if (!gpioProvider_) {
            log_e("Failed to create GpioProvider");
            return false;
        }
    }
    
    if (!displayProvider_) {
        displayProvider_ = providerFactory_->CreateDisplayProvider(deviceProvider_.get());
        if (!displayProvider_) {
            log_e("Failed to create DisplayProvider");
            return false;
        }
    }
    
    return true;
}

/**
 * @brief Internal implementation for creating PanelManager
 * @param display Display provider for UI rendering
 * @param gpio GPIO provider for hardware interaction
 * @param styleService Style service for theme management
 * @param preferenceService Preference service for configuration
 * @param interruptManager Interrupt manager for event handling
 * @return Unique pointer to PanelManager or nullptr on failure
 *
 * Validates all dependencies, creates PanelManager with dependency injection,
 * and initializes it before returning. Reports critical errors on failure.
 */
std::unique_ptr<PanelManager> ManagerFactory::CreatePanelManagerImpl(IDisplayProvider *display, IGpioProvider *gpio,
                                                                      IStyleManager *styleService,
                                                                      IConfigurationManager *preferenceService,
                                                                      InterruptManager *interruptManager)
{

    // Validate required providers
    if (!display)
    {
        log_e("ManagerFactory: Cannot create PanelManager - IDisplayProvider is null");
        ErrorManager::Instance().ReportCriticalError("ManagerFactory",
                                                     "Cannot create PanelManager - DisplayProvider dependency is null");
        return nullptr;
    }
    if (!gpio)
    {
        log_e("ManagerFactory: Cannot create PanelManager - IGpioProvider is null");
        ErrorManager::Instance().ReportCriticalError("ManagerFactory",
                                                     "Cannot create PanelManager - GpioProvider dependency is null");
        return nullptr;
    }
    if (!styleService)
    {
        log_e("ManagerFactory: Cannot create PanelManager - IStyleManager is null");
        ErrorManager::Instance().ReportCriticalError("ManagerFactory",
                                                     "Cannot create PanelManager - StyleService dependency is null");
        return nullptr;
    }
    
    if (!preferenceService)
    {
        log_e("ManagerFactory: Cannot create PanelManager - IConfigurationManager is null");
        ErrorManager::Instance().ReportCriticalError(
            "ManagerFactory", "Cannot create PanelManager - PreferenceService dependency is null");
        return nullptr;
    }

    auto panelManager = std::make_unique<PanelManager>(display, gpio, styleService, preferenceService, interruptManager);
    if (!panelManager)
    {
        log_e("ManagerFactory: Failed to create PanelManager - allocation failed");
        ErrorManager::Instance().ReportCriticalError("ManagerFactory",
                                                     "PanelManager allocation failed - out of memory");
        return nullptr;
    }

    panelManager->Init();
    
    return panelManager;
}

/**
 * @brief Internal implementation for creating StyleManager
 * @param theme Initial theme name (defaults to DAY if null)
 * @return Unique pointer to StyleManager or nullptr on failure
 *
 * Creates StyleManager with specified or default theme.
 * Reports critical error if allocation fails.
 */
std::unique_ptr<StyleManager> ManagerFactory::CreateStyleManagerImpl(const char *theme)
{

    auto manager = std::make_unique<StyleManager>(theme ? theme : Themes::DAY);
    if (!manager)
    {
        log_e("ManagerFactory: Failed to create StyleManager - allocation failed");
        ErrorManager::Instance().ReportCriticalError("ManagerFactory",
                                                     "StyleManager allocation failed - out of memory");
        return nullptr;
    }

    return manager;
}


/**
 * @brief Internal implementation for creating ConfigurationManager
 * @return Unique pointer to IConfigurationManager or nullptr on failure
 *
 * Creates ConfigurationManager as the unified configuration interface.
 * Returns as IConfigurationManager interface for abstraction.
 */
std::unique_ptr<IConfigurationManager> ManagerFactory::CreatePreferenceManagerImpl()
{
    // Create ConfigurationManager as the unified configuration interface
    auto manager = std::make_unique<ConfigurationManager>();
    if (!manager)
    {
        log_e("ManagerFactory: Failed to create ConfigurationManager - allocation failed");
        ErrorManager::Instance().ReportCriticalError("ManagerFactory",
                                                     "ConfigurationManager allocation failed - out of memory");
        return nullptr;
    }

    // Initialize the configuration manager with storage backend
    if (!manager->Initialize()) {
        log_e("ManagerFactory: Failed to initialize ConfigurationManager");
        ErrorManager::Instance().ReportCriticalError("ManagerFactory",
                                                     "ConfigurationManager initialization failed");
        return nullptr;
    }

    log_d("ManagerFactory: ConfigurationManager created and initialized successfully");
    return manager;
}

/**
 * @brief Internal implementation for creating InterruptManager
 * @param gpioProvider GPIO provider for hardware interrupts
 * @return Pointer to InterruptManager singleton or nullptr on failure
 *
 * Gets singleton instance of InterruptManager and initializes it with GPIO provider.
 * Initialization creates handlers that own sensors for interrupt processing.
 */
InterruptManager* ManagerFactory::CreateInterruptManagerImpl(IGpioProvider* gpioProvider)
{

    if (!gpioProvider) {
        log_e("ManagerFactory: Cannot create InterruptManager - GPIO provider is null");
        ErrorManager::Instance().ReportCriticalError("ManagerFactory",
                                                     "Cannot create InterruptManager - GpioProvider dependency is null");
        return nullptr;
    }

    InterruptManager* manager = &InterruptManager::Instance();
    if (!manager)
    {
        log_e("ManagerFactory: Failed to get InterruptManager singleton instance");
        return nullptr;
    }

    manager->Init(gpioProvider);
    
    // IMPORTANT: InterruptManager::Init() creates handlers which own the sensors
    // We must register interrupts AFTER Init() completes so contexts are available
    
    return manager;
}

/**
 * @brief Internal implementation for creating ErrorManager
 * @return Pointer to ErrorManager singleton or nullptr on failure
 *
 * Returns the singleton instance of ErrorManager for system-wide
 * error reporting and management.
 */
ErrorManager *ManagerFactory::CreateErrorManagerImpl()
{

    ErrorManager *errorManager = &ErrorManager::Instance();
    if (!errorManager)
    {
        log_e("ManagerFactory: Failed to get ErrorManager singleton instance");
        return nullptr;
    }

    return errorManager;
}

/**
 * @brief Creates PanelManager with provided or factory-created providers
 * @param display Display provider (optional, created if null)
 * @param gpio GPIO provider (optional, created if null)
 * @param styleService Style service for theme management
 * @param preferenceService Preference service for configuration
 * @param interruptManager Interrupt manager for event handling
 * @return Unique pointer to PanelManager or nullptr on failure
 *
 * Public interface method that uses provided providers or creates them
 * through provider factory if not supplied. Delegates to implementation method.
 */
std::unique_ptr<PanelManager> ManagerFactory::CreatePanelManager(IDisplayProvider *display, IGpioProvider *gpio,
                                                                  IStyleManager *styleService,
                                                                  IConfigurationManager *preferenceService,
                                                                  InterruptManager *interruptManager)
{
    
    // When using the dual factory pattern, get providers from provider factory
    IDisplayProvider* displayToUse = display;
    IGpioProvider* gpioToUse = gpio;
    
    // If providers not passed, get them from provider factory
    if (!display || !gpio) {
        if (!InitializeProviders()) {
            log_e("Failed to initialize providers from factory");
            return nullptr;
        }
        displayToUse = display ? display : displayProvider_.get();
        gpioToUse = gpio ? gpio : gpioProvider_.get();
    }
    
    return CreatePanelManagerImpl(displayToUse, gpioToUse, styleService, preferenceService, interruptManager);
}

/**
 * @brief Creates StyleManager with specified theme
 * @param theme Theme name to initialize with
 * @return Unique pointer to StyleManager
 *
 * Public interface method that delegates to implementation.
 */
std::unique_ptr<StyleManager> ManagerFactory::CreateStyleManager(const char *theme)
{
    return CreateStyleManagerImpl(theme);
}

/**
 * @brief Creates PreferenceManager for configuration management
 * @return Unique pointer to IConfigurationManager interface
 *
 * Public interface method that delegates to implementation.
 */
std::unique_ptr<IConfigurationManager> ManagerFactory::CreatePreferenceManager()
{
    return CreatePreferenceManagerImpl();
}

/**
 * @brief Creates InterruptManager with provided or factory-created GPIO provider
 * @param gpioProvider GPIO provider (optional, created if null)
 * @return Pointer to InterruptManager singleton
 *
 * Public interface method that uses provided GPIO provider or creates one
 * through provider factory if not supplied.
 */
InterruptManager* ManagerFactory::CreateInterruptManager(IGpioProvider* gpioProvider)
{
    
    // If no GPIO provider passed, get from provider factory
    IGpioProvider* gpioToUse = gpioProvider;
    if (!gpioToUse) {
        if (!InitializeProviders()) {
            log_e("Failed to initialize providers from factory");
            return nullptr;
        }
        gpioToUse = gpioProvider_.get();
    }
    
    return CreateInterruptManagerImpl(gpioToUse);
}

/**
 * @brief Creates ErrorManager singleton instance
 * @return Pointer to ErrorManager singleton
 *
 * Public interface method that delegates to implementation.
 */
ErrorManager* ManagerFactory::CreateErrorManager()
{
    return CreateErrorManagerImpl();
}

