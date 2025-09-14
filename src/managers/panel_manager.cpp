#include "managers/panel_manager.h"
#include "utilities/logging.h"

// Static instance for singleton pattern
static PanelManager* instancePtr_ = nullptr;
#include "interfaces/i_action_service.h"
// Direct panel includes (factory pattern eliminated)
#include "panels/splash_panel.h"
#include "panels/oem_oil_panel.h"
#include "panels/error_panel.h"
#include "panels/config_panel.h"
#include "panels/key_panel.h"
#include "panels/lock_panel.h"
// ActionManager include removed - button handling moved to handler-based system
#include "managers/error_manager.h"
#include "managers/interrupt_manager.h"
#include "utilities/ticker.h"
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
                           IPreferenceService *preferenceService, InterruptManager* interruptManager)
    : gpioProvider_(gpio), displayProvider_(display), styleService_(styleService),
      preferenceService_(preferenceService), interruptManager_(interruptManager),
      errorManager_(ErrorManager::Instance())
{
    log_v("PanelManager() constructor called");
    if (!display || !gpio || !styleService || !preferenceService)
    {
        log_e("PanelManager requires all dependencies: display, gpio, styleService, and "
              "preferenceService");
        errorManager_.ReportCriticalError(
            "PanelManager",
            "Missing required dependencies - display, gpio, styleService, or preferenceService is null");
        // In a real embedded system, you might want to handle this more gracefully
        return;
    }


    // Panel names are already const char* constants, no conversion needed
    currentPanel_ = PanelNames::OIL;
    restorationPanel_ = PanelNames::OIL;
    
    // Set singleton instance
    instancePtr_ = this;
}

PanelManager::~PanelManager()
{
    log_v("~PanelManager() destructor called");
    panel_.reset();
    
    // Clear singleton instance
    if (instancePtr_ == this) {
        instancePtr_ = nullptr;
    }
}

// Private Methods

/// @brief Create a panel based on the given type name
/// @param panelName the type name of the panel to be created
/// @return Interface type representing the panel
std::shared_ptr<IPanel> PanelManager::CreatePanel(const char *panelName)
{
    log_v("CreatePanel() called for: %s", panelName);

    // Direct panel instantiation (factory pattern eliminated)

    if (strcmp(panelName, PanelNames::SPLASH) == 0) {
        return std::make_shared<SplashPanel>(gpioProvider_, displayProvider_, styleService_);
    }

    if (strcmp(panelName, PanelNames::OIL) == 0) {
        return std::make_shared<OemOilPanel>(gpioProvider_, displayProvider_, styleService_);
    }

    if (strcmp(panelName, PanelNames::ERROR) == 0) {
        return std::make_shared<ErrorPanel>(gpioProvider_, displayProvider_, styleService_);
    }

    if (strcmp(panelName, PanelNames::CONFIG) == 0) {
        return std::make_shared<ConfigPanel>(gpioProvider_, displayProvider_, styleService_);
    }

    if (strcmp(panelName, PanelNames::KEY) == 0) {
        return std::make_shared<KeyPanel>(gpioProvider_, displayProvider_, styleService_);
    }

    if (strcmp(panelName, PanelNames::LOCK) == 0) {
        return std::make_shared<LockPanel>(gpioProvider_, displayProvider_, styleService_);
    }
    
    // Unknown panel type
    log_e("Failed to create panel: %s", panelName);
    // Use char buffer to avoid string allocation in error path
    char errorMsg[128];
    snprintf(errorMsg, sizeof(errorMsg), "Failed to create panel: %s", panelName);
    errorManager_.ReportError(ErrorLevel::ERROR, "PanelManager", errorMsg);
    return nullptr;
}

// Core IPanelService interface implementations

/// @brief Create and load a panel by name
void PanelManager::CreateAndLoadPanel(const char *panelName, bool isTriggerDriven)
{
    log_t("Panel transition requested: %s", panelName);

    // Check if splash screen should be shown - only if NOT trigger-driven and splash is enabled
    bool showSplash = false;
    if (preferenceService_ && !isTriggerDriven)
    {
        const Configs &config = preferenceService_->GetConfig();
        showSplash = config.showSplash;
    }
    else
    {
        // Trigger-driven panel load - skip splash screen
    }

    if (showSplash)
    {
        log_i("Loading panel with splash transition: %s", panelName);
        CreateAndLoadPanelWithSplash(panelName, isTriggerDriven);
    }
    else
    {
        log_v("Loading panel directly: %s", panelName);
        CreateAndLoadPanelDirect(panelName, isTriggerDriven);
    }
}

