#include "factories/manager_factory.h"
#include "managers/action_manager.h"
#include "managers/error_manager.h"
#include "managers/interrupt_manager.h"
#include "managers/panel_manager.h"
#include "managers/preference_manager.h"
#include "managers/style_manager.h"
#include "managers/trigger_manager.h"
#include "sensors/action_button_sensor.h"
#include "sensors/debug_error_sensor.h"
#include "sensors/key_sensor.h"
#include "sensors/lights_sensor.h"
#include "sensors/lock_sensor.h"
#include "utilities/types.h"
#include <esp32-hal-log.h>

// Factory Methods

std::unique_ptr<PanelManager> ManagerFactory::createPanelManager(IDisplayProvider *display, IGpioProvider *gpio,
                                                                 IStyleService *styleService,
                                                                 IActionManager *actionManager,
                                                                 IPreferenceService *preferenceService)
{
    log_v("createPanelManager() called");
    log_d("ManagerFactory: Creating PanelManager (UI panel coordination)...");

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
    if (!actionManager)
    {
        log_e("ManagerFactory: Cannot create PanelManager - IActionManager is null");
        ErrorManager::Instance().ReportCriticalError("ManagerFactory",
                                                     "Cannot create PanelManager - ActionManager dependency is null");
        return nullptr;
    }
    if (!preferenceService)
    {
        log_e("ManagerFactory: Cannot create PanelManager - IPreferenceService is null");
        ErrorManager::Instance().ReportCriticalError(
            "ManagerFactory", "Cannot create PanelManager - PreferenceService dependency is null");
        return nullptr;
    }

    auto manager = std::make_unique<PanelManager>(display, gpio, styleService, actionManager, preferenceService);
    if (!manager)
    {
        log_e("ManagerFactory: Failed to create PanelManager - allocation failed");
        ErrorManager::Instance().ReportCriticalError("ManagerFactory",
                                                     "PanelManager allocation failed - out of memory");
        return nullptr;
    }

    log_d("ManagerFactory: Initializing PanelManager...");
    manager->Init();
    log_d("ManagerFactory: PanelManager created successfully");
    return manager;
}

std::unique_ptr<StyleManager> ManagerFactory::createStyleManager(const char *theme)
{
    log_v("createStyleManager() called");
    log_d("ManagerFactory: Creating StyleManager (LVGL theme system) with theme: %s", theme ? theme : "default");

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

std::unique_ptr<TriggerManager> ManagerFactory::createTriggerManager(IGpioProvider *gpio, IPanelService *panelService,
                                                                     IStyleService *styleService)
{
    log_v("createTriggerManager() called");
    log_d("ManagerFactory: Creating TriggerManager (sensor monitoring system)...");

    if (!gpio)
    {
        log_e("ManagerFactory: Cannot create TriggerManager - IGpioProvider is null");
        ErrorManager::Instance().ReportCriticalError("ManagerFactory",
                                                     "Cannot create TriggerManager - GpioProvider dependency is null");
        return nullptr;
    }
    if (!panelService)
    {
        log_e("ManagerFactory: Cannot create TriggerManager - IPanelService is null");
        ErrorManager::Instance().ReportCriticalError("ManagerFactory",
                                                     "Cannot create TriggerManager - PanelService dependency is null");
        return nullptr;
    }
    if (!styleService)
    {
        log_e("ManagerFactory: Cannot create TriggerManager - IStyleService is null");
        ErrorManager::Instance().ReportCriticalError("ManagerFactory",
                                                     "Cannot create TriggerManager - StyleService dependency is null");
        return nullptr;
    }

    // Create sensors that TriggerManager needs
    auto keySensor = std::make_shared<KeySensor>(gpio);
    auto lockSensor = std::make_shared<LockSensor>(gpio);
    auto lightSensor = std::make_shared<LightsSensor>(gpio);
    auto debugErrorSensor = std::make_shared<DebugErrorSensor>(gpio);

    if (!keySensor || !lockSensor || !lightSensor || !debugErrorSensor)
    {
        log_e("ManagerFactory: Failed to create sensors for TriggerManager");
        ErrorManager::Instance().ReportCriticalError("ManagerFactory",
                                                     "TriggerManager sensor allocation failed - out of memory");
        return nullptr;
    }

    auto manager = std::make_unique<TriggerManager>(keySensor, lockSensor, lightSensor, debugErrorSensor, panelService,
                                                    styleService);
    if (!manager)
    {
        log_e("ManagerFactory: Failed to create TriggerManager - allocation failed");
        ErrorManager::Instance().ReportCriticalError("ManagerFactory",
                                                     "TriggerManager allocation failed - out of memory");
        return nullptr;
    }

    log_d("ManagerFactory: Initializing TriggerManager with sensors...");
    manager->Init();
    log_d("ManagerFactory: TriggerManager created successfully");
    return manager;
}

std::unique_ptr<PreferenceManager> ManagerFactory::createPreferenceManager()
{
    log_v("createPreferenceManager() called");
    log_d("ManagerFactory: Creating PreferenceManager (persistent settings)...");

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

std::unique_ptr<ActionManager> ManagerFactory::createActionManager(IGpioProvider *gpio, IPanelService *panelService)
{
    log_v("createActionManager() called");
    if (!gpio)
    {
        log_e("ManagerFactory: Cannot create ActionManager - IGpioProvider is null");
        ErrorManager::Instance().ReportCriticalError("ManagerFactory",
                                                     "Cannot create ActionManager - GpioProvider dependency is null");
        return nullptr;
    }

    // Create ActionButtonSensor that ActionManager needs
    auto actionButtonSensor = std::make_shared<ActionButtonSensor>(gpio);
    if (!actionButtonSensor)
    {
        log_e("ManagerFactory: Failed to create ActionButtonSensor for ActionManager");
        ErrorManager::Instance().ReportCriticalError("ManagerFactory",
                                                     "ActionButtonSensor allocation failed - out of memory");
        return nullptr;
    }

    auto manager = std::make_unique<ActionManager>(actionButtonSensor, panelService);
    if (!manager)
    {
        log_e("ManagerFactory: Failed to create ActionManager - allocation failed");
        ErrorManager::Instance().ReportCriticalError("ManagerFactory",
                                                     "ActionManager allocation failed - out of memory");
        return nullptr;
    }

    manager->Init();
    log_d("ManagerFactory: ActionManager created and initialized successfully");
    return manager;
}

std::unique_ptr<InterruptManager> ManagerFactory::createInterruptManager(IPanelService *panelService)
{
    log_v("createInterruptManager() called");
    log_d("ManagerFactory: Creating InterruptManager (interrupt coordination)...");

    auto manager = std::make_unique<InterruptManager>(panelService);
    if (!manager)
    {
        log_e("ManagerFactory: Failed to create InterruptManager - allocation failed");
        ErrorManager::Instance().ReportCriticalError("ManagerFactory",
                                                     "InterruptManager allocation failed - out of memory");
        return nullptr;
    }

    log_d("ManagerFactory: InterruptManager created successfully with panel service: %s",
          panelService ? "provided" : "null");
    return manager;
}

ErrorManager *ManagerFactory::createErrorManager()
{
    log_v("createErrorManager() called");
    log_d("ManagerFactory: Creating ErrorManager (system error tracking)...");

    ErrorManager *errorManager = &ErrorManager::Instance();
    if (!errorManager)
    {
        log_e("ManagerFactory: Failed to get ErrorManager singleton instance");
        return nullptr;
    }

    log_d("ManagerFactory: ErrorManager created successfully");
    return errorManager;
}