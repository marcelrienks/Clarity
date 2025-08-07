#include "managers/panel_manager.h"
#include "managers/error_manager.h"
#include "utilities/ticker.h"
#include <esp32-hal-log.h>
#include <cstring>

// Core Functionality Methods

/// @brief Initialise the panel service to control the flow and rendering of all panels
/// Registers all available panel types with the factory for dynamic creation
/// Hardware providers are already injected via constructor
void PanelManager::Init()
{
    log_d("Initializing panel manager...");
    
    // Register all available panel types with the factory
    RegisterAllPanels();
    
    Ticker::handleLvTasks();
}


void PanelManager::RegisterAllPanels()
{
    log_d("Registering all panels...");

    // Panel registration is now handled by PanelFactory in main.cpp
    // This method performs no operations as factory handles all panel creation
    
    log_d("Panel registration handled by PanelFactory - no action required");
}


// Constructors and Destructors

PanelManager::PanelManager(IDisplayProvider *display, IGpioProvider *gpio, IStyleService *styleService)
    : gpioProvider_(gpio), displayProvider_(display), styleService_(styleService)
{
    if (!display || !gpio || !styleService) {
        log_e("PanelManager requires all dependencies: display, gpio, and styleService");
        ErrorManager::Instance().ReportCriticalError("PanelManager", 
            "Missing required dependencies - display, gpio, or styleService is null");
        // In a real embedded system, you might want to handle this more gracefully
    } else {
        log_d("Creating PanelManager with injected dependencies");
    }
    
    // Initialize the current panel buffer with the default value
    strncpy(currentPanelBuffer, PanelNames::OIL, sizeof(currentPanelBuffer) - 1);
    currentPanelBuffer[sizeof(currentPanelBuffer) - 1] = '\0';
    currentPanel = currentPanelBuffer;
}

PanelManager::~PanelManager()
{
    panel_.reset();
}

// Private Methods

/// @brief Create a panel based on the given type name
/// @param panelName the type name of the panel to be created
/// @return Interface type representing the panel
std::shared_ptr<IPanel> PanelManager::CreatePanel(const char *panelName)
{
    log_d("Creating panel instance for type: %s", panelName);

    // Use UIFactory for direct panel creation
    std::unique_ptr<IPanel> uniquePanel;
    
    if (strcmp(panelName, PanelNames::KEY) == 0) {
        uniquePanel = UIFactory::createKeyPanel(gpioProvider_, displayProvider_, styleService_);
    } else if (strcmp(panelName, PanelNames::LOCK) == 0) {
        uniquePanel = UIFactory::createLockPanel(gpioProvider_, displayProvider_, styleService_);
    } else if (strcmp(panelName, PanelNames::SPLASH) == 0) {
        uniquePanel = UIFactory::createSplashPanel(gpioProvider_, displayProvider_, styleService_);
    } else if (strcmp(panelName, PanelNames::OIL) == 0) {
        uniquePanel = UIFactory::createOemOilPanel(gpioProvider_, displayProvider_, styleService_);
    } else if (strcmp(panelName, PanelNames::ERROR) == 0) {
        uniquePanel = UIFactory::createErrorPanel(gpioProvider_, displayProvider_, styleService_);
    } else {
        log_e("Unknown panel type: %s", panelName);
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "PanelManager", 
            std::string("Unknown panel type: ") + panelName);
        return nullptr;
    }
    
    return std::shared_ptr<IPanel>(uniquePanel.release());
}



// Callback Methods

/// @brief callback function to be executed on splash panel show completion
void PanelManager::SplashCompletionCallback(const char *panelName)
{
    log_d("Splash screen animation completed, transitioning to panel: %s", panelName);

    panel_.reset();
    Ticker::handleLvTasks();

    CreateAndLoadPanel(panelName, [this]()
                       { this->PanelManager::PanelCompletionCallback(); });
}

/// @brief callback function to be executed on panel show completion
void PanelManager::PanelCompletionCallback()
{
    SetUiState(UIState::IDLE);
    
    // System initialization complete - triggers will remain in INIT state
    // until actual GPIO pin changes occur
    static bool systemInitialized = false;
    if (!systemInitialized)
    {
        systemInitialized = true;
    }
}

// Core IPanelService interface implementations

/// @brief Create and load a panel by name with optional completion callback
void PanelManager::CreateAndLoadPanel(const char *panelName, std::function<void()> completionCallback, bool isTriggerDriven)
{
    log_d("Creating and loading panel: %s (trigger-driven: %s)", panelName, isTriggerDriven ? "yes" : "no");

    // Track this as the last non-trigger panel only for user-driven changes
    if (!isTriggerDriven)
    {
        restorationPanel = panelName;
    }

    // Clean up existing panel before creating new one
    if (panel_)
    {
        log_d("Cleaning up existing panel before creating new one");
        panel_.reset();
    }

    panel_ = CreatePanel(panelName);
    if (panel_) {
        panel_->Init(gpioProvider_, displayProvider_);

        // Make a copy of the panel name to avoid pointer issues
        strncpy(currentPanelBuffer, panelName, sizeof(currentPanelBuffer) - 1);
        currentPanelBuffer[sizeof(currentPanelBuffer) - 1] = '\0';
        currentPanel = currentPanelBuffer;

        SetUiState(UIState::LOADING);
        panel_->Load(completionCallback, gpioProvider_, displayProvider_);
        Ticker::handleLvTasks();
    }
}

/// @brief Load a panel after first showing a splash screen transition
void PanelManager::CreateAndLoadPanelWithSplash(const char *panelName)
{
    log_d("Loading panel with splash screen transition: %s", panelName);

    // Capture panel name as string to avoid pointer corruption issues
    std::string targetPanel(panelName);
    CreateAndLoadPanel(PanelNames::SPLASH, [this, targetPanel]()
                       { this->PanelManager::SplashCompletionCallback(targetPanel.c_str()); });
}

/// @brief Update the currently active panel (called from main loop)
void PanelManager::UpdatePanel()
{
    if (panel_) {
        SetUiState(UIState::UPDATING);
        panel_->Update([this]()
                       { this->PanelManager::PanelCompletionCallback(); }, gpioProvider_, displayProvider_);
        Ticker::handleLvTasks();
    }
}

/// @brief Set current UI state for synchronization
void PanelManager::SetUiState(UIState state)
{
    uiState_ = state;
}

/// @brief Get the current panel name
const char *PanelManager::GetCurrentPanel() const
{
    return currentPanel;
}

/// @brief Get the restoration panel name (panel to restore when triggers are inactive)
const char *PanelManager::GetRestorationPanel() const
{
    return restorationPanel;
}

/// @brief Callback executed when trigger-driven panel loading is complete
void PanelManager::TriggerPanelSwitchCallback(const char *triggerId)
{
    SetUiState(UIState::IDLE);
    // No need to clear triggers - GPIO state manages trigger active/inactive status
}

