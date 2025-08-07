#include "main.h"
#include "device.h"
#include "managers/preference_manager.h"
#include "managers/style_manager.h"
#include "managers/trigger_manager.h"
#include "managers/panel_manager.h"
#include "managers/error_manager.h"
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
    
    panelManager->UpdatePanel();
    Ticker::handleLvTasks();
    Ticker::handleDynamicDelay(millis());
    
    // TEMPORARY: Test error handling with automatic error clearing
    #ifdef CLARITY_DEBUG
    static unsigned long lastTestTime = 0;
    static unsigned long lastClearTime = 0;
    static int testCounter = 0;
    static bool systemStable = false;
    static bool errorGenerated = false;
    
    // Wait for system to be stable (10 seconds after startup)
    if (!systemStable && millis() > 10000) {
        systemStable = true;
        log_d("System stable - error testing enabled");
    }
    
    if (systemStable) {
        // Generate new error every 7.5 seconds (reduced by half)
        if (millis() - lastTestTime > 7500) {
            lastTestTime = millis();
            lastClearTime = millis(); // Reset clear timer
            testCounter++;
            errorGenerated = true;
            
            switch (testCounter % 7) {
                case 1:
                    ErrorManager::Instance().ReportWarning("TestSystem", "Sample warning for testing error panel");
                    log_d("Generated test warning");
                    break;
                case 2:
                    ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "TestSystem", 
                        "Sample error for testing error panel");
                    log_d("Generated test error");
                    break;
                case 3:
                    ErrorManager::Instance().ReportCriticalError("TestSystem", 
                        "Sample critical error for testing");
                    log_d("Generated test critical error");
                    break;
                case 4:
                    ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "SensorSystem", 
                        "Sensor communication timeout detected");
                    log_d("Generated sensor error");
                    break;
                case 5:
                    ErrorManager::Instance().ReportWarning("DisplaySystem", 
                        "Display refresh rate below optimal threshold");
                    log_d("Generated display warning");
                    break;
                case 6:
                    ErrorManager::Instance().ReportCriticalError("PowerSystem", 
                        "Battery voltage critically low - system shutdown imminent");
                    log_d("Generated power critical error");
                    break;
                case 0:
                    ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "NetworkSystem", 
                        "Connection to remote monitoring service failed");
                    log_d("Generated network error");
                    break;
            }
        }
        
        // Clear errors after 5 seconds to allow new error types to show (reduced by half)
        if (errorGenerated && millis() - lastClearTime > 5000) {
            if (ErrorManager::Instance().HasPendingErrors()) {
                log_d("Auto-clearing errors after 5s to allow next error type");
                ErrorManager::Instance().ClearAllErrors();
                errorGenerated = false;
            }
        }
    }
    #endif
}

