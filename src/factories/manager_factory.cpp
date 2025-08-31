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
    
    log_d("Registering all 7 system interrupts...");
    
    // Get the handlers from InterruptManager to access their sensors
    auto* polledHandler = interruptManager->GetPolledHandler();
    auto* queuedHandler = interruptManager->GetQueuedHandler();
    
    if (!polledHandler || !queuedHandler) {
        log_e("Cannot register interrupts - handlers not initialized");
        return;
    }
    
    // 1. Key Present Interrupt - Single purpose: Load KEY panel and save restoration point
    Interrupt keyPresentInterrupt = {
        .id = TriggerIds::KEY_PRESENT,
        .priority = Priority::CRITICAL,
        .source = InterruptSource::POLLED,
        .execute = InterruptCallbacks::KeyPresentActivate,
        .context = polledHandler->GetKeyPresentSensor(),
        .data = {.panelName = PanelNames::KEY},
        .blocking = false,  // Key panel doesn't block restoration
        .flags = InterruptFlags::NONE
    };
    interruptManager->RegisterInterrupt(keyPresentInterrupt);
    
    // 2. Key Not Present Interrupt - Single purpose: Load KEY panel and save restoration point  
    Interrupt keyNotPresentInterrupt = {
        .id = TriggerIds::KEY_NOT_PRESENT,
        .priority = Priority::IMPORTANT,
        .source = InterruptSource::POLLED,
        .execute = InterruptCallbacks::KeyNotPresentActivate,
        .context = polledHandler->GetKeyNotPresentSensor(),
        .data = {.panelName = PanelNames::KEY},
        .blocking = false,  // Key panel doesn't block restoration
        .flags = InterruptFlags::NONE
    };
    interruptManager->RegisterInterrupt(keyNotPresentInterrupt);
    
    // 3. Lock Engaged Interrupt - Single purpose: Load LOCK panel and save restoration point
    Interrupt lockEngagedInterrupt = {
        .id = "lock_engaged",
        .priority = Priority::IMPORTANT,
        .source = InterruptSource::POLLED,
        .execute = InterruptCallbacks::LockEngagedActivate,
        .context = polledHandler->GetLockSensor(),
        .data = {.panelName = PanelNames::LOCK},
        .blocking = true,  // Lock panel blocks restoration
        .flags = InterruptFlags::NONE
    };
    interruptManager->RegisterInterrupt(lockEngagedInterrupt);
    
    // 4. Lock Disengaged Interrupt - Single purpose: Restore to previous panel if no other blocking interrupts
    Interrupt lockDisengagedInterrupt = {
        .id = "lock_disengaged",
        .priority = Priority::IMPORTANT,
        .source = InterruptSource::POLLED,
        .execute = InterruptCallbacks::LockDisengagedActivate,
        .context = polledHandler->GetLockSensor(),
        .data = {.panelName = nullptr},  // No panel to load, just restore
        .blocking = false,  // Deactivation doesn't block
        .flags = InterruptFlags::NONE
    };
    interruptManager->RegisterInterrupt(lockDisengagedInterrupt);
    
    // 5. Lights On Interrupt - Single purpose: Set NIGHT theme
    Interrupt lightsOnInterrupt = {
        .id = "lights_on",
        .priority = Priority::NORMAL,
        .source = InterruptSource::POLLED,
        .execute = InterruptCallbacks::LightsOnActivate,
        .context = polledHandler->GetLightsSensor(),
        .data = {.theme = Themes::NIGHT},
        .blocking = false,  // Theme change doesn't block
        .flags = InterruptFlags::NONE
    };
    interruptManager->RegisterInterrupt(lightsOnInterrupt);
    
    // 6. Lights Off Interrupt - Single purpose: Set DAY theme
    Interrupt lightsOffInterrupt = {
        .id = "lights_off",
        .priority = Priority::NORMAL,
        .source = InterruptSource::POLLED,
        .execute = InterruptCallbacks::LightsOffActivate,
        .context = polledHandler->GetLightsSensor(),
        .data = {.theme = Themes::DAY},
        .blocking = false,  // Theme change doesn't block
        .flags = InterruptFlags::NONE
    };
    interruptManager->RegisterInterrupt(lightsOffInterrupt);
    
    // 7. Error Occurred Interrupt - Single purpose: Load ERROR panel and save restoration point
    Interrupt errorOccurredInterrupt = {
        .id = "error_occurred",
        .priority = Priority::CRITICAL,
        .source = InterruptSource::POLLED,
        .execute = InterruptCallbacks::ErrorOccurredActivate,
        .context = &ErrorManager::Instance(),
        .data = {.panelName = PanelNames::ERROR},
        .blocking = true,  // Error panel blocks restoration
        .flags = InterruptFlags::NONE
    };
    interruptManager->RegisterInterrupt(errorOccurredInterrupt);
    
    // 8. Error Cleared Interrupt - Single purpose: Restore to previous panel if no other blocking interrupts
    Interrupt errorClearedInterrupt = {
        .id = "error_cleared",
        .priority = Priority::CRITICAL,
        .source = InterruptSource::POLLED,
        .execute = InterruptCallbacks::ErrorClearedActivate,
        .context = &ErrorManager::Instance(),
        .data = {.panelName = nullptr},  // No panel to load, just restore
        .blocking = false,  // Deactivation doesn't block
        .flags = InterruptFlags::NONE
    };
    interruptManager->RegisterInterrupt(errorClearedInterrupt);
    
    // 9. Short Press Interrupt - Single purpose: Execute short press action
    Interrupt shortPressInterrupt = {
        .id = "short_press",
        .priority = Priority::IMPORTANT,
        .source = InterruptSource::QUEUED,
        .execute = InterruptCallbacks::ShortPressActivate,
        .context = queuedHandler->GetButtonSensor(),
        .data = {.action = ButtonAction::SHORT_PRESS},
        .blocking = false,  // Button actions don't block
        .flags = InterruptFlags::NONE
    };
    interruptManager->RegisterInterrupt(shortPressInterrupt);
    
    // 10. Long Press Interrupt - Single purpose: Execute long press action
    Interrupt longPressInterrupt = {
        .id = "long_press",
        .priority = Priority::NORMAL,
        .source = InterruptSource::QUEUED,
        .execute = InterruptCallbacks::LongPressActivate,
        .context = queuedHandler->GetButtonSensor(),
        .data = {.action = ButtonAction::LONG_PRESS},
        .blocking = false,  // Button actions don't block
        .flags = InterruptFlags::NONE
    };
    interruptManager->RegisterInterrupt(longPressInterrupt);
    
    log_i("Successfully registered 10 simplified system interrupts");
}