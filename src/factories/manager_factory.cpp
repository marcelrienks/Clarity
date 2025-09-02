#include "factories/manager_factory.h"
#include "managers/error_manager.h"
#include "managers/interrupt_manager.h"
#include "managers/panel_manager.h"
#include "managers/preference_manager.h"
#include "managers/style_manager.h"
#include "sensors/button_sensor.h"
#ifdef CLARITY_DEBUG
#include "sensors/debug_error_sensor.h"
#endif
#include "utilities/types.h"
#include "utilities/interrupt_callbacks.h"
#include <esp32-hal-log.h>

// Implementation Methods

std::unique_ptr<PanelManager> ManagerFactory::CreatePanelManagerImpl(IDisplayProvider *display, IGpioProvider *gpio,
                                                                      IStyleService *styleService,
                                                                      IPreferenceService *preferenceService,
                                                                      InterruptManager *interruptManager)
{
    log_v("CreatePanelManager() called");

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

    log_d("ManagerFactory: Initializing PanelManager...");
    panelManager->Init();
    
    log_d("ManagerFactory: PanelManager created successfully");
    return panelManager;
}

std::unique_ptr<StyleManager> ManagerFactory::CreateStyleManagerImpl(const char *theme)
{
    log_v("CreateStyleManager() called");

    auto manager = std::make_unique<StyleManager>(theme ? theme : Themes::DAY);
    if (!manager)
    {
        log_e("ManagerFactory: Failed to create StyleManager - allocation failed");
        ErrorManager::Instance().ReportCriticalError("ManagerFactory",
                                                     "StyleManager allocation failed - out of memory");
        return nullptr;
    }

    log_d("ManagerFactory: StyleManager created successfully");
    return manager;
}


std::unique_ptr<PreferenceManager> ManagerFactory::CreatePreferenceManagerImpl()
{
    log_v("CreatePreferenceManager() called");

    auto manager = std::make_unique<PreferenceManager>();
    if (!manager)
    {
        log_e("ManagerFactory: Failed to create PreferenceManager - allocation failed");
        ErrorManager::Instance().ReportCriticalError("ManagerFactory",
                                                     "PreferenceManager allocation failed - out of memory");
        return nullptr;
    }

    log_d("ManagerFactory: Initializing PreferenceManager...");
    manager->Init();
    log_d("ManagerFactory: PreferenceManager created successfully");
    return manager;
}

InterruptManager* ManagerFactory::CreateInterruptManagerImpl(IGpioProvider* gpioProvider)
{
    log_v("CreateInterruptManager() called");

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

    log_d("ManagerFactory: Initializing InterruptManager singleton with GPIO provider...");
    manager->Init(gpioProvider);
    
    // IMPORTANT: InterruptManager::Init() creates handlers which own the sensors
    // We must register interrupts AFTER Init() completes so contexts are available
    
    log_d("ManagerFactory: InterruptManager initialized successfully with GPIO provider (system interrupts registered internally)");
    return manager;
}

ErrorManager *ManagerFactory::CreateErrorManagerImpl()
{
    log_v("CreateErrorManager() called");

    ErrorManager *errorManager = &ErrorManager::Instance();
    if (!errorManager)
    {
        log_e("ManagerFactory: Failed to get ErrorManager singleton instance");
        return nullptr;
    }

    log_d("ManagerFactory: ErrorManager created successfully");
    return errorManager;
}

// IManagerFactory Interface Implementation

std::unique_ptr<PanelManager> ManagerFactory::CreatePanelManager(IDisplayProvider *display, IGpioProvider *gpio,
                                                                  IStyleService *styleService,
                                                                  IPreferenceService *preferenceService,
                                                                  InterruptManager *interruptManager)
{
    return CreatePanelManagerImpl(display, gpio, styleService, preferenceService, interruptManager);
}

std::unique_ptr<StyleManager> ManagerFactory::CreateStyleManager(const char *theme)
{
    return CreateStyleManagerImpl(theme);
}

std::unique_ptr<PreferenceManager> ManagerFactory::CreatePreferenceManager()
{
    return CreatePreferenceManagerImpl();
}

InterruptManager* ManagerFactory::CreateInterruptManager(IGpioProvider* gpioProvider)
{
    return CreateInterruptManagerImpl(gpioProvider);
}

ErrorManager* ManagerFactory::CreateErrorManager()
{
    return CreateErrorManagerImpl();
}

// Static Convenience Methods (for backward compatibility)

std::unique_ptr<PanelManager> ManagerFactory::CreatePanelManagerStatic(IDisplayProvider *display, IGpioProvider *gpio,
                                                                        IStyleService *styleService,
                                                                        IPreferenceService *preferenceService,
                                                                        InterruptManager *interruptManager)
{
    return CreatePanelManagerImpl(display, gpio, styleService, preferenceService, interruptManager);
}

std::unique_ptr<StyleManager> ManagerFactory::CreateStyleManagerStatic(const char *theme)
{
    return CreateStyleManagerImpl(theme);
}

std::unique_ptr<PreferenceManager> ManagerFactory::CreatePreferenceManagerStatic()
{
    return CreatePreferenceManagerImpl();
}

InterruptManager* ManagerFactory::CreateInterruptManagerStatic(IGpioProvider* gpioProvider)
{
    return CreateInterruptManagerImpl(gpioProvider);
}

ErrorManager* ManagerFactory::CreateErrorManagerStatic()
{
    return CreateErrorManagerImpl();
}