/// @brief Internal method to create and load a panel directly without splash
void PanelManager::CreateAndLoadPanelDirect(const char *panelName, bool isTriggerDriven)
{
    log_v("CreateAndLoadPanelDirect() called for: %s", panelName);

    // Splash panel is purely transitional - don't track for restoration purposes
    bool isSplashPanel = strcmp(panelName, PanelNames::SPLASH) == 0;
    
    if (!isSplashPanel)
    {
        // If loading a trigger-driven panel, save the current panel for restoration
        if (isTriggerDriven && !currentPanelIsTriggerDriven_ && !currentPanel_.empty())
        {
            // Save the current non-trigger panel as the restoration target
            restorationPanel_ = currentPanel_;
            log_i("Saving current panel '%s' for restoration when triggers deactivate", restorationPanel_.c_str());
        }
        
        // Track current panel trigger state (only for non-splash panels)
        currentPanelIsTriggerDriven_ = isTriggerDriven;
    }

    // Clean up existing panel before creating new one
    if (panel_)
    {
        // ActionManager removed - button handling moved to handler-based system
        panel_.reset();
    }

    panel_ = CreatePanel(panelName);
    if (!panel_)
    {
        log_e("Failed to create panel: %s", panelName);
        // Use char buffer to avoid string allocation in error path
        char errorMsg[128];
        snprintf(errorMsg, sizeof(errorMsg), "Panel creation failed for: %s", panelName);
        errorManager_.ReportError(ErrorLevel::ERROR, "PanelManager", errorMsg);
        return;
    }

    // Inject managers for all panels (they can choose to use them or not)
    panel_->SetManagers(this, styleService_);

    // Special injection for ConfigPanel - inject preference service
    bool isConfig = strcmp(panelName, PanelNames::CONFIG) == 0;
    bool isOil = strcmp(panelName, PanelNames::OIL) == 0;
    bool isSplash = strcmp(panelName, PanelNames::SPLASH) == 0;
    
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

    // Update current panel directly (PanelNames are static constants)
    currentPanel_ = panelName;
    
    // Apply current theme from preferences BEFORE panel is loaded
    // This ensures correct theme is set before any rendering happens
    if (styleService_)
    {
        log_i("PanelManager: About to call ApplyCurrentTheme for panel: %s", panelName);
        styleService_->ApplyCurrentTheme();
        log_i("PanelManager: ApplyCurrentTheme completed for panel: %s", panelName);
    }
    else
    {
        log_w("PanelManager: No style service available to apply theme");
    }

    // Update universal button interrupts with this panel's functions
    UpdatePanelButtonFunctions(panel_.get());

    // Set BUSY before loading panel
    SetUiState(UIState::BUSY);
    
    panel_->Load();
    Ticker::handleLvTasks();
    
    log_v("CreateAndLoadPanelDirect() completed");
}

/// @brief Internal method to load a panel after first showing a splash screen transition
/// @note This method is only used during application startup, never during restoration
void PanelManager::CreateAndLoadPanelWithSplash(const char *panelName, bool isTriggerDriven)
{
    log_v("CreateAndLoadPanelWithSplash() called for: %s", panelName);

    // Store the target panel name for after splash completion
    // Using std::string for safe data storage (see docs/guidelines.md)
    splashTargetPanel_ = panelName;
    
    // Store the original trigger state to preserve restoration behavior
    splashTargetTriggerDriven_ = isTriggerDriven;
    
    // Splash panel itself is neutral - doesn't affect restoration logic
    // Use false for splash since it's just a transitional UI element
    CreateAndLoadPanelDirect(PanelNames::SPLASH, false);
}

// Callback Methods

/// @brief callback function to be executed on splash panel show completion
void PanelManager::SplashCompletionCallback(const char *panelName)
{
    log_v("SplashCompletionCallback() called for panel: %s", panelName);

    panel_.reset();
    Ticker::handleLvTasks();

    // Use the stored trigger state to preserve restoration behavior
    CreateAndLoadPanelDirect(panelName, splashTargetTriggerDriven_);
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
        
        panel_->Update();
        Ticker::handleLvTasks();
    }
}

/// @brief Set current UI state for synchronization
void PanelManager::SetUiState(UIState state)
{
    log_v("SetUiState() called with state: %s", UIStateToString(state));
    uiState_ = state;
    // UI state logging removed - was causing test loop due to frequent BUSY/IDLE transitions
}

/// @brief Get the current UI state
UIState PanelManager::GetUiState() const
{
    return uiState_;
}

/// @brief Get the current panel name
const char *PanelManager::GetCurrentPanel() const
{
    return currentPanel_.c_str();
}

