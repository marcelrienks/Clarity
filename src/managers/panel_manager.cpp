#include "managers/panel_manager.h"
#include "interfaces/i_action_service.h"
#include "interfaces/i_panel_factory.h"
#include "interfaces/i_component_factory.h"
#include "managers/action_manager.h"
#include "managers/error_manager.h"
#include "panels/config_panel.h"
#include "panels/oem_oil_panel.h"
#include "panels/splash_panel.h"
#include "utilities/ticker.h"
#include "factories/panel_factory.h"
#include "factories/component_factory.h"
#include "utilities/constants.h"
#include <cstring>
#include <esp32-hal-log.h>

// Core Functionality Methods

/// @brief Initialise the panel service to control the flow and rendering of all panels
/// Registers all available panel types with the factory for dynamic creation
/// Hardware providers are already injected via constructor
void PanelManager::Init()
{
    log_v("Init() called");

    Ticker::handleLvTasks();
    log_i("PanelManager initialization completed");
}

// Constructors and Destructors

PanelManager::PanelManager(IDisplayProvider *display, IGpioProvider *gpio, IStyleService *styleService,
                           IActionManager *actionManager, IPreferenceService *preferenceService,
                           IPanelFactory* panelFactory, IComponentFactory* componentFactory)
    : gpioProvider_(gpio), displayProvider_(display), styleService_(styleService), actionManager_(actionManager),
      preferenceService_(preferenceService), panelFactory_(panelFactory ? panelFactory : &PanelFactory::Instance()),
      componentFactory_(componentFactory ? componentFactory : &ComponentFactory::Instance())
{
    if (!display || !gpio || !styleService || !actionManager || !preferenceService)
    {
        log_e("PanelManager requires all dependencies: display, gpio, styleService, actionManager, and "
              "preferenceService");
        ErrorManager::Instance().ReportCriticalError(
            "PanelManager",
            "Missing required dependencies - display, gpio, styleService, actionManager, or preferenceService is null");
        // In a real embedded system, you might want to handle this more gracefully
        return;
    }

    log_d("Creating PanelManager with injected dependencies");

    // Initialize panel names using std::string for memory safety
    currentPanelStr_ = PanelNames::OIL;
    currentPanel = currentPanelStr_.c_str();

    restorationPanelStr_ = PanelNames::OIL;
    restorationPanel = restorationPanelStr_.c_str();
}

PanelManager::~PanelManager()
{
    log_v("~PanelManager() destructor called");
    panel_.reset();
}

// Private Methods

/// @brief Create a panel based on the given type name
/// @param panelName the type name of the panel to be created
/// @return Interface type representing the panel
std::shared_ptr<IPanel> PanelManager::CreatePanel(const char *panelName)
{
    log_v("CreatePanel() called for: %s", panelName);

    // Use injected factory for panel creation
    std::unique_ptr<IPanel> uniquePanel;
    log_d("Panel creation - type: %s, factory: %p", panelName, panelFactory_);
    
    if (strcmp(panelName, PanelNames::SPLASH) == 0) {
        uniquePanel = panelFactory_->CreateSplashPanel(gpioProvider_, displayProvider_, styleService_);
    } else if (strcmp(panelName, PanelNames::OIL) == 0) {
        uniquePanel = panelFactory_->CreateOemOilPanel(gpioProvider_, displayProvider_, styleService_);
    } else if (strcmp(panelName, PanelNames::ERROR) == 0) {
        uniquePanel = panelFactory_->CreateErrorPanel(gpioProvider_, displayProvider_, styleService_);
    } else if (strcmp(panelName, PanelNames::CONFIG) == 0) {
        uniquePanel = panelFactory_->CreateConfigPanel(gpioProvider_, displayProvider_, styleService_);
    } else if (strcmp(panelName, PanelNames::KEY) == 0) {
        uniquePanel = panelFactory_->CreateKeyPanel(gpioProvider_, displayProvider_, styleService_);
    } else if (strcmp(panelName, PanelNames::LOCK) == 0) {
        uniquePanel = panelFactory_->CreateLockPanel(gpioProvider_, displayProvider_, styleService_);
    }
    
    if (!uniquePanel)
    {
        log_e("Failed to create panel: %s", panelName);
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "PanelManager",
                                             std::string("Failed to create panel: ") + panelName);
        return nullptr;
    }

    log_v("CreatePanel() completed successfully for: %s", panelName);
    return std::shared_ptr<IPanel>(uniquePanel.release());
}

