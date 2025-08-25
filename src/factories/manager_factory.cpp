#include "factories/manager_factory.h"
#include "managers/action_manager.h"
#include "managers/error_manager.h"
#include "managers/interrupt_manager.h"
#include "managers/panel_manager.h"
#include "managers/preference_manager.h"
#include "managers/style_manager.h"
#include "sensors/action_button_sensor.h"
#include "sensors/debug_error_sensor.h"
#include "utilities/types.h"
#include "utilities/interrupt_callbacks.h"
#include <esp32-hal-log.h>

// Factory Methods

std::unique_ptr<PanelManager> ManagerFactory::createPanelManager(IDisplayProvider *display, IGpioProvider *gpio,
                                                                 IStyleService *styleService,
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
    // ActionManager dependency removed - button handling moved to handler-based system
    if (!preferenceService)
    {
        log_e("ManagerFactory: Cannot create PanelManager - IPreferenceService is null");
        ErrorManager::Instance().ReportCriticalError(
            "ManagerFactory", "Cannot create PanelManager - PreferenceService dependency is null");
        return nullptr;
    }

    auto panelManager = std::make_unique<PanelManager>(display, gpio, styleService, preferenceService);
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

// createActionManager method removed - button handling moved to handler-based system

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
    
    // Register all system interrupts after initialization
    RegisterSystemInterrupts(manager);
    
    log_d("ManagerFactory: InterruptManager initialized successfully with GPIO provider");
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


// Private helper function to register all system interrupts
void ManagerFactory::RegisterSystemInterrupts(InterruptManager* interruptManager)
{
    log_v("RegisterSystemInterrupts() called");
    
    if (!interruptManager)
    {
        log_e("Cannot register interrupts - InterruptManager is null");
        return;
    }
    
    log_d("Registering all 7 system interrupts with static callbacks...");
    
    // Get handler sensors for context (they're created during handler initialization)
    // We'll access them through the handlers themselves since they own the sensors
    
    // 1. Key Present Interrupt (POLLED, CRITICAL priority)
    Interrupt keyPresentInterrupt = {
        .id = "key_present",
        .priority = Priority::CRITICAL,
        .source = InterruptSource::POLLED,
        .effect = InterruptEffect::LOAD_PANEL,
        .evaluationFunc = InterruptCallbacks::KeyPresentChanged,
        .executionFunc = InterruptCallbacks::LoadKeyPanel,
        .context = nullptr,  // Will be set by handler when it registers
        .active = true,
        .lastEvaluation = 0
    };
    interruptManager->RegisterInterrupt(keyPresentInterrupt);
    
    // 2. Key Not Present Interrupt (POLLED, IMPORTANT priority)
    Interrupt keyNotPresentInterrupt = {
        .id = "key_not_present",
        .priority = Priority::IMPORTANT,
        .source = InterruptSource::POLLED,
        .effect = InterruptEffect::LOAD_PANEL,
        .evaluationFunc = InterruptCallbacks::KeyNotPresentChanged,
        .executionFunc = InterruptCallbacks::RestoreFromKeyPanel,
        .context = nullptr,  // Will be set by handler when it registers
        .active = true,
        .lastEvaluation = 0
    };
    interruptManager->RegisterInterrupt(keyNotPresentInterrupt);
    
    // 3. Lock State Interrupt (POLLED, IMPORTANT priority)
    Interrupt lockStateInterrupt = {
        .id = "lock_state",
        .priority = Priority::IMPORTANT,
        .source = InterruptSource::POLLED,
        .effect = InterruptEffect::LOAD_PANEL,
        .evaluationFunc = InterruptCallbacks::LockStateChanged,
        .executionFunc = InterruptCallbacks::LoadLockPanel,
        .context = nullptr,  // Will be set by handler when it registers
        .active = true,
        .lastEvaluation = 0
    };
    interruptManager->RegisterInterrupt(lockStateInterrupt);
    
    // 4. Lights State Interrupt (POLLED, NORMAL priority)
    Interrupt lightsStateInterrupt = {
        .id = "lights_state",
        .priority = Priority::NORMAL,
        .source = InterruptSource::POLLED,
        .effect = InterruptEffect::SET_THEME,
        .evaluationFunc = InterruptCallbacks::LightsStateChanged,
        .executionFunc = InterruptCallbacks::SetThemeBasedOnLights,
        .context = nullptr,  // Will be set by handler when it registers
        .active = true,
        .lastEvaluation = 0
    };
    interruptManager->RegisterInterrupt(lightsStateInterrupt);
    
    // 5. Error Occurred Interrupt (POLLED, CRITICAL priority)
    Interrupt errorInterrupt = {
        .id = "error_occurred",
        .priority = Priority::CRITICAL,
        .source = InterruptSource::POLLED,
        .effect = InterruptEffect::LOAD_PANEL,
        .evaluationFunc = InterruptCallbacks::ErrorOccurred,
        .executionFunc = InterruptCallbacks::LoadErrorPanel,
        .context = nullptr,  // Context will be ErrorManager instance
        .active = true,
        .lastEvaluation = 0
    };
    interruptManager->RegisterInterrupt(errorInterrupt);
    
    // 6. Short Press Interrupt (QUEUED, IMPORTANT priority)
    Interrupt shortPressInterrupt = {
        .id = "universal_short_press",
        .priority = Priority::IMPORTANT,
        .source = InterruptSource::QUEUED,
        .effect = InterruptEffect::BUTTON_ACTION,
        .evaluationFunc = InterruptCallbacks::HasShortPressEvent,
        .executionFunc = InterruptCallbacks::ExecutePanelShortPress,
        .context = nullptr,  // Will be set by handler when it registers
        .active = true,
        .lastEvaluation = 0
    };
    interruptManager->RegisterInterrupt(shortPressInterrupt);
    
    // 7. Long Press Interrupt (QUEUED, NORMAL priority) 
    Interrupt longPressInterrupt = {
        .id = "universal_long_press",
        .priority = Priority::NORMAL,
        .source = InterruptSource::QUEUED,
        .effect = InterruptEffect::BUTTON_ACTION,
        .evaluationFunc = InterruptCallbacks::HasLongPressEvent,
        .executionFunc = InterruptCallbacks::ExecutePanelLongPress,
        .context = nullptr,  // Will be set by handler when it registers
        .active = true,
        .lastEvaluation = 0
    };
    interruptManager->RegisterInterrupt(longPressInterrupt);
    
    log_i("Successfully registered 7 system interrupts with static callbacks");
}