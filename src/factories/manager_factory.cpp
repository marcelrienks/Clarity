#include "factories/manager_factory.h"
#include "managers/error_manager.h"
#include "managers/interrupt_manager.h"
#include "managers/panel_manager.h"
#include "managers/preference_manager.h"
#include "managers/style_manager.h"
#include "handlers/polled_handler.h"
#include "handlers/queued_handler.h"
#include "sensors/action_button_sensor.h"
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
    
    // Get the handlers from InterruptManager to access their sensors
    auto* polledHandler = interruptManager->GetPolledHandler();
    auto* queuedHandler = interruptManager->GetQueuedHandler();
    
    if (!polledHandler || !queuedHandler) {
        log_e("Cannot register interrupts - handlers not initialized");
        return;
    }
    
    // 1. Key Present Interrupt (POLLED, CRITICAL priority)
    Interrupt keyPresentInterrupt = {
        .id = "key_present",
        .priority = Priority::CRITICAL,
        .source = InterruptSource::POLLED,
        .effect = InterruptEffect::LOAD_PANEL,
        .processFunc = InterruptCallbacks::KeyPresentProcess,
        .context = polledHandler->GetKeyPresentSensor(),  // Direct sensor context
        .executionMode = InterruptExecutionMode::EXCLUSIVE,
        .exclusionGroup = "key_states",
        .canExecuteInContext = nullptr,
        .data = {},  // Union data initialized empty
        .active = true,
        .stateChanged = false,
        .lastEvaluation = 0
    };
    interruptManager->RegisterInterrupt(keyPresentInterrupt);
    
    // 2. Key Not Present Interrupt (POLLED, IMPORTANT priority)
    Interrupt keyNotPresentInterrupt = {
        .id = "key_not_present",
        .priority = Priority::IMPORTANT,
        .source = InterruptSource::POLLED,
        .effect = InterruptEffect::LOAD_PANEL,
        .processFunc = InterruptCallbacks::KeyNotPresentProcess,
        .context = polledHandler->GetKeyNotPresentSensor(),  // Direct sensor context
        .executionMode = InterruptExecutionMode::EXCLUSIVE,
        .exclusionGroup = "key_states",
        .canExecuteInContext = nullptr,
        .data = {},
        .active = true,
        .stateChanged = false,
        .lastEvaluation = 0
    };
    interruptManager->RegisterInterrupt(keyNotPresentInterrupt);
    
    // 3. Lock State Interrupt (POLLED, IMPORTANT priority)
    Interrupt lockStateInterrupt = {
        .id = "lock_state",
        .priority = Priority::IMPORTANT,
        .source = InterruptSource::POLLED,
        .effect = InterruptEffect::LOAD_PANEL,
        .processFunc = InterruptCallbacks::LockStateProcess,
        .context = polledHandler->GetLockSensor(),  // Direct sensor context
        .executionMode = InterruptExecutionMode::EXCLUSIVE,
        .exclusionGroup = "key_states",
        .canExecuteInContext = nullptr,
        .data = {},
        .active = true,
        .stateChanged = false,
        .lastEvaluation = 0
    };
    interruptManager->RegisterInterrupt(lockStateInterrupt);
    
    // 4. Lights State Interrupt (POLLED, NORMAL priority) - ALWAYS executes
    Interrupt lightsStateInterrupt = {
        .id = "lights_state",
        .priority = Priority::NORMAL,
        .source = InterruptSource::POLLED,
        .effect = InterruptEffect::SET_THEME,
        .processFunc = InterruptCallbacks::LightsStateProcess,
        .context = polledHandler->GetLightsSensor(),  // Direct sensor context
        .executionMode = InterruptExecutionMode::ALWAYS,  // Always execute for theme changes
        .exclusionGroup = nullptr,
        .canExecuteInContext = nullptr,
        .data = {},
        .active = true,
        .stateChanged = false,
        .lastEvaluation = 0
    };
    interruptManager->RegisterInterrupt(lightsStateInterrupt);
    
    // 5. Error Occurred Interrupt (POLLED, CRITICAL priority)
    Interrupt errorInterrupt = {
        .id = "error_occurred",
        .priority = Priority::CRITICAL,
        .source = InterruptSource::POLLED,
        .effect = InterruptEffect::LOAD_PANEL,
        .processFunc = InterruptCallbacks::ErrorOccurredProcess,
        .context = &ErrorManager::Instance(),  // Direct ErrorManager context
        .executionMode = InterruptExecutionMode::EXCLUSIVE,
        .exclusionGroup = nullptr,
        .canExecuteInContext = nullptr,
        .data = {},
        .active = true,
        .stateChanged = false,
        .lastEvaluation = 0
    };
    interruptManager->RegisterInterrupt(errorInterrupt);
    
    // 6. Short Press Interrupt (QUEUED, IMPORTANT priority) - CONDITIONAL based on panel
    Interrupt shortPressInterrupt = {
        .id = "universal_short_press",
        .priority = Priority::IMPORTANT,
        .source = InterruptSource::QUEUED,
        .effect = InterruptEffect::BUTTON_ACTION,
        .processFunc = InterruptCallbacks::ShortPressProcess,
        .context = queuedHandler->GetButtonSensor(),  // Direct sensor context
        .executionMode = InterruptExecutionMode::CONDITIONAL,
        .exclusionGroup = nullptr,
        .canExecuteInContext = nullptr,  // TODO: Add context check for config panel restriction
        .data = {},
        .active = true,
        .stateChanged = false,
        .lastEvaluation = 0
    };
    interruptManager->RegisterInterrupt(shortPressInterrupt);
    
    // 7. Long Press Interrupt (QUEUED, NORMAL priority) - CONDITIONAL based on panel
    Interrupt longPressInterrupt = {
        .id = "universal_long_press",
        .priority = Priority::NORMAL,
        .source = InterruptSource::QUEUED,
        .effect = InterruptEffect::BUTTON_ACTION,
        .processFunc = InterruptCallbacks::LongPressProcess,
        .context = queuedHandler->GetButtonSensor(),  // Direct sensor context
        .executionMode = InterruptExecutionMode::CONDITIONAL,
        .exclusionGroup = nullptr,
        .canExecuteInContext = nullptr,  // TODO: Add context check for config panel restriction
        .data = {},
        .active = true,
        .stateChanged = false,
        .lastEvaluation = 0
    };
    interruptManager->RegisterInterrupt(longPressInterrupt);
    
    log_i("Successfully registered 7 system interrupts with static callbacks");
}