#include "main.h"
#include "device.h"
#include "managers/preference_manager.h"
#include "managers/style_manager.h"
#include "managers/trigger_manager.h"
#include "managers/panel_manager.h"
#include "managers/error_manager.h"
#include "managers/input_manager.h"
#include "factories/manager_factory.h"
#include "providers/gpio_provider.h"
#include "providers/lvgl_display_provider.h"
#include "utilities/types.h"
#include "utilities/ticker.h"

// Global services - direct instantiation to avoid service container issues
std::unique_ptr<Device> device;
std::unique_ptr<GpioProvider> gpioProvider;
std::unique_ptr<LvglDisplayProvider> displayProvider;
std::unique_ptr<StyleManager> styleManager;
std::unique_ptr<PreferenceManager> preferenceManager;
std::unique_ptr<PanelManager> panelManager;
std::unique_ptr<TriggerManager> triggerManager;
std::unique_ptr<InputManager> inputManager;


void initializeServices() {
    log_d("Starting service initialization...");
    
    log_d("Creating Device...");
    device = std::make_unique<Device>();
    
    log_d("Creating GpioProvider...");
    gpioProvider = std::make_unique<GpioProvider>();
    
    log_d("Creating DisplayProvider...");
    displayProvider = std::make_unique<LvglDisplayProvider>(device->screen);
    
    log_d("Creating StyleManager...");
    styleManager = std::make_unique<StyleManager>();
    styleManager->Init(Themes::DAY);
    
    log_d("Creating PreferenceManager...");
    preferenceManager = std::make_unique<PreferenceManager>();
    preferenceManager->Init();
    
    log_d("Creating PanelManager...");
    panelManager = std::make_unique<PanelManager>(
        displayProvider.get(),
        gpioProvider.get(),
        styleManager.get()
    );
    
    log_d("Creating TriggerManager...");
    triggerManager = ManagerFactory::createTriggerManager(
        gpioProvider.get(),
        panelManager.get(),
        styleManager.get()
    );
    
    log_d("Creating InputManager...");
    inputManager = ManagerFactory::createInputManager(gpioProvider.get(), panelManager.get());
    
    log_d("Connecting InputManager to PanelManager...");
    panelManager->SetInputManager(inputManager.get());
    
    log_d("Initializing ErrorManager...");
    // ErrorManager is a singleton, just initialize it
    ErrorManager::Instance();
    
    log_d("Service initialization completed successfully");
}


void setup()
{
    log_d("Starting Clarity application setup - using direct service instantiation");

    initializeServices();

    log_d("Preparing device...");
    device->prepare();
    Ticker::handleLvTasks();
    
    log_d("Initializing Clarity application");
    
    // Initialize trigger service after all dependencies are resolved
    triggerManager->Init();

    Ticker::handleLvTasks();

    // Check if startup triggers require a specific panel, otherwise use config default
    const char* startupPanel = triggerManager->GetStartupPanelOverride();
    if (startupPanel) {
        log_i("Using startup panel override: %s", startupPanel);
        panelManager->CreateAndLoadPanelWithSplash(startupPanel);
    } else {
        auto config = preferenceManager->GetConfig();
        log_i("Using config default panel: %s", config.panelName.c_str());
        panelManager->CreateAndLoadPanelWithSplash(config.panelName.c_str());
    }
    
    Ticker::handleLvTasks();
}

void loop()
{
    // Process trigger events directly
    triggerManager->ProcessTriggerEvents();
    
    // Process input events directly
    inputManager->ProcessInputEvents();
    
    panelManager->UpdatePanel();
    Ticker::handleLvTasks();
    Ticker::handleDynamicDelay(millis());
}