// Callback Methods

/// @brief callback function to be executed on splash panel show completion
void PanelManager::SplashCompletionCallback(const char *panelName)
{
    log_v("SplashCompletionCallback() called for panel: %s", panelName);

    panel_.reset();
    Ticker::handleLvTasks();

    CreateAndLoadPanel(panelName, [this]() { this->PanelManager::PanelCompletionCallback(); });
}

/// @brief callback function to be executed on panel show completion
void PanelManager::PanelCompletionCallback()
{
    log_v("PanelCompletionCallback() called");
    SetUiState(UIState::IDLE);

    // System initialization complete - triggers will remain in INIT state
    // until actual GPIO pin changes occur
    static bool systemInitialized = false;
    if (!systemInitialized)
    {
        systemInitialized = true;
        log_i("System initialization complete");
    }
}

// Core IPanelService interface implementations

/// @brief Create and load a panel by name with optional completion callback
void PanelManager::CreateAndLoadPanel(const char *panelName, std::function<void()> completionCallback,
                                      bool isTriggerDriven)
{
    log_i("Panel transition requested: %s", panelName);

    // Check if splash screen should be shown - only if NOT trigger-driven and splash is enabled
    bool showSplash = false;
    if (preferenceService_ && !isTriggerDriven)
    {
        const Configs &config = preferenceService_->GetConfig();
        showSplash = config.showSplash;
        log_d("User-driven panel load - splash setting: %s", showSplash ? "enabled" : "disabled");
    }
    else
    {
        log_d("Trigger-driven panel load - skipping splash screen");
    }

    if (showSplash)
    {
        log_d("Loading panel with splash screen transition: %s", panelName);
        CreateAndLoadPanelWithSplash(panelName);
    }
    else
    {
        log_d("Loading panel directly: %s", panelName);
        CreateAndLoadPanelDirect(panelName, completionCallback, isTriggerDriven);
    }
}

