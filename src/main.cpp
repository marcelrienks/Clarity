#include "main.h"
#include "factories/provider_factory.h"
#include "factories/manager_factory.h"
#include "providers/device_provider.h"
#include "providers/gpio_provider.h"
#include "providers/lvgl_display_provider.h"
#include "managers/style_manager.h"
#include "managers/preference_manager.h"
#include "managers/input_manager.h"
#include "managers/panel_manager.h"
#include "managers/trigger_manager.h"
#include "managers/interrupt_manager.h"
#include "managers/error_manager.h"
#include "utilities/types.h"
#include "utilities/ticker.h"

// Global services - factory-created instances
std::unique_ptr<DeviceProvider> deviceProvider;
std::unique_ptr<GpioProvider> gpioProvider;
std::unique_ptr<LvglDisplayProvider> displayProvider;
std::unique_ptr<StyleManager> styleManager;
std::unique_ptr<PreferenceManager> preferenceManager;
std::unique_ptr<InputManager> inputManager;
std::unique_ptr<PanelManager> panelManager;
std::unique_ptr<TriggerManager> triggerManager;
std::unique_ptr<InterruptManager> interruptManager;
ErrorManager *errorManager;

bool initializeServices()
{
    log_i("Starting Clarity service initialization...");

    // Create providers - factories handle all error checking and logging
    deviceProvider = ProviderFactory::createDeviceProvider();
    gpioProvider = ProviderFactory::createGpioProvider();
    displayProvider = ProviderFactory::createLvglDisplayProvider(deviceProvider.get());

    // Create managers - factories handle all error checking and logging
    styleManager = ManagerFactory::createStyleManager(Themes::DAY);
    preferenceManager = ManagerFactory::createPreferenceManager();
    inputManager = ManagerFactory::createInputManager(gpioProvider.get(), nullptr);
    panelManager = ManagerFactory::createPanelManager(
        displayProvider.get(),
        gpioProvider.get(), styleManager.get(),
        inputManager.get());
    triggerManager = ManagerFactory::createTriggerManager(
        gpioProvider.get(),
        panelManager.get(),
        styleManager.get());
    interruptManager = ManagerFactory::createInterruptManager();
    errorManager = ManagerFactory::createErrorManager();

    // Verify all critical services were created
    bool allServicesCreated = deviceProvider && gpioProvider && displayProvider &&
                              styleManager && preferenceManager && inputManager &&
                              panelManager && triggerManager && interruptManager && errorManager;

    if (!allServicesCreated)
    {
        log_e("Critical service creation failed - check factory logs above");
        return false;
    }

    // Setup InputManager callback after successful creation
    inputManager->SetPanelSwitchCallback([&](const char *panelName){
        log_d("InputManager requests panel switch to: %s", panelName);
        panelManager->CreateAndLoadPanel(panelName); });

    log_i("All services initialized successfully");
    return true;
}

void setup()
{
    log_i("Starting Clarity application...");

    if (!initializeServices())
    {
        log_e("Service initialization failed - cannot continue");
        return;
    }

    Ticker::handleLvTasks();
    styleManager->InitializeStyles();
    interruptManager->Init();
    interruptManager->RegisterInterruptSource(triggerManager.get()); // Priority 100
    interruptManager->RegisterInterruptSource(inputManager.get());   // Priority 50
    Ticker::handleLvTasks();

    // Load startup panel
    const char *startupPanel = triggerManager->GetStartupPanelOverride();
    if (startupPanel)
    {
        panelManager->CreateAndLoadPanelWithSplash(startupPanel);
    }
    else
    {
        auto config = preferenceManager->GetConfig();
        panelManager->CreateAndLoadPanelWithSplash(config.panelName.c_str());
    }

    Ticker::handleLvTasks();
    log_i("Clarity application started successfully");
}

void loop()
{
    // Process all interrupt sources via InterruptManager (triggers, inputs, etc.)
    interruptManager->CheckAllInterrupts();

    // Update panel state
    panelManager->UpdatePanel();
    Ticker::handleLvTasks();

    Ticker::handleDynamicDelay(millis());
}
