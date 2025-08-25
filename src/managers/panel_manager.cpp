#include "managers/panel_manager.h"
#include "interfaces/i_action_service.h"
#include "interfaces/i_panel_factory.h"
#include "interfaces/i_component_factory.h"
// ActionManager include removed - button handling moved to handler-based system
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
                           IPreferenceService *preferenceService,
                           IPanelFactory* panelFactory, IComponentFactory* componentFactory)
    : gpioProvider_(gpio), displayProvider_(display), styleService_(styleService),
      preferenceService_(preferenceService), panelFactory_(panelFactory ? panelFactory : &PanelFactory::Instance()),
      componentFactory_(componentFactory ? componentFactory : &ComponentFactory::Instance())
{
    log_v("PanelManager() constructor called");
    if (!display || !gpio || !styleService || !preferenceService)
    {
        log_e("PanelManager requires all dependencies: display, gpio, styleService, and "
              "preferenceService");
        ErrorManager::Instance().ReportCriticalError(
            "PanelManager",
            "Missing required dependencies - display, gpio, styleService, or preferenceService is null");
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
    log_d("Panel creation - type: %s, factory: %p", panelName, panelFactory_);
    
    if (strcmp(panelName, PanelNames::SPLASH) == 0) {
        return panelFactory_->CreateSplashPanel(gpioProvider_, displayProvider_, styleService_);
    }
    
    if (strcmp(panelName, PanelNames::OIL) == 0) {
        return panelFactory_->CreateOemOilPanel(gpioProvider_, displayProvider_, styleService_);
    }
    
    if (strcmp(panelName, PanelNames::ERROR) == 0) {
        return panelFactory_->CreateErrorPanel(gpioProvider_, displayProvider_, styleService_);
    }
    
    if (strcmp(panelName, PanelNames::CONFIG) == 0) {
        return panelFactory_->CreateConfigPanel(gpioProvider_, displayProvider_, styleService_);
    }
    
    if (strcmp(panelName, PanelNames::KEY) == 0) {
        return panelFactory_->CreateKeyPanel(gpioProvider_, displayProvider_, styleService_);
    }
    
    if (strcmp(panelName, PanelNames::LOCK) == 0) {
        return panelFactory_->CreateLockPanel(gpioProvider_, displayProvider_, styleService_);
    }
    
    // Unknown panel type
    log_e("Failed to create panel: %s", panelName);
    ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "PanelManager",
                                         std::string("Failed to create panel: ") + panelName);
    return nullptr;
}

// Core IPanelService interface implementations

/// @brief Create and load a panel by name
void PanelManager::CreateAndLoadPanel(const char *panelName, bool isTriggerDriven)
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
        CreateAndLoadPanelDirect(panelName, [this]() { this->PanelManager::PanelCompletionCallback(); }, isTriggerDriven);
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

        // ActionManager removed - button handling moved to handler-based system

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

    // ActionManager removed - button handling moved to handler-based system
    // TODO: Register panel button functions with QueuedHandler when implemented
    IActionService *actionService = panel_.get();
    if (actionService)
    {
        log_i("Panel %s implements IActionService - button functions ready for handler registration", currentPanel);
        // Button function registration will be handled by QueuedHandler in future implementation
    }
    else
    {
        log_d("Panel %s does not implement IActionService", panelName);
    }

    // Set BUSY before loading panel
    SetUiState(UIState::BUSY);
    
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
    CreateAndLoadPanelDirect(PanelNames::SPLASH, [this, targetPanel]()
                             { this->PanelManager::SplashCompletionCallback(targetPanel.c_str()); });
}

// Callback Methods

/// @brief callback function to be executed on splash panel show completion
void PanelManager::SplashCompletionCallback(const char *panelName)
{
    log_v("SplashCompletionCallback() called for panel: %s", panelName);

    panel_.reset();
    Ticker::handleLvTasks();

    CreateAndLoadPanelDirect(panelName, [this]() { this->PanelManager::PanelCompletionCallback(); });
}

/// @brief callback function to be executed on panel show completion
void PanelManager::PanelCompletionCallback()
{
    log_v("PanelCompletionCallback() called");
    
    // Set IDLE when panel operation completes
    SetUiState(UIState::IDLE);
}

/// @brief Update the currently active panel (called from main loop)
void PanelManager::UpdatePanel()
{
    log_v("UpdatePanel() called");
    if (panel_)
    {
        // Set BUSY before updating panel
        SetUiState(UIState::BUSY);
        
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
