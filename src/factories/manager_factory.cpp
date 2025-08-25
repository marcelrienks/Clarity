#include "factories/manager_factory.h"
#include "managers/action_manager.h"
#include "managers/error_manager.h"
#include "managers/interrupt_manager.h"
#include "managers/panel_manager.h"
#include "managers/preference_manager.h"
#include "managers/style_manager.h"
#include "sensors/action_button_sensor.h"
#include "sensors/debug_error_sensor.h"
#include "sensors/key_sensor.h"
#include "sensors/lights_sensor.h"
#include "sensors/lock_sensor.h"
#include "providers/gpio_provider.h"
#include "utilities/types.h"
#include <esp32-hal-log.h>

// Factory Methods

std::unique_ptr<PanelManager> ManagerFactory::createPanelManager(IDisplayProvider *display, IGpioProvider *gpio,
                                                                 IStyleService *styleService,
                                                                 IActionManager *actionManager,
                                                                 IPreferenceService *preferenceService)
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

    auto panelManager = std::make_unique<PanelManager>(display, gpio, styleService, actionManager, preferenceService);
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

InterruptManager* ManagerFactory::createInterruptManager()
{
    log_v("createInterruptManager() called");

    InterruptManager* manager = &InterruptManager::Instance();
    if (!manager)
    {
        log_e("ManagerFactory: Failed to get InterruptManager singleton instance");
        return nullptr;
    }

    log_d("ManagerFactory: Initializing InterruptManager singleton...");
    manager->Init();
    
    // Register all system interrupts after initialization
    RegisterSystemInterrupts(manager);
    
    log_d("ManagerFactory: InterruptManager initialized successfully");
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

// Static callback functions for interrupt system
static bool EvaluateLightsChange(void* context)
{
    log_v("EvaluateLightsChange() called");
    auto* sensor = static_cast<LightsSensor*>(context);
    if (!sensor)
    {
        log_e("Null sensor in lights interrupt evaluation");
        return false;
    }
    return sensor->HasStateChanged();
}

static void ExecuteLightsTheme(void* context)
{
    log_v("ExecuteLightsTheme() called");
    auto* sensor = static_cast<LightsSensor*>(context);
    if (!sensor)
    {
        log_e("Null sensor in lights interrupt execution");
        return;
    }
    
    // Get current lights state and set appropriate theme
    bool lightsOn = sensor->GetLightsState();
    const char* newTheme = lightsOn ? Themes::DAY : Themes::NIGHT;
    
    // Apply theme change through StyleManager
    // TODO: We need access to StyleManager here - this needs to be passed differently
    log_i("Lights changed - would set theme to %s (lights %s)", newTheme, lightsOn ? "ON" : "OFF");
}

static bool EvaluateKeyChange(void* context)
{
    log_v("EvaluateKeyChange() called");
    auto* sensor = static_cast<KeySensor*>(context);
    if (!sensor)
    {
        log_e("Null sensor in key interrupt evaluation");
        return false;
    }
    return sensor->HasStateChanged();
}

static void ExecuteKeyPanel(void* context)
{
    log_v("ExecuteKeyPanel() called");
    // For now, just log - we'll need PanelService access
    log_i("Key state changed - would load KEY panel");
}

static bool EvaluateLockChange(void* context)
{
    log_v("EvaluateLockChange() called");
    auto* sensor = static_cast<LockSensor*>(context);
    if (!sensor)
    {
        log_e("Null sensor in lock interrupt evaluation");
        return false;
    }
    return sensor->HasStateChanged();
}

static void ExecuteLockPanel(void* context)
{
    log_v("ExecuteLockPanel() called");
    // For now, just log - we'll need PanelService access
    log_i("Lock state changed - would load LOCK panel");
}

// Private helper function to register all system interrupts
void ManagerFactory::RegisterSystemInterrupts(InterruptManager* interruptManager)
{
    log_v("RegisterSystemInterrupts() called");
    
    if (!interruptManager)
    {
        log_e("Cannot register interrupts - InterruptManager is null");
        return;
    }
    
    // Create GPIO provider for sensors
    auto gpioProvider = std::make_unique<GpioProvider>();
    if (!gpioProvider)
    {
        log_e("Failed to create GPIO provider for interrupt sensors");
        return;
    }
    
    // Create sensors for interrupt registration
    auto keySensor = std::make_shared<KeySensor>(gpioProvider.get());
    auto lockSensor = std::make_shared<LockSensor>(gpioProvider.get());
    auto lightsSensor = std::make_shared<LightsSensor>(gpioProvider.get());
    auto debugErrorSensor = std::make_shared<DebugErrorSensor>(gpioProvider.get());
    
    if (!keySensor || !lockSensor || !lightsSensor || !debugErrorSensor)
    {
        log_e("Failed to create sensors for interrupt registration");
        return;
    }
    
    // Initialize sensors
    keySensor->Init();
    lockSensor->Init();
    lightsSensor->Init();
    debugErrorSensor->Init();
    
    // Register lights interrupt (this was the missing piece!)
    Interrupt lightsInterrupt = {};
    lightsInterrupt.id = "lights_state";
    lightsInterrupt.priority = Priority::NORMAL;
    lightsInterrupt.source = InterruptSource::POLLED;
    lightsInterrupt.effect = InterruptEffect::SET_THEME;
    lightsInterrupt.evaluationFunc = EvaluateLightsChange;
    lightsInterrupt.executionFunc = ExecuteLightsTheme;
    lightsInterrupt.context = lightsSensor.get();
    lightsInterrupt.active = false;
    
    bool lightsRegistered = interruptManager->RegisterInterrupt(lightsInterrupt);
    if (lightsRegistered)
    {
        log_i("Registered lights interrupt successfully");
    }
    else
    {
        log_e("Failed to register lights interrupt");
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "ManagerFactory", 
                                           "Failed to register lights interrupt");
    }
    
    // Register key present interrupt
    Interrupt keyPresentInterrupt = {};
    keyPresentInterrupt.id = "key_present";
    keyPresentInterrupt.priority = Priority::CRITICAL;
    keyPresentInterrupt.source = InterruptSource::POLLED;
    keyPresentInterrupt.effect = InterruptEffect::LOAD_PANEL;
    keyPresentInterrupt.evaluationFunc = EvaluateKeyChange;
    keyPresentInterrupt.executionFunc = ExecuteKeyPanel;
    keyPresentInterrupt.context = keySensor.get();
    keyPresentInterrupt.active = false;
    
    bool keyRegistered = interruptManager->RegisterInterrupt(keyPresentInterrupt);
    if (keyRegistered)
    {
        log_i("Registered key interrupt successfully");
    }
    else
    {
        log_e("Failed to register key interrupt");
    }
    
    // Register lock interrupt
    Interrupt lockInterrupt = {};
    lockInterrupt.id = "lock_state";
    lockInterrupt.priority = Priority::IMPORTANT;
    lockInterrupt.source = InterruptSource::POLLED;
    lockInterrupt.effect = InterruptEffect::LOAD_PANEL;
    lockInterrupt.evaluationFunc = EvaluateLockChange;
    lockInterrupt.executionFunc = ExecuteLockPanel;
    lockInterrupt.context = lockSensor.get();
    lockInterrupt.active = false;
    
    bool lockRegistered = interruptManager->RegisterInterrupt(lockInterrupt);
    if (lockRegistered)
    {
        log_i("Registered lock interrupt successfully");
    }
    else
    {
        log_e("Failed to register lock interrupt");
    }
    
    log_d("System interrupt registration completed");
}