/// @brief Get the restoration panel name (panel to restore when triggers are inactive)
const char *PanelManager::GetRestorationPanel() const
{
    return restorationPanel_.c_str();
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

/// @brief Update universal button interrupts with current panel's functions
void PanelManager::UpdatePanelButtonFunctions(IPanel* panel)
{
    log_v("UpdatePanelButtonFunctions() called");
    
    if (!panel || !interruptManager_)
    {
        log_e("Cannot update button functions - panel=%p, interrupt_manager=%p", 
              (void*)panel, (void*)interruptManager_);
        return;
    }
    
    
    // Try to cast panel to IActionService to get button functions
    IActionService* actionService = dynamic_cast<IActionService*>(panel);
    if (!actionService)
    {
        log_w("UpdatePanelButtonFunctions: Panel does not implement IActionService - no button functions available");
        return;
    }
    
    
    // Extract button functions from panel
    void (*shortPressFunc)(void* context) = actionService->GetShortPressFunction();
    void (*longPressFunc)(void* context) = actionService->GetLongPressFunction();
    void* panelContext = actionService->GetPanelContext();
    
    log_i("UpdatePanelButtonFunctions: Extracted functions - short=%p, long=%p, context=%p", 
          (void*)shortPressFunc, (void*)longPressFunc, panelContext);
    
    if (!shortPressFunc || !longPressFunc)
    {
        log_e("UpdatePanelButtonFunctions: Panel provided null button functions - short=%p, long=%p", 
              (void*)shortPressFunc, (void*)longPressFunc);
        return;
    }
    
    // Inject functions into universal button interrupts
    interruptManager_->UpdatePanelFunctions(shortPressFunc, longPressFunc, panelContext);
    
    log_i("UpdatePanelButtonFunctions: Successfully updated universal button interrupts with panel functions");
}

// Static instance for singleton pattern
// Static instance pointer already declared above

/// @brief Singleton access for new interrupt architecture
PanelManager& PanelManager::Instance() {
    if (!instancePtr_) {
        log_e("PanelManager::Instance() called before initialization");
        // In embedded systems, we need a valid instance
        // This should be set during ManagerFactory initialization
    }
    return *instancePtr_;
}

/// @brief Handle short button press action (new architecture)
void PanelManager::HandleShortPress() {
    log_t("HandleShortPress() called");
    
    if (!panel_) {
        log_w("No active panel for short press action");
        return;
    }
    
    // Try to cast panel to IActionService to handle press
    IActionService* actionService = dynamic_cast<IActionService*>(panel_.get());
    if (actionService) {
        void (*shortPressFunc)(void* context) = actionService->GetShortPressFunction();
        void* panelContext = actionService->GetPanelContext();
        if (shortPressFunc) {
            shortPressFunc(panelContext);
        }
    } else {
        // Current panel does not support button actions
    }
}

/// @brief Handle long button press action (new architecture)
void PanelManager::HandleLongPress() {
    log_t("HandleLongPress() called");
    
    if (!panel_) {
        log_w("No active panel for long press action");
        return;
    }
    
    // Try to cast panel to IActionService to handle press
    IActionService* actionService = dynamic_cast<IActionService*>(panel_.get());
    if (actionService) {
        void (*longPressFunc)(void* context) = actionService->GetLongPressFunction();
        void* panelContext = actionService->GetPanelContext();
        if (longPressFunc) {
            longPressFunc(panelContext);
        }
    } else {
        // Current panel does not support button actions
    }
}

/// @brief Load a panel by name (new architecture)
void PanelManager::LoadPanel(const char* panelName) {
    log_t("LoadPanel() called for: %s", panelName);
    CreateAndLoadPanel(panelName, true);  // Mark as trigger-driven
}

/// @brief Check restoration and load appropriate panel (new architecture)
void PanelManager::CheckRestoration() {
    log_t("CheckRestoration() called");
    
    // First check if any PANEL triggers are active that should take priority
    // This ensures we don't restore to oil panel if a trigger is still active
    if (interruptManager_) {
        // Ask the interrupt manager to evaluate and execute any active triggers
        // If a trigger is active, it will load its panel and we should not restore
        if (interruptManager_->CheckAndExecuteHighestPriorityTrigger()) {
            log_t("Active trigger found and executed - skipping restoration");
            return;
        }
        
        // Check for active STYLE triggers and apply them BEFORE loading restoration panel
        // This ensures theme triggers like lights are applied to the oil panel
        interruptManager_->CheckAndExecuteActiveStyleTriggers();
    }
    
    // No active panel triggers, proceed with restoration
    if (!restorationPanel_.empty()) {
        log_t("No blocking interrupts - restoring to '%s'", restorationPanel_.c_str());

        // Restoration should ALWAYS be direct - never use splash screen
        // Splash is only for application startup, not trigger restoration
        CreateAndLoadPanelDirect(restorationPanel_.c_str(), false);  // Direct restoration without splash
        
        // Note: restoration panel persists for future trigger activations
    } else {
        // No restoration panel to restore to
    }
}


// IPanelNotificationService implementation
void PanelManager::OnPanelLoadComplete(IPanel* panel) {
    log_i("Panel load completed for panel: %p", panel);
    
    // Check if this is a splash panel completion that should trigger target panel load
    if (currentPanel_ == PanelNames::SPLASH && !splashTargetPanel_.empty()) {
        log_i("Splash panel completed - transitioning to target panel: %s", splashTargetPanel_.c_str());
        // Splash target panel processing
        SplashCompletionCallback(splashTargetPanel_.c_str());

        // Clear the target panel after using it
        splashTargetPanel_.clear();
    } else {
        // Normal panel completion
        // Normal panel completion processing
        PanelCompletionCallback();
    }
}

void PanelManager::OnPanelUpdateComplete(IPanel* panel) {
    log_v("Panel update completed for panel: %p", panel);
    // Update completion doesn't require special handling - panel continues normal operation
}
