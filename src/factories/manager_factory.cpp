#include "factories/manager_factory.h"
#include "factories/provider_factory.h"
#include "managers/error_manager.h"
#include "managers/interrupt_manager.h"
#include "managers/panel_manager.h"
#include "managers/preference_manager.h"
#include "managers/style_manager.h"
#include "providers/device_provider.h"
#include "sensors/button_sensor.h"
#ifdef CLARITY_DEBUG
#include "sensors/gpio_sensor.h"
#endif
#include "utilities/types.h"
#include <esp32-hal-log.h>

// Constructors

ManagerFactory::ManagerFactory(std::unique_ptr<IProviderFactory> providerFactory)
    : providerFactory_(std::move(providerFactory))
{
    if (!providerFactory_) {
        log_w("ManagerFactory created without provider factory - will create default ProviderFactory");
        providerFactory_ = std::make_unique<ProviderFactory>();
    }
}

ManagerFactory::ManagerFactory()
    : providerFactory_(std::make_unique<ProviderFactory>())
{
}

// Private Helper Methods

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

// Implementation Methods

std::unique_ptr<PanelManager> ManagerFactory::CreatePanelManagerImpl(IDisplayProvider *display, IGpioProvider *gpio,
                                                                      IStyleService *styleService,
                                                                      IPreferenceService *preferenceService,
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
        log_e("ManagerFactory: Cannot create PanelManager - IStyleService is null");
        ErrorManager::Instance().ReportCriticalError("ManagerFactory",
                                                     "Cannot create PanelManager - StyleService dependency is null");
        return nullptr;
    }
    
    if (!preferenceService)
    {
        log_e("ManagerFactory: Cannot create PanelManager - IPreferenceService is null");
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


std::unique_ptr<IPreferenceService> ManagerFactory::CreatePreferenceManagerImpl()
{
    // Create PreferenceManager for modern configuration capabilities
    auto manager = std::make_unique<PreferenceManager>();
    if (!manager)
    {
        log_e("ManagerFactory: Failed to create PreferenceManager - allocation failed");
        ErrorManager::Instance().ReportCriticalError("ManagerFactory",
                                                     "PreferenceManager allocation failed - out of memory");
        return nullptr;
    }

    return manager;
}

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

// IManagerFactory Interface Implementation

std::unique_ptr<PanelManager> ManagerFactory::CreatePanelManager(IDisplayProvider *display, IGpioProvider *gpio,
                                                                  IStyleService *styleService,
                                                                  IPreferenceService *preferenceService,
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

std::unique_ptr<StyleManager> ManagerFactory::CreateStyleManager(const char *theme)
{
    return CreateStyleManagerImpl(theme);
}

std::unique_ptr<IPreferenceService> ManagerFactory::CreatePreferenceManager()
{
    return CreatePreferenceManagerImpl();
}

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

ErrorManager* ManagerFactory::CreateErrorManager()
{
    return CreateErrorManagerImpl();
}

