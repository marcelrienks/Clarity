#include "factories/manager_factory.h"
#include "managers/error_manager.h"
#include "managers/interrupt_manager.h"
#include "managers/panel_manager.h"
#include "managers/preference_manager.h"
#include "managers/style_manager.h"
#include "handlers/polled_handler.h"
#include "handlers/queued_handler.h"
#include "sensors/button_sensor.h"
#ifdef CLARITY_DEBUG
#include "sensors/debug_error_sensor.h"
#endif
#include "utilities/types.h"
#include "utilities/interrupt_callbacks.h"
#include <esp32-hal-log.h>

// Factory Methods

std::unique_ptr<PanelManager> ManagerFactory::createPanelManager(IDisplayProvider *display, IGpioProvider *gpio,
                                                                 IStyleService *styleService,
                                                                 IPreferenceService *preferenceService,
                                                                 InterruptManager *interruptManager)
{
    log_v("createPanelManager() called");

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

std::unique_ptr<StyleManager> ManagerFactory::createStyleManager(const char *theme)
{
    log_v("createStyleManager() called");

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


std::unique_ptr<PreferenceManager> ManagerFactory::createPreferenceManager()
{
    log_v("createPreferenceManager() called");

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

InterruptManager* ManagerFactory::createInterruptManager(IGpioProvider* gpioProvider)
{
    log_v("createInterruptManager() called");

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

ErrorManager *ManagerFactory::createErrorManager()
{
    log_v("createErrorManager() called");

    ErrorManager *errorManager = &ErrorManager::Instance();
    if (!errorManager)
    {
        log_e("ManagerFactory: Failed to get ErrorManager singleton instance");
        return nullptr;
    }

    log_d("ManagerFactory: ErrorManager created successfully");
    return errorManager;
}