/// @brief Internal method to create and load a panel directly without splash
void PanelManager::CreateAndLoadPanelDirect(const char *panelName, std::function<void()> completionCallback,
                                            bool isTriggerDriven)
{
    log_v("CreateAndLoadPanelDirect() called for: %s", panelName);

    // Track current panel trigger state
    currentPanelIsTriggerDriven_ = isTriggerDriven;

    // Track this as the last user-driven panel only
    if (!isTriggerDriven)
    {
        // Update restoration panel using std::string for memory safety
        restorationPanelStr_ = panelName;
        restorationPanel = restorationPanelStr_.c_str();
        log_d("Restoration panel updated to: %s (user-driven)", restorationPanel);
    }

    // Clean up existing panel before creating new one
    if (panel_)
    {
        log_d("Cleaning up existing panel before creating new one");

        // Unregister input service if current panel implements it
        if (actionManager_)
        {
            actionManager_->ClearPanel();
        }

        panel_.reset();
    }

    panel_ = CreatePanel(panelName);
    if (!panel_)
    {
        log_e("Failed to create panel: %s", panelName);
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "PanelManager",
                                             std::string("Panel creation failed for: ") + panelName);
        return;
    }

    // Inject managers for all panels (they can choose to use them or not)
    panel_->SetManagers(this, styleService_);

    // Special injection for ConfigPanel - inject preference service
    bool isConfig = strcmp(panelName, PanelNames::CONFIG) == 0;
    bool isOil = strcmp(panelName, PanelNames::OIL) == 0;
    bool isSplash = strcmp(panelName, PanelNames::SPLASH) == 0;
    log_d("Panel injection - type: %s, isConfig: %s, isOil: %s, isSplash: %s, preferenceService: %p", 
          panelName, isConfig ? "true" : "false", isOil ? "true" : "false", 
          isSplash ? "true" : "false", preferenceService_);
    
    if (isConfig)
    {
        ConfigPanel *configPanel = static_cast<ConfigPanel *>(panel_.get());
        if (configPanel && preferenceService_)
        {
            configPanel->SetPreferenceService(preferenceService_);
        }
    }

    // Special injection for OemOilPanel - inject preference service to apply sensor update rates
    if (isOil)
    {
        OemOilPanel *oemOilPanel = static_cast<OemOilPanel *>(panel_.get());
        if (oemOilPanel && preferenceService_)
        {
            oemOilPanel->SetPreferenceService(preferenceService_);
        }
    }

    // Special injection for SplashPanel - inject preference service for configurable duration
    if (isSplash)
    {
        SplashPanel *splashPanel = static_cast<SplashPanel *>(panel_.get());
        if (splashPanel && preferenceService_)
        {
            splashPanel->SetPreferenceService(preferenceService_);
        }
    }

    panel_->Init();

    // Update current panel using std::string for memory safety
    currentPanelStr_ = panelName;
    currentPanel = currentPanelStr_.c_str();

    // Register input service if panel implements it (using composition approach)
    if (actionManager_)
    {
        IActionService *actionService = panel_->GetInputService();
        if (actionService)
        {
            log_i("Panel %s implements IActionService, registering for actions", currentPanel);
            actionManager_->RegisterPanel(actionService, currentPanel);
        }
        else
        {
            log_d("Panel %s does not implement IActionService", panelName);
        }
    }

    panel_->Load(completionCallback);
    Ticker::handleLvTasks();
    
    log_v("CreateAndLoadPanelDirect() completed");
}

/// @brief Internal method to load a panel after first showing a splash screen transition
void PanelManager::CreateAndLoadPanelWithSplash(const char *panelName)
{
    log_v("CreateAndLoadPanelWithSplash() called for: %s", panelName);

    // Capture panel name as string to avoid pointer corruption issues
    std::string targetPanel(panelName);
    CreateAndLoadPanel(PanelNames::SPLASH, [this, targetPanel]()
                             { this->PanelManager::SplashCompletionCallback(targetPanel.c_str()); });
}

/// @brief Update the currently active panel (called from main loop)
void PanelManager::UpdatePanel()
{
    log_v("UpdatePanel() called");
    if (panel_)
    {
        panel_->Update([this]() { this->PanelManager::PanelCompletionCallback(); });
        Ticker::handleLvTasks();
    }
}

/// @brief Set current UI state for synchronization
void PanelManager::SetUiState(UIState state)
{
    log_v("SetUiState() called with state: %s", UIStateToString(state));
    uiState_ = state;
}

/// @brief Get the current UI state
UIState PanelManager::GetUiState() const
{
    log_v("GetUiState() called");
    return uiState_;
}

/// @brief Get the current panel name
const char *PanelManager::GetCurrentPanel() const
{
    log_v("GetCurrentPanel() called");
    return currentPanel;
}

/// @brief Get the restoration panel name (panel to restore when triggers are inactive)
const char *PanelManager::GetRestorationPanel() const
{
    log_v("GetRestorationPanel() called");
    return restorationPanel;
}

/// @brief Check if the current panel was loaded by a trigger
/// @return true if panel was loaded by hardware trigger, false if user-driven
bool PanelManager::IsCurrentPanelTriggerDriven() const
{
    log_v("IsCurrentPanelTriggerDriven() called");
    return currentPanelIsTriggerDriven_;
}

/// @brief Callback executed when trigger-driven panel loading is complete
void PanelManager::TriggerPanelSwitchCallback(const char *triggerId)
{
    log_v("TriggerPanelSwitchCallback() called for trigger: %s", triggerId);
    SetUiState(UIState::IDLE);
}
