#include "main.h"
#include "device.h"
#include "managers/preference_manager.h"
#include "managers/style_manager.h"
#include "managers/trigger_manager.h"
#include "managers/panel_manager.h"
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


void initializeServices() {
    log_d("Starting service initialization...");
    
    // Create services in dependency order
    log_d("Creating Device...");
    device = std::make_unique<Device>();
    
    log_d("Creating GpioProvider...");
    gpioProvider = std::make_unique<GpioProvider>();
    
    log_d("Creating DisplayProvider...");
    displayProvider = std::make_unique<LvglDisplayProvider>(device->screen);
    
    log_d("Creating StyleManager...");
    styleManager = std::make_unique<StyleManager>();
    styleManager->init(Themes::DAY);
    
    log_d("Creating PreferenceManager...");
    preferenceManager = std::make_unique<PreferenceManager>();
    preferenceManager->init();
    
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
    
    log_d("Service initialization completed successfully");
}

void setup()
{
    log_d("Starting Clarity application setup - using direct service instantiation");

    // Initialize all services
    initializeServices();

    // Initialize hardware
    log_d("Preparing device...");
    device->prepare();
    Ticker::handleLvTasks();
    
    // Initialize application
    log_d("Initializing Clarity application");
    
    // Initialize trigger service after all dependencies are resolved
    triggerManager->init();

    Ticker::handleLvTasks();

    // Check if startup triggers require a specific panel, otherwise use config default
    const char* startupPanel = triggerManager->getStartupPanelOverride();
    if (startupPanel) {
        log_i("Using startup panel override: %s", startupPanel);
        panelManager->createAndLoadPanelWithSplash(startupPanel);
    } else {
        auto config = preferenceManager->getConfig();
        log_i("Using config default panel: %s", config.panelName.c_str());
        panelManager->createAndLoadPanelWithSplash(config.panelName.c_str());
    }
    
    Ticker::handleLvTasks();
}

void loop()
{
    // Process trigger events directly
    triggerManager->processTriggerEvents();
    
    panelManager->updatePanel();
    Ticker::handleLvTasks();
    Ticker::handleDynamicDelay(millis());
}

