#include "factories/manager_factory.h"
#include "utilities/types.h"
#include "managers/panel_manager.h"
#include "managers/style_manager.h"
#include "managers/trigger_manager.h"
#include "managers/preference_manager.h"
#include "managers/action_manager.h"
#include "managers/interrupt_manager.h"
#include "managers/error_manager.h"
#include "sensors/key_sensor.h"
#include "sensors/lock_sensor.h"
#include "sensors/light_sensor.h"
#include "sensors/debug_error_sensor.h"
#include "sensors/input_button_sensor.h"
#include <esp32-hal-log.h>

// Factory Methods

std::unique_ptr<PanelManager> ManagerFactory::createPanelManager(IDisplayProvider *display, IGpioProvider *gpio, IStyleService *styleService, ActionManager* actionManager)
{
    log_d("ManagerFactory: Creating PanelManager (UI panel coordination)...");
    
    if (!display) {
        log_e("ManagerFactory: Cannot create PanelManager - IDisplayProvider is null");
        return nullptr;
    }
    if (!gpio) {
        log_e("ManagerFactory: Cannot create PanelManager - IGpioProvider is null");
        return nullptr;
    }
    if (!styleService) {
        log_e("ManagerFactory: Cannot create PanelManager - IStyleService is null");
        return nullptr;
    }
    if (!actionManager) {
        log_e("ManagerFactory: Cannot create PanelManager - ActionManager is null");
        return nullptr;
    }
    
    auto manager = std::make_unique<PanelManager>(display, gpio, styleService, actionManager);
    if (!manager) {
        log_e("ManagerFactory: Failed to create PanelManager - allocation failed");
        return nullptr;
    }
    
    log_d("ManagerFactory: Initializing PanelManager...");
    manager->Init();
    log_d("ManagerFactory: PanelManager created successfully");
    return manager;
}

std::unique_ptr<StyleManager> ManagerFactory::createStyleManager(const char *theme)
{
    log_d("ManagerFactory: Creating StyleManager (LVGL theme system) with theme: %s", theme ? theme : "default");
    
    auto manager = std::make_unique<StyleManager>(theme ? theme : Themes::DAY);
    if (!manager) {
        log_e("ManagerFactory: Failed to create StyleManager - allocation failed");
        return nullptr;
    }
    
    log_d("ManagerFactory: StyleManager created successfully");
    return manager;
}

std::unique_ptr<TriggerManager> ManagerFactory::createTriggerManager(IGpioProvider *gpio, IPanelService *panelService, IStyleService *styleService)
{
    log_d("ManagerFactory: Creating TriggerManager (sensor monitoring system)...");
    
    if (!gpio) {
        log_e("ManagerFactory: Cannot create TriggerManager - IGpioProvider is null");
        return nullptr;
    }
    if (!panelService) {
        log_e("ManagerFactory: Cannot create TriggerManager - IPanelService is null");
        return nullptr;
    }
    if (!styleService) {
        log_e("ManagerFactory: Cannot create TriggerManager - IStyleService is null");
        return nullptr;
    }
    
    // Create sensors that TriggerManager needs
    auto keySensor = std::make_shared<KeySensor>(gpio);
    auto lockSensor = std::make_shared<LockSensor>(gpio);
    auto lightSensor = std::make_shared<LightSensor>(gpio);
    auto debugErrorSensor = std::make_shared<DebugErrorSensor>(gpio);
    
    if (!keySensor || !lockSensor || !lightSensor || !debugErrorSensor) {
        log_e("ManagerFactory: Failed to create sensors for TriggerManager");
        return nullptr;
    }
    
    auto manager = std::make_unique<TriggerManager>(keySensor, lockSensor, lightSensor, debugErrorSensor, panelService, styleService);
    if (!manager) {
        log_e("ManagerFactory: Failed to create TriggerManager - allocation failed");
        return nullptr;
    }
    
    log_d("ManagerFactory: Initializing TriggerManager with sensors...");
    manager->Init();
    log_d("ManagerFactory: TriggerManager created successfully");
    return manager;
}

std::unique_ptr<PreferenceManager> ManagerFactory::createPreferenceManager()
{
    log_d("ManagerFactory: Creating PreferenceManager (persistent settings)...");
    
    auto manager = std::make_unique<PreferenceManager>();
    if (!manager) {
        log_e("ManagerFactory: Failed to create PreferenceManager - allocation failed");
        return nullptr;
    }
    
    log_d("ManagerFactory: Initializing PreferenceManager...");
    manager->Init();
    log_d("ManagerFactory: PreferenceManager created successfully");
    return manager;
}

std::unique_ptr<ActionManager> ManagerFactory::createActionManager(IGpioProvider* gpio, IPanelService* panelService)
{
    log_d("ManagerFactory: Creating ActionManager (button input system)...");
    
    if (!gpio) {
        log_e("ManagerFactory: Cannot create ActionManager - IGpioProvider is null");
        return nullptr;
    }
    
    // Create InputButtonSensor that ActionManager needs
    auto inputButtonSensor = std::make_shared<InputButtonSensor>(gpio);
    if (!inputButtonSensor) {
        log_e("ManagerFactory: Failed to create InputButtonSensor for ActionManager");
        return nullptr;
    }
    
    auto manager = std::make_unique<ActionManager>(inputButtonSensor);
    if (!manager) {
        log_e("ManagerFactory: Failed to create ActionManager - allocation failed");
        return nullptr;
    }
    
    log_d("ManagerFactory: Initializing ActionManager...");
    manager->Init();
    log_d("ManagerFactory: ActionManager created successfully");
    return manager;
}

std::unique_ptr<InterruptManager> ManagerFactory::createInterruptManager()
{
    log_d("ManagerFactory: Creating InterruptManager (interrupt coordination)...");
    
    auto manager = std::make_unique<InterruptManager>();
    if (!manager) {
        log_e("ManagerFactory: Failed to create InterruptManager - allocation failed");
        return nullptr;
    }
    
    log_d("ManagerFactory: InterruptManager created successfully");
    return manager;
}

ErrorManager* ManagerFactory::createErrorManager()
{
    log_d("ManagerFactory: Creating ErrorManager (system error tracking)...");
    
    ErrorManager* errorManager = &ErrorManager::Instance();
    if (!errorManager) {
        log_e("ManagerFactory: Failed to get ErrorManager singleton instance");
        return nullptr;
    }
    
    log_d("ManagerFactory: ErrorManager created successfully");
    return errorManager;
}